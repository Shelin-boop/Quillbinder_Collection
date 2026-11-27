#!/usr/bin/env bash
# The encryption libraries for the Windows build (probe of 0.99.0): the Windows counterpart of tools/get-crypto.sh, with
# the same pins: Botan 3.14.1 (minimized to RNP's modules), json-c 0.19, bzip2 1.0.8, RNP 0.18.1 (with its sexpp), all as
# STATIC libraries built with MSVC and the dynamic C++ runtime (/MD, as vcpkg's triplet x64-windows-static-md). zlib is
# not built here: it is the one vcpkg built for the program (1.4; packaging/windows/win-vcpkg.sh).
#
#   packaging/windows/win-crypto.sh <prefix>
#
# Runs in Git Bash inside MSVC's environment (packaging/windows/win-ci.sh); needs git, cmake, ninja, python and the
# vcpkg zlib. The workflow keeps the result in a cache under the hash of this script (no pack of the recipe: a pack's
# recipe in win-deps-pack.sh would change that script's hash, which the Hunspell pack's recipe holds).
# Result: <prefix>/lib/*.lib, <prefix>/include (botan-3/, rnp/, json-c/, bzlib.h), <prefix>/licenses, <prefix>/version.txt.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
PREFIX=$1
WORK="$PWD/.crypto-work"
JOBS=${QB_JOBS:-$(nproc)}
TRIPLET=${VCPKG_TRIPLET:-x64-windows-static-md}

BOTAN_TAG=3.14.1
BOTAN_COMMIT=01af88c039fcfaedc418b1af4d6d3a139674e591
JSONC_TAG=json-c-0.19-20260627
JSONC_COMMIT=aa716cd8d663c976b99b0f30f102ee1d8ef63146
BZIP2_TAG=bzip2-1.0.8
BZIP2_COMMIT=6a8690fc8d26c815e798c588f796eabe9d684cf0
RNP_TAG=v0.18.1
RNP_COMMIT=aad1892e4d8423398a0b973bd0a7e4544c359afc
SEXPP_COMMIT=c641a2f36520bab783657a58650d9fda548b9dec
# the module list of tools/get-crypto.sh (RNP's own list for Botan 3 and the pcurves modules)
BOTAN_MODULES="aead aes auto_rng bigint blowfish camellia cast128 cbc cfb crc24 x25519 des dl_algo dl_group dsa eax ecc_key ecdh ecdsa ed25519 elgamal eme_pkcs1 emsa_pkcs1 emsa_raw ffi hash raw_hash hmac hmac_drbg idea kdf md5 ocb pgp_s2k rfc3394 rmd160 rsa sha1 sha2_32 sha2_64 sha3 sm2 sm3 sm4 sp800_56a twofish pcurves_secp192r1 pcurves_secp256k1 pcurves_secp256r1 pcurves_secp384r1 pcurves_secp521r1 pcurves_brainpool256r1 pcurves_brainpool384r1 pcurves_brainpool512r1 pcurves_sm2p256v1"

