# SPDX-License-Identifier: MIT
# Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

"""Check that every example can be found.

    python tools/check_examples.py

The rules are in DESIGN.md, "Examples and lessons". For each examples/*.c:
  - examples/README.md links to it from one of its groups (a table row that
    starts with the link)
  - its header has a "See also:" line naming the examples next to it
and every example the README links to exists. Prints each problem and exits
with status 1 if there is one. The wiki's Examples page is checked by
tools/wiki_audit.py.
"""
import re
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent
examples = root / "examples"

problems = []
files = sorted(p.name for p in examples.glob("*.c"))
readme = (examples / "README.md").read_text(encoding="utf-8")

# A group row: "| [name.c](name.c) | what it shows |"
rows = set(re.findall(r"(?m)^\| \[([a-z0-9_]+\.c)\]\(\1\) \|", readme))
for name in files:
    if name not in rows:
        problems.append(f"examples/README.md: no row for {name} in the groups")
for name in sorted(set(re.findall(r"\]\(([a-z0-9_]+\.c)\)", readme)) - set(files)):
    problems.append(f"examples/README.md: links to {name}, which doesn't exist")

for name in files:
    header = (examples / name).read_text(encoding="utf-8").split("SPDX-License-Identifier", 1)[0]
    if not re.search(r"(?m)^// See also: \S", header):
        problems.append(f"examples/{name}: no '// See also:' line in the header")

for p in problems:
    print(p)
print(f"{len(files)} examples checked, {len(problems)} problem(s)")
sys.exit(1 if problems else 0)
