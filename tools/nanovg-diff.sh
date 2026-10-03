#!/usr/bin/env bash
# Shows CProcessing's changes to its vendored NanoVG: the difference between
# Processing_Sample/CProcessing/nanovg/ and the upstream commit it is based
# on. Every difference should be one of the patches listed in
# Processing_Sample/CProcessing/nanovg/CPROCESSING.md, marked in the code
# with a "CProcessing:" comment.
#
# Usage: tools/nanovg-diff.sh [--stat]
# Needs git and network access. Runs in Git Bash on Windows.
set -euo pipefail

# Keep these in step with CPROCESSING.md and DEPENDENCIES.md.
NANOVG_URL=https://github.com/memononen/nanovg.git
NANOVG_COMMIT=ce3bf745eb2d2dbc14a50bf2446783f691ac4353  # 2026-02-19

# Vendored file -> path in the upstream repository
FILES=(
    "src/nanovg.c"
    "src/nanovg.h"
    "src/nanovg_gl.h"
    "src/nanovg_gl_utils.h"
    "src/fontstash.h"
    "src/stb_image.h"
    "src/stb_truetype.h"
    "LICENSE.txt"
    "README.md"
)

repo=$(cd "$(dirname "$0")/.." && pwd)
vendored="$repo/Processing_Sample/CProcessing/nanovg"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

git init --quiet "$tmp/nanovg"
git -C "$tmp/nanovg" fetch --quiet --depth 1 "$NANOVG_URL" "$NANOVG_COMMIT"
git -C "$tmp/nanovg" -c advice.detachedHead=false checkout --quiet FETCH_HEAD

status=0
for f in "${FILES[@]}"; do
    # Line endings differ between checkouts on Windows and elsewhere; ignore them
    git -c core.safecrlf=false --no-pager diff --no-index --ignore-cr-at-eol "${@}" \
        "$tmp/nanovg/$f" "$vendored/$f" || status=$?
done
# git diff --no-index exits 1 when files differ, which is expected here
[ "$status" -le 1 ]
