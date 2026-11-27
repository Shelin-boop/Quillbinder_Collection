#!/bin/bash
# Fetches ONNX Runtime (MIT license) for Linux x64 for the engine packages of the AI add-ons (0.99.0), from Maven Central:
#   tools/get-onnxruntime.sh [prefix]          (default build/onnxruntime, or QB_ONNXRUNTIME_DIR)
# The prefix gets the headers (include/), libonnxruntime.so* (lib/), the license files (licenses/), README.txt (the source:
# Maven coordinates, version, SHA-256 of both files) and version.txt.
# The program never links this library: it loads it with dlopen and calls OrtGetApiBase, from an engine package that is
# installed later, so there is no link-time dependency on it. Name for dlopen: the full path of lib/libonnxruntime.so
# (a symlink to libonnxruntime.so.<version>; libonnxruntime.so.1 is a second symlink, for the library's own soname).
#
# Why Maven Central: the GitHub repository microsoft/onnxruntime is not public under that name any more (its release list
# answers 404, git asks for a login). Microsoft publishes the same library on Maven Central (the version also on PyPI):
#   com.microsoft.onnxruntime:onnxruntime:<v>          the JAR of the desktop Java package; it carries the native libraries per
#                                                     system (ai/onnxruntime/native/linux-x64/libonnxruntime.so; win-x64 has the
#                                                     DLL, which the Windows part will use) for the Linux library
#   com.microsoft.onnxruntime:onnxruntime-android:<v>  the AAR of Android: headers/onnxruntime_c_api.h and jni/<abi>/libonnxruntime.so,
#                                                     for the C headers (and later Android's libraries)
# While SHA256_JAR or SHA256_AAR is empty the download is accepted only with QB_ONNXRUNTIME_FIRST_RUN=1 (the workflow
# .github/workflows/deps.yml sets it). That run reads maven-metadata.xml of both packages, takes the newest plain release
# (digits and dots only) that both have, prints both versions, downloads both files, prints their file lists (the native
# libraries, the headers, the license files) and the line "pin this: VERSION=... SHA256_JAR=... SHA256_AAR=...": put those
# values below (the recipe's hash changes and the workflow builds the pack again, in seconds). With the pins the download is by
# VERSION and both SHA-256 decide. QB_MAVEN_URL changes the repository (the tests use a file:// folder).
# As get-whisper.sh: first the ready-made pack of this recipe (tools/deps-pack.sh, pinned in tools/deps-packs.txt), the
# download only without one (QB_DEPS_PACK=0: always).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${1:-${QB_ONNXRUNTIME_DIR:-$ROOT/build/onnxruntime}}
WORK=$ROOT/build/onnxruntime-src
MAVEN=${QB_MAVEN_URL:-https://repo1.maven.org/maven2}
GROUP_PATH=com/microsoft/onnxruntime
VERSION=1.32.0
SHA256_JAR="5f14563a52ac9dd5166c18b18ce837fd7589d4834e76e4e552c7b053d4b0538c"
SHA256_AAR="8e8dbb412ed4939675a157702f0c1e3621b436a645af7f7244536ac20bfad628"
STAMP="onnxruntime $VERSION linux-x64"

done_here() { [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && [ -f "$PREFIX/lib/libonnxruntime.so" ] && [ -f "$PREFIX/include/onnxruntime_c_api.h" ]; }
if done_here; then
    echo "ONNX Runtime is there: $PREFIX ($VERSION)"
    exit 0
fi
if "$ROOT/tools/deps-pack.sh" fetch onnxruntime "$PREFIX" && done_here; then
    echo "ONNX Runtime is there: $PREFIX ($VERSION)"
    exit 0
fi
FIRST=0
if [ -z "$SHA256_JAR" ] || [ -z "$SHA256_AAR" ]; then
    if [ "${QB_ONNXRUNTIME_FIRST_RUN:-0}" != 1 ]; then
        echo "get-onnxruntime: no SHA-256 is pinned yet for the JAR and the AAR: run the workflow deps.yml (it sets QB_ONNXRUNTIME_FIRST_RUN=1), then pin the line it prints" >&2
        exit 1
    fi
    FIRST=1
fi
mkdir -p "$WORK"
cd "$WORK"
if [ "$FIRST" = 1 ]; then
    for a in onnxruntime onnxruntime-android; do
        curl -fsSL -o "metadata-$a.xml" "$MAVEN/$GROUP_PATH/$a/maven-metadata.xml"
    done
    # the newest plain release that both packages have (a version with a word in it is a pre-release)
    VERSION=$(python3 -I - metadata-onnxruntime.xml metadata-onnxruntime-android.xml <<'PY'
import re, sys
sets = []
for f in sys.argv[1:]:
    vs = re.findall(r"<version>([^<]+)</version>", open(f, encoding="utf-8").read())
    plain = {v.strip() for v in vs if re.fullmatch(r"\d+(\.\d+)*", v.strip())}
    print("get-onnxruntime: %s: %d versions, newest plain release %s" % (f[9:-4], len(plain),
          max(plain, key=lambda v: tuple(map(int, v.split(".")))) if plain else "none"), file=sys.stderr)
    sets.append(plain)
both = sets[0] & sets[1]
if not both:
    sys.exit("no plain release is in both packages")
print(max(both, key=lambda v: tuple(map(int, v.split(".")))))
PY
    )
    echo "get-onnxruntime: chosen version $VERSION (newest plain release of both packages)"
fi
JAR=onnxruntime-$VERSION.jar
AAR=onnxruntime-android-$VERSION.aar
POM=onnxruntime-$VERSION.pom
if [ "$FIRST" = 1 ]; then rm -f "$JAR" "$AAR" "$POM"; fi
[ -f "$JAR" ] || curl -fsSL -o "$JAR" "$MAVEN/$GROUP_PATH/onnxruntime/$VERSION/$JAR"
[ -f "$AAR" ] || curl -fsSL -o "$AAR" "$MAVEN/$GROUP_PATH/onnxruntime-android/$VERSION/$AAR"
[ -f "$POM" ] || curl -fsSL -o "$POM" "$MAVEN/$GROUP_PATH/onnxruntime/$VERSION/$POM"
GOT_JAR=$(sha256sum "$JAR" | cut -d' ' -f1)
GOT_AAR=$(sha256sum "$AAR" | cut -d' ' -f1)
if [ "$FIRST" = 0 ]; then
    for p in "$JAR $GOT_JAR $SHA256_JAR" "$AAR $GOT_AAR $SHA256_AAR"; do
        set -- $p
        [ "$2" = "$3" ] || { echo "get-onnxruntime: $1 has the SHA-256 $2, not the pinned $3" >&2; rm -f "$1"; exit 1; }
    done
fi
# what is really in the files decides
echo "get-onnxruntime: JAR file list (native libraries, headers, licenses):"
unzip -Z1 "$JAR" | grep -E 'native/|\.h$|LICENSE|NOTICE|ThirdParty|Privacy' | sed 's/^/get-onnxruntime:   jar: /' || true
echo "get-onnxruntime: AAR file list (libraries, headers, licenses):"
unzip -Z1 "$AAR" | grep -E '\.so$|\.h$|LICENSE|NOTICE|ThirdParty|Privacy' | sed 's/^/get-onnxruntime:   aar: /' || true
if [ "$FIRST" = 1 ]; then
    echo "get-onnxruntime: no SHA-256 pinned yet (first run)."
    echo "pin this: VERSION=$VERSION SHA256_JAR=\"$GOT_JAR\" SHA256_AAR=\"$GOT_AAR\""
fi
LIBENTRY=$(unzip -Z1 "$JAR" | grep -E '(^|/)linux-x64/libonnxruntime\.so$' | head -1 || true)
HDRENTRY=$(unzip -Z1 "$AAR" | grep -E '(^|/)onnxruntime_c_api\.h$' | head -1 || true)
[ -n "$LIBENTRY" ] || { echo "get-onnxruntime: the JAR has no linux-x64/libonnxruntime.so" >&2; exit 1; }
[ -n "$HDRENTRY" ] || { echo "get-onnxruntime: the AAR has no onnxruntime_c_api.h" >&2; exit 1; }
echo "get-onnxruntime: Linux library: $LIBENTRY (JAR); headers: ${HDRENTRY%/*}/ (AAR)"
rm -rf unpacked && mkdir -p unpacked/jar unpacked/aar
unzip -q -o "$JAR" "$LIBENTRY" -d unpacked/jar
HDIR=${HDRENTRY%/*}
unzip -q -o "$AAR" "$HDIR/*" -d unpacked/aar
# license files the two archives carry (any LICENSE, notices or privacy text at any depth)
rm -rf "$PREFIX"
mkdir -p "$PREFIX/licenses" "$PREFIX/lib" "$PREFIX/include"
cp -a "unpacked/aar/$HDIR"/. "$PREFIX/include/"
cp "unpacked/jar/$LIBENTRY" "$PREFIX/lib/libonnxruntime.so.$VERSION"
chmod 755 "$PREFIX/lib/libonnxruntime.so.$VERSION"
ln -s "libonnxruntime.so.$VERSION" "$PREFIX/lib/libonnxruntime.so.${VERSION%%.*}"
ln -s "libonnxruntime.so.$VERSION" "$PREFIX/lib/libonnxruntime.so"
LICSRC=none
for z in "$JAR" "$AAR"; do
    while IFS= read -r e; do
        base=$(basename "$e")
        case "$base" in LICENSE*|ThirdPartyNotices*|NOTICE*|Privacy*) ;; *) continue ;; esac
        unzip -p "$z" "$e" > "$PREFIX/licenses/$base-onnxruntime-$([ "$z" = "$JAR" ] && echo jar || echo aar)"
        LICSRC=archive
    done < <(unzip -Z1 "$z" | grep -v '/$' | grep -E '(^|/)(LICENSE|ThirdPartyNotices|NOTICE|Privacy)' || true)
done
if [ "$LICSRC" = none ]; then
    # neither file carries a license text: the POM's license entry names the MIT license, whose text is written here
    grep -q '<name>MIT' "$POM" || { echo "get-onnxruntime: no license file in the JAR or the AAR and no MIT entry in the POM" >&2; exit 1; }
    POMLIC=$(tr -d '\n' < "$POM" | sed -n 's|.*<license>\(.*\)</license>.*|\1|p' | sed 's/<[^>]*>/ /g; s/  */ /g')
    cat > "$PREFIX/licenses/LICENSE-onnxruntime.txt" <<LIC
ONNX Runtime $VERSION: the JAR and the AAR carry no license file. The POM of com.microsoft.onnxruntime:onnxruntime:$VERSION
names the license:$POMLIC
The text of the MIT license (as used by ONNX Runtime):

MIT License

Copyright (c) Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
LIC
fi
cat > "$PREFIX/README.txt" <<README
ONNX Runtime $VERSION for Linux x64, unchanged, from Maven Central:
  com.microsoft.onnxruntime:onnxruntime:$VERSION         ($JAR, SHA-256 $GOT_JAR)
      lib/libonnxruntime.so.$VERSION is $LIBENTRY of this JAR
  com.microsoft.onnxruntime:onnxruntime-android:$VERSION ($AAR, SHA-256 $GOT_AAR)
      include/ is ${HDIR}/ of this AAR (the C headers)
lib/libonnxruntime.so is a symlink to libonnxruntime.so.$VERSION; load it with dlopen by its full path and call OrtGetApiBase.
License: MIT (Microsoft). $( [ "$LICSRC" = archive ] && echo "The license files of the JAR and the AAR are in licenses/." || echo "Neither file carries a license text: licenses/LICENSE-onnxruntime.txt is the MIT text named by the POM's license entry." )
Fetched by tools/get-onnxruntime.sh of Quillbinder.
README
echo "onnxruntime $VERSION linux-x64" > "$PREFIX/version.txt"
# the library must need nothing outside the base system
echo "get-onnxruntime: NEEDED libraries of lib/libonnxruntime.so.$VERSION (objdump -p):"
objdump -p "$PREFIX/lib/libonnxruntime.so.$VERSION" | awk '/NEEDED|SONAME/ {print "get-onnxruntime:   " $1 " " $2}'
echo "ONNX Runtime is in $PREFIX ($VERSION)"
ls -l "$PREFIX/lib" "$PREFIX/licenses"
