#!/usr/bin/env bash
# Shows CProcessing's changes to its vendored NanoVG: the difference between
# Processing_Sample/CProcessing/nanovg/ and the upstream commits it is based
# on. Every difference should be one of the patches listed in
# Processing_Sample/CProcessing/nanovg/CPROCESSING.md, marked in the code
# with a "CProcessing:" comment. stb_image.h and stb_truetype.h come from
# the stb repository and have no changes.
#
# Usage: tools/nanovg-diff.sh [--stat]
# Needs git and network access. Runs in Git Bash on Windows.
set -euo pipefail

# Keep these in step with CPROCESSING.md and DEPENDENCIES.md.
NANOVG_URL=https://github.com/memononen/nanovg.git
NANOVG_COMMIT=ce3bf745eb2d2dbc14a50bf2446783f691ac4353  # 2026-02-19
STB_URL=https://github.com/nothings/stb.git
STB_COMMIT=2c980bb59875b0d32144a71867fbdebb2f77cd20     # 2026-08-01

# Upstream repository, path there, path in the vendored folder
FILES=(
    "nanovg src/nanovg.c src/nanovg.c"
    "nanovg src/nanovg.h src/nanovg.h"
    "nanovg src/nanovg_gl.h src/nanovg_gl.h"
    "nanovg src/nanovg_gl_utils.h src/nanovg_gl_utils.h"
    "nanovg src/fontstash.h src/fontstash.h"
    "nanovg LICENSE.txt LICENSE.txt"
    "nanovg README.md README.md"
    "stb stb_image.h src/stb_image.h"
    "stb stb_truetype.h src/stb_truetype.h"
)

repo=$(cd "$(dirname "$0")/.." && pwd)
vendored="$repo/Processing_Sample/CProcessing/nanovg"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

fetch() {  # name url commit
    git init --quiet "$tmp/$1"
    git -C "$tmp/$1" fetch --quiet --depth 1 "$2" "$3"
    git -C "$tmp/$1" -c advice.detachedHead=false checkout --quiet FETCH_HEAD
}
fetch nanovg "$NANOVG_URL" "$NANOVG_COMMIT"
fetch stb "$STB_URL" "$STB_COMMIT"

status=0
for entry in "${FILES[@]}"; do
    read -r upstream path local <<< "$entry"
    # Line endings differ between checkouts on Windows and elsewhere; ignore them
    git -c core.safecrlf=false --no-pager diff --no-index --ignore-cr-at-eol "${@}" \
        "$tmp/$upstream/$path" "$vendored/$local" || status=$?
done
# git diff --no-index exits 1 when files differ, which is expected here
[ "$status" -le 1 ]
