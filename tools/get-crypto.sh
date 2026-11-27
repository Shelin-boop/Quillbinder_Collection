#!/bin/bash
# Builds the encryption libraries built into the program (0.99.0): RNP (OpenPGP) with Botan 3 as its crypto backend,
# json-c and bzip2 (both of which RNP needs), as STATIC libraries compiled with -fPIC:
#   tools/get-crypto.sh [prefix]          (default build/crypto, or QB_CRYPTO_DIR)
# The program gets no new shared library from this: it links these archives (zlib too: Ubuntu's own libz.a is not
# position-independent and could not go into a shared object, so zlib is built here), so a program update still runs on
# the libraries of the AppImage it was installed on (the runtime id).
# Every source is a git tag checked by its commit (no archive download): a tag that moved makes the script stop.
# Needs git, cmake, ninja, python3, gcc/g++ and make (tools/cloud-setup.sh installs them).
# Takes about 15 to 25 minutes on two cores. A finished build with the same pins is reused.
# As get-whisper.sh: first the ready-made pack of this recipe (tools/deps-pack.sh, pinned in tools/deps-packs.txt),
# built from source only without one (QB_DEPS_PACK=0: always). QB_JOBS limits the build processes.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${1:-${QB_CRYPTO_DIR:-$ROOT/build/crypto}}
WORK=$ROOT/build/crypto-src
JOBS=${QB_JOBS:-$(nproc)}

# the pins: a tag and the commit it must point to (for an annotated tag: the commit inside it)
BOTAN_TAG=3.14.1
BOTAN_COMMIT=01af88c039fcfaedc418b1af4d6d3a139674e591
JSONC_TAG=json-c-0.19-20260627
JSONC_COMMIT=aa716cd8d663c976b99b0f30f102ee1d8ef63146
BZIP2_TAG=bzip2-1.0.8
BZIP2_COMMIT=6a8690fc8d26c815e798c588f796eabe9d684cf0
ZLIB_TAG=v1.4
ZLIB_COMMIT=99623ef7097352da7993ba614161fd6a311e046e
RNP_TAG=v0.18.1
RNP_COMMIT=aad1892e4d8423398a0b973bd0a7e4544c359afc
# RNP's own submodule (the S-expression library), at the commit RNP v0.18.1 names
SEXPP_COMMIT=c641a2f36520bab783657a58650d9fda548b9dec
# Botan's modules: the list of RNP's own CI for Botan 3 (rnp ci/botan3-modules, v0.18.1), minimized build; Botan 3.14 made
# "curve25519" an internal module (configure refuses it), the public one is "x25519"; since 3.7 RNP also asks for the
# "pcurves" modules of the curves it offers (the ones at the end of the list; pcurves_impl comes with them)
BOTAN_MODULES="aead aes auto_rng bigint blowfish camellia cast128 cbc cfb crc24 x25519 des dl_algo dl_group dsa eax ecc_key ecdh ecdsa ed25519 elgamal eme_pkcs1 emsa_pkcs1 emsa_raw ffi hash raw_hash hmac hmac_drbg idea kdf md5 ocb pgp_s2k rfc3394 rmd160 rsa sha1 sha2_32 sha2_64 sha3 sm2 sm3 sm4 sp800_56a twofish pcurves_secp192r1 pcurves_secp256k1 pcurves_secp256r1 pcurves_secp384r1 pcurves_secp521r1 pcurves_brainpool256r1 pcurves_brainpool384r1 pcurves_brainpool512r1 pcurves_sm2p256v1"
STAMP="botan $BOTAN_TAG $BOTAN_COMMIT; json-c $JSONC_TAG $JSONC_COMMIT; bzip2 $BZIP2_TAG $BZIP2_COMMIT; zlib $ZLIB_TAG $ZLIB_COMMIT; rnp $RNP_TAG $RNP_COMMIT; sexpp $SEXPP_COMMIT"

