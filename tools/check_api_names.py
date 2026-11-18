# SPDX-License-Identifier: MIT
# Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

"""Check that every function in the public header is defined with the same
return type and parameter names.

    python tools/check_api_names.py [<repo root>]

The header (inc/cprocessing.h) is what an editor shows a student, so the
names in it are the ones that count; a source file that calls a parameter
something else would show up in a debugger and in the next person's head.
Prints each difference and exits with status 1 if there is one.
"""
import re
import sys
from pathlib import Path

root = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent)
lib = root / "CProcessing"

PATTERN = re.compile(r"CP_API\s+([\w\s\*]+?)\s*\b(CP_\w+)\s*\(([^)]*)\)\s*([;{]?)")


def param_names(text):
    if text.strip() in ("", "void"):
        return []
    names = []
    for arg in text.split(","):
        m = re.search(r"(\w+)\s*(\[\])?\s*$", arg.strip())
        names.append(m.group(1) if m else arg.strip())
    return names


declared = {}
for m in PATTERN.finditer((lib / "inc" / "cprocessing.h").read_text(encoding="utf-8")):
    declared[m.group(2)] = (" ".join(m.group(1).split()), param_names(m.group(3)))

defined = {}
for path in sorted((lib / "src").glob("*.c")):
    for m in PATTERN.finditer(path.read_text(encoding="utf-8", errors="replace")):
        if m.group(4) != ";":
            defined[m.group(2)] = (" ".join(m.group(1).split()), param_names(m.group(3)), path.name)

problems = 0
for name, (ret, params) in declared.items():
    if name not in defined:
        print(f"{name}: declared in cprocessing.h but not defined in src")
        problems += 1
        continue
    ret2, params2, where = defined[name]
    if ret != ret2:
        print(f"{name}: returns '{ret}' in the header but '{ret2}' in {where}")
        problems += 1
    if params != params2:
        print(f"{name}: parameters ({', '.join(params)}) in the header but ({', '.join(params2)}) in {where}")
        problems += 1
for name, (_, _, where) in defined.items():
    if name not in declared:
        print(f"{name}: defined in {where} but not declared in cprocessing.h (or cprocessing_common.h)")
        problems += 1

print(f"{len(declared)} functions checked, {problems} problem(s)")
sys.exit(1 if problems else 0)
