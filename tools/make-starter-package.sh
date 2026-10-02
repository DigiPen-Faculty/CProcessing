#!/usr/bin/env bash
# Assemble a Linux or macOS release package: the CMake starter project
# (Processing_Empty_CMake) with a prebuilt CProcessing library.
#
#   tools/make-starter-package.sh <built library> <output .zip>
#
# For example, after a Release build of the repository with CMake:
#   tools/make-starter-package.sh build/bin/libCProcessing.so CProcessingTemplate-linux-x64.zip
#
# The zip holds one folder, CProcessingTemplate/:
#   CMakeLists.txt, main.c, README.md, .vscode/   the starter project
#   Assets/                                       the template's sample assets
#   CProcessing/inc/cprocessing*.h                the public headers
#   CProcessing/lib/<library>                     the prebuilt library
#   CProcessing/Exo2_license.txt                  license of the built-in font
set -euo pipefail

if [ $# -ne 2 ]; then
    echo "usage: $0 <built library> <output .zip>" >&2
    exit 2
fi

repo="$(cd "$(dirname "$0")/.." && pwd)"
lib="$1"
out="$(cd "$(dirname "$2")" && pwd)/$(basename "$2")"

stage="$(mktemp -d)"
trap 'rm -rf "$stage"' EXIT
pkg="$stage/CProcessingTemplate"

mkdir -p "$pkg/CProcessing/inc" "$pkg/CProcessing/lib"
cp -R "$repo/Processing_Empty_CMake/." "$pkg/"
rm -rf "$pkg/build"
cp -R "$repo/Processing_Empty/Assets/." "$pkg/Assets/"
cp "$repo"/Processing_Sample/CProcessing/inc/cprocessing*.h "$pkg/CProcessing/inc/"
cp "$lib" "$pkg/CProcessing/lib/"
cp "$repo/Processing_Sample/Assets/Exo2_license.txt" "$pkg/CProcessing/"

rm -f "$out"
(cd "$stage" && cmake -E tar cf "$out" --format=zip CProcessingTemplate)
echo "Wrote $out:"
(cd "$stage" && find CProcessingTemplate -type f | sort)
