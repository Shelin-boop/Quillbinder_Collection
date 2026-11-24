#!/usr/bin/env bash
# vcpkg's ports for the Windows build (Windows 0.93.0): Hunspell and zlib come from vcpkg (statically linked into
# quillbinder.exe). Until 0.92.0 the build took the ports of whatever vcpkg GitHub's runner image had (zlib 1.3.2 in
# 0.92.0); since 0.93.0 the runner's vcpkg is moved to the commit below first, so that the program gets the zlib of
# that commit (1.4, the same zlib as Qt 6.12.1; security issue #8) and the recipe of the Hunspell pack
# (packaging/windows/win-deps-pack.sh, which reads the ports' files) stays the same from run to run.
#
#   packaging/windows/win-vcpkg.sh            move the runner's vcpkg (VCPKG_INSTALLATION_ROOT) to COMMIT
#   packaging/windows/win-vcpkg.sh tool       ... and fetch the vcpkg program that commit names (before "vcpkg install")
#
# A new commit: its ports/zlib/vcpkg.json must say ZLIB below (the script stops otherwise), and tools/security-watch.json
# names that zlib ("zlib (vcpkg, Windows)", pinned to ZLIB here).
set -euo pipefail
COMMIT=7f2ca8f8f9a2b6ae02ea4b4b7e0ef27490568afa   # microsoft/vcpkg master, 2026-11-24
ZLIB=1.4
HUNSPELL=1.7.5
VR=$(cygpath -u "${VCPKG_INSTALLATION_ROOT:?vcpkg is not on this runner image}")
if [ "$(git -C "$VR" rev-parse HEAD 2> /dev/null)" != "$COMMIT" ]; then
    git -C "$VR" -c advice.detachedHead=false fetch -q --depth 1 origin "$COMMIT"
    git -C "$VR" -c advice.detachedHead=false checkout -q -f "$COMMIT"
fi
port_version() { sed -n 's/.*"version[a-z-]*": *"\([^"]*\)".*/\1/p' "$VR/ports/$1/vcpkg.json" | head -1; }
z=$(port_version zlib)
h=$(port_version hunspell)
echo "vcpkg ports at $COMMIT: zlib $z, hunspell $h"
[ "$z" = "$ZLIB" ] || { echo "win-vcpkg.sh: vcpkg's zlib port is $z, not $ZLIB" >&2; exit 1; }
[ "$h" = "$HUNSPELL" ] || { echo "win-vcpkg.sh: vcpkg's hunspell port is $h, not $HUNSPELL" >&2; exit 1; }
if [ "${1:-}" = tool ]; then
    # the ports of a newer commit may need a newer vcpkg program than the image's (scripts/vcpkg-tool-metadata.txt)
    powershell -NoProfile -ExecutionPolicy Bypass -File "$(cygpath -w "$VR/scripts/bootstrap.ps1")" -disableMetrics > /dev/null
    echo "vcpkg program: $("$VR/vcpkg.exe" version | head -1 | tr -d '\r')"
fi
