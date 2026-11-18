# SPDX-License-Identifier: MIT
# Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

"""Check that every first-party source file carries the license header.

    python tools/check_license_headers.py

The rule (BUILDING.md, "License headers"): a source file we author says, near
the top,

    SPDX-License-Identifier: MIT
    Copyright (c) <year or years> DigiPen Institute of Technology and CProcessing contributors

in the file's own comment syntax. Left out on purpose: the bundled libraries
and Unity (their own licenses), the templates (the student's files), and the
embedded Exo 2 font data (SIL Open Font License, which has its own line).
Prints each file that is missing or has a wrong header and exits with status 1
if there is one.
"""
import re
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent

EXTENSIONS = (".c", ".h", ".cpp", ".py", ".sh", ".bat", ".cmd", ".ps1", ".cmake")
SKIP_PREFIXES = ("CProcessing/third_party/", "tests/unity/", "templates/", "CProcessing/lib/")
OWN_LICENSE = {"CProcessing/src/Internal_Resources.h": "OFL-1.1"}

MIT_LINE = re.compile(r"SPDX-License-Identifier: MIT\s*$")
COPYRIGHT_LINE = re.compile(r"Copyright \(c\) \d{4}(-\d{4})? DigiPen Institute of Technology and CProcessing contributors\s*$")
HEAD_LINES = 80  # the header ends the first comment; an example's teaching notes can run past line 40


def tracked():
    out = subprocess.run(["git", "ls-files", "-z"], cwd=root, check=True, capture_output=True).stdout
    return [p for p in out.decode("utf-8", errors="replace").split("\0") if p]


def needs_header(path):
    if path.startswith(SKIP_PREFIXES):
        return False
    if path.startswith("docs/") and not path.startswith("docs/offline/"):
        return False
    return path.endswith(EXTENSIONS) or path == "CMakeLists.txt"


def head(path):
    text = (root / path).read_text(encoding="utf-8", errors="replace")
    return text.splitlines()[:HEAD_LINES]


def main():
    bad = 0
    for path in tracked():
        if path in OWN_LICENSE:
            wanted = f"SPDX-License-Identifier: {OWN_LICENSE[path]}"
            if not any(wanted in line for line in head(path)):
                print(f"{path}: should say '{wanted}'")
                bad += 1
            continue
        if not needs_header(path):
            continue
        lines = head(path)
        if not any(MIT_LINE.search(line) for line in lines):
            print(f"{path}: no 'SPDX-License-Identifier: MIT' line near the top")
            bad += 1
        elif not any(COPYRIGHT_LINE.search(line) for line in lines):
            print(f"{path}: no 'Copyright (c) <years> DigiPen Institute of Technology and CProcessing contributors' line")
            bad += 1
    license_lines = (root / "LICENSE").read_text(encoding="utf-8").splitlines()
    if not any(COPYRIGHT_LINE.match(line) for line in license_lines):
        print("LICENSE: the copyright line should name 'DigiPen Institute of Technology and CProcessing contributors'")
        bad += 1
    print(f"{bad} problem(s)")
    return 1 if bad else 0


sys.exit(main())
