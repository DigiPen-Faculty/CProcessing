"""Check that every tracked folder and file follows the naming rule.

    python tools/check_layout.py [--rev <git revision>]

The rule (BUILDING.md, "Naming and layout"): a folder or file we author is
lowercase, words joined with underscores. Prints each name that breaks it
and exits with status 1 if there is one. With --rev, checks that revision
instead of the working tree (handy to see what an old layout would report).

The exceptions are the ones in BUILDING.md; each is a pattern below, so a new
one has to be added here, where a reviewer sees it.
"""
import re
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent

LOWER = re.compile(r"^[a-z0-9][a-z0-9_]*$")
LOWER_FILE = re.compile(r"^[a-z0-9][a-z0-9_]*(\.[a-z0-9_]+)*$")

# Folders with a fixed name: the product, and folders other tools look for.
FOLDER_EXCEPTIONS = {"CProcessing"}

# File names other people or tools expect, or that name a product.
FILE_PATTERNS = [
    (re.compile(r"^\.[A-Za-z0-9_.-]+$"), "a dot-file"),
    (re.compile(r"^[A-Z][A-Z0-9_]*(\.[a-z]+)?$"), "a document with an ALL-CAPS name (README.md, LICENSE, ...)"),
    (re.compile(r"^CMakeLists\.txt$"), "CMake looks for this name"),
    (re.compile(r"^(CProcessing|MyGame)(_[A-Za-z0-9]+)*\.(sln|vcxproj|vcxproj\.filters)$"), "a Visual Studio project, named after the product"),
]

# Names that mirror the API: CP_Color.c holds CP_Color_* (see check_api_names.py).
SOURCE_PREFIXES = re.compile(r"^(CP_[A-Za-z0-9]+\.c|Internal_[A-Za-z0-9]+\.h)$")

LESSON_PAGE = re.compile(r"^Lesson-\d\d-[A-Za-z0-9-]+\.md$")  # becomes a wiki page name


def tracked(rev):
    if rev:
        cmd = ["git", "ls-tree", "-r", "--name-only", "-z", rev]
    else:
        cmd = ["git", "ls-files", "-z"]
    out = subprocess.run(cmd, cwd=root, check=True, capture_output=True).stdout
    return [p for p in out.decode("utf-8", errors="replace").split("\0") if p]


def problems_in(path):
    parts = path.split("/")
    folders, name = parts[:-1], parts[-1]
    found = []

    # Other people's code keeps its names, below the folder we put it in.
    if len(parts) > 3 and parts[:2] == ["CProcessing", "third_party"]:
        folders, name = folders[:3], None
    if parts[:2] == ["tests", "unity"]:
        folders, name = folders[:2], None

    # Image, sound and font files are content that programs refer to by name.
    lowered = [f.lower() for f in folders]
    if "assets" in lowered:
        at = lowered.index("assets")
        folders, name = folders[: at + 1], None
        for f in parts[at + 1 : -1]:
            if not LOWER.match(f):
                found.append(f"folder '{f}' should be lowercase")

    for f in folders:
        if f in FOLDER_EXCEPTIONS or f.startswith(".") or LOWER.match(f):
            continue
        found.append(f"folder '{f}' should be lowercase with underscores")

    if name is not None:
        ok = LOWER_FILE.match(name)
        ok = ok or any(rx.match(name) for rx, _ in FILE_PATTERNS)
        ok = ok or (parts[:2] == ["CProcessing", "src"] and SOURCE_PREFIXES.match(name))
        ok = ok or (parts[:2] == ["docs", "lessons"] and LESSON_PAGE.match(name))
        ok = ok or parts[:3] == ["docs", "lessons", "images"]  # pictures the wiki pages show
        ok = ok or path == "docs/offline/student/Open CProcessing Docs.html"
        if not ok:
            found.append(f"file '{name}' should be lowercase with underscores")
    return found


def main():
    rev = None
    if "--rev" in sys.argv:
        rev = sys.argv[sys.argv.index("--rev") + 1]
    bad = 0
    seen = set()
    for path in tracked(rev):
        for message in problems_in(path):
            if (path.rsplit("/", 1)[0], message) in seen and message.startswith("folder"):
                continue  # report a folder once, not once per file in it
            seen.add((path.rsplit("/", 1)[0], message))
            print(f"{path}: {message}")
            bad += 1
    print(f"{bad} problem(s)")
    return 1 if bad else 0


sys.exit(main())
