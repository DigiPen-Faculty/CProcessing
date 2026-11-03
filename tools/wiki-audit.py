"""Check the CProcessing wiki against the public headers.

    python tools/wiki-audit.py <wiki clone> [<repo root>]

The wiki (https://github.com/DigiPen-Faculty/CProcessing/wiki) is a separate
git repository, so nothing stops it drifting from the API. This reports, and
exits with status 1 for:
  - API functions (CP_API in cprocessing.h / cprocessing_common.h) with no
    "# CP_Name" entry, or with more than one
  - entries for names the headers don't define (a removed or renamed function)
  - an entry whose "## Function" signature differs from the header
  - parameter bullets ("* name (type) - ...") that don't match the signature's
    parameter names
  - links to a page or heading that doesn't exist, and table-of-contents
    entries missing or out of order
  - Windows-only calls in examples (sprintf_s, _countof, ...)
The offline-docs workflow runs it every week on the wiki it just cloned.
"""
import re
import sys
from pathlib import Path

# Deliberate exceptions
PARAM_EXCEPTIONS = {"CP_Matrix_Set"}                 # documents m00..m22 as one bullet
PORTABILITY_EXCEPTIONS = {"CP_System_GetWindowHandle"}  # its example is Windows-only on purpose

wiki = Path(sys.argv[1])
repo = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(__file__).resolve().parent.parent
inc = repo / "Processing_Sample/CProcessing/inc"
problems = []


def squash(s):
    return re.sub(r"\s+", "", s)


def param_names(signature):
    inner = signature[signature.find("(") + 1: signature.rfind(")")].strip()
    if inner in ("", "void"):
        return []
    return [re.search(r"(\w+)\s*$", p.strip()).group(1) for p in inner.split(",")]


# The API
header_text = ""
api = {}
for header in ("cprocessing.h", "cprocessing_common.h"):
    text = (inc / header).read_text(encoding="latin-1")
    header_text += text
    for m in re.finditer(r"^CP_API\s+([^;]+?\))\s*;", text, re.M):
        decl = re.sub(r"\s+", " ", m.group(1)).strip()
        api[re.search(r"(\w+)\s*\(", decl).group(1)] = decl + ";"

# The wiki: pages, heading anchors, entries
pages = {p.stem: p.read_text(encoding="utf-8") for p in sorted(wiki.glob("*.md"))}
anchors = {}
entries = {}  # name -> list of (page, entry text)
for page, text in pages.items():
    anchors[page] = set()
    in_code = False
    for line in text.splitlines():
        if line.startswith("```"):
            in_code = not in_code
        elif not in_code:
            h = re.match(r"^#+\s+(.*?)\s*$", line)
            if h:
                anchors[page].add(re.sub(r"[^\w\- ]", "", h.group(1)).strip().lower().replace(" ", "-"))
    for part in re.split(r"(?m)^(?=# CP_\w+(?:\(\))?\s*$)", text):
        h = re.match(r"# (CP_\w+?)(?:\(\))?\s*\n", part)
        if h:
            entries.setdefault(h.group(1), []).append((page, part))

for name in sorted(api):
    found = entries.get(name, [])
    if not found:
        problems.append(f"{name}: no wiki entry")
    elif len(found) > 1:
        problems.append(f"{name}: documented {len(found)} times ({', '.join(p for p, _ in found)})")

for name, found in sorted(entries.items()):
    if name not in api and not re.search(r"\b" + re.escape(name) + r"\b", header_text):
        problems.append(f"{name} ({found[0][0]}): not in the headers")
    for page, part in found:
        if name not in api:
            continue
        fm = re.search(r"## Function\s*\n```[Cc]\s*\n(.*?)\n```", part, re.S)
        if not fm:
            problems.append(f"{name} ({page}): no '## Function' code block")
            continue
        signature = fm.group(1).strip()
        if squash(signature) != squash(api[name]):
            problems.append(f"{name} ({page}): signature\n      wiki   {signature}\n      header {api[name]}")
        pm = re.search(r"### Parameters\s*\n(.*?)(?:\n###|\n## |\Z)", part, re.S)
        if pm and name not in PARAM_EXCEPTIONS:
            have = re.findall(r"(?m)^\* (\w+) \(", pm.group(1))
            want = param_names(api[name])
            if have != want:
                problems.append(f"{name} ({page}): parameter bullets {have}, signature {want}")
        if name not in PORTABILITY_EXCEPTIONS:
            for line in part.splitlines():
                if re.search(r"\b(sprintf_s|strcpy_s|strcat_s|fopen_s|scanf_s|_countof|HWND|windows\.h)\b", line):
                    problems.append(f"{name} ({page}): Windows-only code: {line.strip()}")

for page, text in pages.items():
    prose = re.sub(r"```.*?```", "", text, flags=re.S)
    for m in re.finditer(r"\]\(([^)\s]+)\)", prose):
        target = m.group(1)
        if target.startswith(("http://", "https://", "mailto:")):
            continue
        path, _, fragment = target.partition("#")
        target_page = path.rstrip("/") or page
        if target_page not in pages:
            problems.append(f"{page}: link to a missing page: {target}")
        elif fragment and fragment.lower() not in anchors[target_page]:
            problems.append(f"{page}: link to a missing heading: {target}")
    heads = [h.lower() for h in re.findall(r"(?m)^# (CP_\w+?)(?:\(\))?\s*$", text)]
    if "# Table" in text:
        toc = [t.lower() for t in re.findall(r"\(#(\w+)\)", text.split("# Table", 1)[1].split("\n# ", 1)[0])]
        if [t for t in toc if t in heads] != [h for h in heads if h in toc] or set(heads) - set(toc):
            problems.append(f"{page}: table of contents doesn't list every entry in page order")

for p in problems:
    print(p)
print(f"{len(problems)} problem(s): {len(api)} API functions, {len(pages)} wiki pages")
sys.exit(1 if problems else 0)