ZI="$(cygpath -u "${VCPKG_INSTALLATION_ROOT:?vcpkg is not on this machine}")/installed/$TRIPLET"
ZLIB=""
for n in libzs.lib zs.lib zlib.lib zlibstatic.lib; do [ -f "$ZI/lib/$n" ] && { ZLIB="$ZI/lib/$n"; break; }; done
[ -f "$ZI/include/zlib.h" ] && [ -n "$ZLIB" ] || { echo "win-crypto.sh: no zlib in $ZI (the vcpkg step first)" >&2; exit 1; }
ZV=$(sed -n 's/^#define ZLIB_VERSION "\(.*\)".*/\1/p' "$ZI/include/zlib.h" | head -1)
MSVC=${VCToolsVersion:-unknown}
STAMP="botan $BOTAN_TAG $BOTAN_COMMIT; json-c $JSONC_TAG $JSONC_COMMIT; bzip2 $BZIP2_TAG $BZIP2_COMMIT; rnp $RNP_TAG $RNP_COMMIT; sexpp $SEXPP_COMMIT; zlib $ZV; msvc $(echo "$MSVC" | cut -d. -f1,2); script $(sha256sum "$0" | cut -c1-16)"
if [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && ls "$PREFIX"/lib/librnp.lib "$PREFIX"/lib/botan-3*.lib > /dev/null 2>&1; then
    echo "the encryption libraries are there: $PREFIX"
    exit 0
fi

clone() {  # <dir> <url> <tag> <commit>
    rm -rf "${WORK:?}/$1"
    git clone -q --depth 1 --branch "$3" "$2" "$WORK/$1"
    test "$(git -C "$WORK/$1" rev-parse 'HEAD^{commit}')" = "$4" || { echo "win-crypto: the tag $3 of $2 is not the commit $4" >&2; exit 1; }
}
rm -rf "$WORK" "$PREFIX"
mkdir -p "$WORK" "$PREFIX/lib" "$PREFIX/include" "$PREFIX/licenses"
clone botan https://github.com/randombit/botan "$BOTAN_TAG" "$BOTAN_COMMIT"
clone json-c https://github.com/json-c/json-c "$JSONC_TAG" "$JSONC_COMMIT"
clone bzip2 https://github.com/libarchive/bzip2 "$BZIP2_TAG" "$BZIP2_COMMIT"
clone rnp https://github.com/rnpgp/rnp "$RNP_TAG" "$RNP_COMMIT"
git init -q "$WORK/sexpp"
git -C "$WORK/sexpp" fetch -q --depth 1 https://github.com/rnpgp/sexpp.git "$SEXPP_COMMIT"
git -C "$WORK/sexpp" checkout -q FETCH_HEAD
test "$(git -C "$WORK/sexpp" rev-parse HEAD)" = "$SEXPP_COMMIT"
test "$(git -C "$WORK/rnp" ls-tree HEAD src/libsexpp | awk '{print $3}')" = "$SEXPP_COMMIT"
rm -rf "$WORK/rnp/src/libsexpp"
cp -a "$WORK/sexpp" "$WORK/rnp/src/libsexpp"
rm -rf "$WORK/rnp/src/libsexpp/.git"

PY=$(command -v python3 || command -v python)
P=$(cygpath -m "$PREFIX")

# ---- Botan: static, minimized, MSVC with /MD, built with Ninja
echo "== Botan"
# Botan 3.14.1's ffi_util.h line 291 (botan_ffi_visit) does not compile with MSVC 14.52 (run 46026018575: C2512 "no
# appropriate default constructor" on every FFI file that visits an object): the same test written with std::invoke_result_t,
# which means the same for a generic lambda. The only change to the source; the script stops when the line is not there.
FFU="$WORK/botan/src/lib/ffi/ffi_util.h"
grep -qF 'std::is_void_v<decltype(func(ref))>' "$FFU" || { echo "win-crypto: ffi_util.h is not as expected" >&2; exit 1; }
sed -i 's/std::is_void_v<decltype(func(ref))>/std::is_void_v<std::invoke_result_t<VisitorFnT\&, std::remove_reference_t<decltype(ref)>\&>>/' "$FFU"
grep -n 'invoke_result_t' "$FFU"
( cd "$WORK/botan"
  "$PY" ./configure.py --prefix="$P" --os=windows --cpu=x86_64 --cc=msvc --msvc-runtime=MD --build-targets=static \
      --disable-shared-library --minimized-build --enable-modules="$(echo $BOTAN_MODULES | tr ' ' ',')" \
      --without-documentation --build-tool=ninja > configure.log 2>&1 || { tail -30 configure.log; exit 1; }
  ninja libs > build.log 2>&1 || { grep -E 'error|FAILED' build.log | head -30; tail -10 build.log; exit 1; }
  ninja install > install.log 2>&1 || { tail -20 install.log; exit 1; } )
echo "Botan installed: $(ls "$PREFIX/lib" | tr '\n' ' ')"

# ---- json-c: static (its CMake build; the static library is json-c.lib or json-c-static.lib)
echo "== json-c"
cmake -S "$WORK/json-c" -B "$WORK/b-json-c" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$P" \
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF \
    -DBUILD_APPS=OFF -DDISABLE_EXTRA_LIBS=ON > "$WORK/json-c.log" 2>&1 || { tail -30 "$WORK/json-c.log"; exit 1; }
cmake --build "$WORK/b-json-c" -j"$JOBS" >> "$WORK/json-c.log" 2>&1 || { grep -E 'error|FAILED' "$WORK/json-c.log" | head -30; exit 1; }
cmake --install "$WORK/b-json-c" >> "$WORK/json-c.log" 2>&1
JSONC_LIB=""
for n in json-c.lib json-c-static.lib; do [ -f "$PREFIX/lib/$n" ] && { JSONC_LIB="$P/lib/$n"; break; }; done
[ -n "$JSONC_LIB" ] || { echo "win-crypto: no json-c library in $PREFIX/lib: $(ls "$PREFIX/lib" | tr '\n' ' ')" >&2; exit 1; }

# ---- bzip2: the library's seven files (the project's makefile.msc builds more than needed), /MD, as bz2.lib
echo "== bzip2"
mkdir -p "$WORK/b-bzip2"
( cd "$WORK/b-bzip2"
  for f in blocksort bzlib compress crctable decompress huffman randtable; do
      cl.exe -nologo -c -O2 -MD -D_FILE_OFFSET_BITS=64 -D_CRT_SECURE_NO_WARNINGS "$(cygpath -m "$WORK/bzip2/$f.c")" || exit 1
  done
  lib.exe -nologo -out:bz2.lib ./*.obj ) || { echo "win-crypto: bzip2 did not build" >&2; exit 1; }
cp "$WORK/b-bzip2/bz2.lib" "$PREFIX/lib/"
cp "$WORK/bzip2/bzlib.h" "$PREFIX/include/"

# ---- RNP: static, Botan 3 backend; "-include cstring" is MSVC's -FI (dash form: Git Bash turns a leading slash into a path, run 46029274249) (see tools/get-crypto.sh), -DRNP_STATIC is what the
# program's own compile needs too (the header says dllimport without it)
echo "== RNP"
# (RNP's command line tools need getopt and dirent on Windows and its CMake stops without them; only the library is built,
# so the three variables get a harmless value)
cmake -S "$WORK/rnp" -B "$WORK/b-rnp" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$P" \
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DCMAKE_PREFIX_PATH="$P" -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_FLAGS="-FIcstring -EHsc -DRNP_STATIC" -DCMAKE_C_FLAGS="-DRNP_STATIC" \
    -DCRYPTO_BACKEND=botan3 -DBOTAN_ROOT_DIR="$P" -DENABLE_CRYPTO_REFRESH=OFF -DENABLE_PQC=OFF -DBUILD_TESTING=OFF \
    -DENABLE_DOC=OFF -DDOWNLOAD_GTEST=OFF -DENABLE_COVERAGE=OFF -DENABLE_SANITIZERS=OFF -DENABLE_FUZZERS=OFF \
    -DBZIP2_INCLUDE_DIR="$P/include" -DBZIP2_LIBRARY_RELEASE="$P/lib/bz2.lib" \
    -DZLIB_INCLUDE_DIR="$(cygpath -m "$ZI/include")" -DZLIB_LIBRARY="$(cygpath -m "$ZLIB")" \
    -DGETOPT_INCLUDE_DIR="$P/include" -DDIRENT_INCLUDE_DIR="$P/include" -DGETOPT_LIBRARY="$P/lib/bz2.lib" \
    -DJSON-C_INCLUDE_DIR="$P/include/json-c" -DJSON-C_LIBRARY="$JSONC_LIB" > "$WORK/rnp.log" 2>&1 || { tail -40 "$WORK/rnp.log"; exit 1; }
cmake --build "$WORK/b-rnp" --target librnp -j"$JOBS" >> "$WORK/rnp.log" 2>&1 || { grep -E 'error|FAILED' "$WORK/rnp.log" | head -30; exit 1; }
cmake --install "$WORK/b-rnp" --component development >> "$WORK/rnp.log" 2>&1 || { tail -20 "$WORK/rnp.log"; exit 1; }

cp "$WORK/botan/license.txt" "$PREFIX/licenses/LICENSE-botan.txt"
cp "$WORK/json-c/COPYING" "$PREFIX/licenses/LICENSE-json-c.txt"
cp "$WORK/bzip2/LICENSE" "$PREFIX/licenses/LICENSE-bzip2.txt"
cp "$WORK/rnp/LICENSE.md" "$PREFIX/licenses/LICENSE-rnp.txt"
cp "$WORK/rnp/LICENSE-OCB.md" "$PREFIX/licenses/LICENSE-rnp-OCB.txt"
cp "$WORK/sexpp/LICENSE.md" "$PREFIX/licenses/LICENSE-sexpp.txt"
ls "$PREFIX/lib/librnp.lib" "$PREFIX"/lib/botan-3*.lib > /dev/null
echo "$STAMP" > "$PREFIX/version.txt"
rm -rf "$WORK"
echo "the encryption libraries are built: $PREFIX"
ls "$PREFIX/lib"