done_here() { [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && [ -f "$PREFIX/lib/librnp.a" ] && [ -f "$PREFIX/lib/libbotan-3.a" ]; }
if done_here; then
    echo "the encryption libraries are there: $PREFIX (RNP $RNP_TAG, Botan $BOTAN_TAG)"
    exit 0
fi
if "$ROOT/tools/deps-pack.sh" fetch crypto "$PREFIX" && done_here; then
    echo "the encryption libraries are there: $PREFIX (RNP $RNP_TAG, Botan $BOTAN_TAG)"
    exit 0
fi

clone() {  # <dir> <url> <tag> <commit>
    if [ "$(git -C "$WORK/$1" rev-parse HEAD 2>/dev/null)" != "$4" ]; then
        rm -rf "${WORK:?}/$1"
        git clone -q --depth 1 --branch "$3" "$2" "$WORK/$1"
    fi
    test "$(git -C "$WORK/$1" rev-parse 'HEAD^{commit}')" = "$4" || { echo "get-crypto: the tag $3 of $2 is not the commit $4" >&2; exit 1; }
}
mkdir -p "$WORK"
clone botan https://github.com/randombit/botan "$BOTAN_TAG" "$BOTAN_COMMIT"
clone json-c https://github.com/json-c/json-c "$JSONC_TAG" "$JSONC_COMMIT"
clone bzip2 https://github.com/libarchive/bzip2 "$BZIP2_TAG" "$BZIP2_COMMIT"   # (the libarchive project's mirror of bzip2 1.0.8)
clone zlib https://github.com/madler/zlib "$ZLIB_TAG" "$ZLIB_COMMIT"
clone rnp https://github.com/rnpgp/rnp "$RNP_TAG" "$RNP_COMMIT"
if [ "$(git -C "$WORK/sexpp" rev-parse HEAD 2>/dev/null)" != "$SEXPP_COMMIT" ]; then
    rm -rf "$WORK/sexpp"
    git init -q "$WORK/sexpp"
    git -C "$WORK/sexpp" fetch -q --depth 1 https://github.com/rnpgp/sexpp.git "$SEXPP_COMMIT"
    git -C "$WORK/sexpp" checkout -q FETCH_HEAD
fi
test "$(git -C "$WORK/sexpp" rev-parse HEAD)" = "$SEXPP_COMMIT"
# RNP wants sexpp in src/libsexpp (a submodule: not part of the clone); it must be the one RNP v0.18.1 names
test "$(git -C "$WORK/rnp" ls-tree HEAD src/libsexpp | awk '{print $3}')" = "$SEXPP_COMMIT"
rm -rf "$WORK/rnp/src/libsexpp"
cp -a "$WORK/sexpp" "$WORK/rnp/src/libsexpp"
rm -rf "$WORK/rnp/src/libsexpp/.git"

# QB_CRYPTO_RESUME=1 (for working on this script): keep a finished part of an earlier, unfinished run (its marker .done-<part>)
if [ "${QB_CRYPTO_RESUME:-0}" != 1 ]; then
    rm -rf "$PREFIX"
fi
rm -rf "$WORK/b-json-c" "$WORK/b-rnp"
mkdir -p "$PREFIX/lib" "$PREFIX/include" "$PREFIX/licenses"
part_done() { [ "${QB_CRYPTO_RESUME:-0}" = 1 ] && [ -f "$PREFIX/.done-$1" ]; }

if ! part_done botan; then
# Botan: static library, minimized to the modules RNP needs, position-independent, no documentation
(cd "$WORK/botan" && python3 ./configure.py --prefix="$PREFIX" --build-targets=static --disable-shared-library \
    --minimized-build --enable-modules="$(echo $BOTAN_MODULES | tr ' ' ',')" --without-documentation \
    --extra-cxxflags=-fPIC --cc=gcc >/dev/null
 make -j"$JOBS" libs
 make install >/dev/null)
touch "$PREFIX/.done-botan"
fi

# json-c
if ! part_done json-c; then
cmake -S "$WORK/json-c" -B "$WORK/b-json-c" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_APPS=OFF \
    -DDISABLE_EXTRA_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON >/dev/null
cmake --build "$WORK/b-json-c" -j"$JOBS"
cmake --install "$WORK/b-json-c" >/dev/null
touch "$PREFIX/.done-json-c"
fi

# bzip2: the static library only (the Makefile's libbz2.a, with -fPIC)
if ! part_done bzip2; then
(cd "$WORK/bzip2" && make clean >/dev/null 2>&1 || true
 make -j"$JOBS" libbz2.a CC=gcc CFLAGS="-O2 -fPIC -D_FILE_OFFSET_BITS=64 -Wall"
 cp libbz2.a "$PREFIX/lib/" && cp bzlib.h "$PREFIX/include/")
touch "$PREFIX/.done-bzip2"
fi

# zlib: the static library only (./configure --static), with -fPIC
if ! part_done zlib; then
(cd "$WORK/zlib" && make distclean >/dev/null 2>&1 || true
 CFLAGS="-O2 -fPIC" ./configure --static --prefix="$PREFIX" >/dev/null
 make -j"$JOBS" libz.a
 make install >/dev/null)
touch "$PREFIX/.done-zlib"
fi

# RNP: static, Botan 3 backend, no crypto refresh, no post-quantum, no tests, no documentation; only the library
# (the "development" component: libraries, headers, CMake files; not the command line tools).
# RNP 0.18.1's crypto/mem.cpp calls strlen without including <cstring> (Botan 3.14's headers no longer bring it in): the
# compiler is told to include it, so that the source stays as it was tagged.
cmake -S "$WORK/rnp" -B "$WORK/b-rnp" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_PREFIX_PATH="$PREFIX" -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_CXX_FLAGS="-include cstring" -DCMAKE_C_FLAGS="" \
    -DCRYPTO_BACKEND=botan3 -DBOTAN_ROOT_DIR="$PREFIX" -DENABLE_CRYPTO_REFRESH=OFF -DENABLE_PQC=OFF -DBUILD_TESTING=OFF \
    -DENABLE_DOC=OFF -DDOWNLOAD_GTEST=OFF -DENABLE_COVERAGE=OFF -DENABLE_SANITIZERS=OFF -DENABLE_FUZZERS=OFF \
    -DBZIP2_INCLUDE_DIR="$PREFIX/include" -DBZIP2_LIBRARY_RELEASE="$PREFIX/lib/libbz2.a" \
    -DZLIB_INCLUDE_DIR="$PREFIX/include" -DZLIB_LIBRARY="$PREFIX/lib/libz.a" \
    -DJSON-C_INCLUDE_DIR="$PREFIX/include" -DJSON-C_LIBRARY="$PREFIX/lib/libjson-c.a" >/dev/null
cmake --build "$WORK/b-rnp" --target librnp -j"$JOBS"
cmake --install "$WORK/b-rnp" --component development >/dev/null

# the license texts of every part (the pack carries them)
cp "$WORK/botan/license.txt" "$PREFIX/licenses/LICENSE-botan.txt"
cp "$WORK/json-c/COPYING" "$PREFIX/licenses/LICENSE-json-c.txt"
cp "$WORK/bzip2/LICENSE" "$PREFIX/licenses/LICENSE-bzip2.txt"
cp "$WORK/rnp/LICENSE.md" "$PREFIX/licenses/LICENSE-rnp.txt"
cp "$WORK/rnp/LICENSE-OCB.md" "$PREFIX/licenses/LICENSE-rnp-OCB.txt"
cp "$WORK/sexpp/LICENSE.md" "$PREFIX/licenses/LICENSE-sexpp.txt"
cp "$WORK/zlib/LICENSE" "$PREFIX/licenses/LICENSE-zlib.txt"
rm -f "$PREFIX"/.done-*
echo "$STAMP" > "$PREFIX/version.txt"
echo "the encryption libraries are built: $PREFIX (RNP $RNP_TAG, Botan $BOTAN_TAG)"
ls "$PREFIX/lib"
