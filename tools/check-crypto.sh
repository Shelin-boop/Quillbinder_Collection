#!/bin/bash
# Checks the encryption pack (0.99.0): builds tests/crypto_pack_check.cpp against the static libraries of tools/get-crypto.sh and
# runs it (RNP creates a key in memory, encrypts a buffer, decrypts it), then links the same libraries into a shared object
# (they are position-independent). Prints what the program is linked to (ldd): only
# the system's base libraries may be there (libstdc++, libm, libgcc_s, libc), nothing of the pack.
#   tools/check-crypto.sh [prefix]          (default build/crypto, or QB_CRYPTO_DIR)
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
P=${1:-${QB_CRYPTO_DIR:-$ROOT/build/crypto}}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
g++ -std=c++17 -O1 -fPIE -pie "$ROOT/tests/crypto_pack_check.cpp" -I"$P/include" -L"$P/lib" \
    -lrnp -lsexpp -lbotan-3 -ljson-c -lbz2 -lz -lpthread -o "$T/check"
"$T/check"
echo "linked to:"
ldd "$T/check"
g++ -std=c++17 -fPIC -shared -I"$P/include" "$ROOT/tests/crypto_pack_check.cpp" -o "$T/libcheck.so" \
    -Wl,--whole-archive -L"$P/lib" -lrnp -Wl,--no-whole-archive -lsexpp -lbotan-3 -ljson-c -lbz2 -lz
echo "the libraries go into a shared object too (position-independent)"
if ldd "$T/check" | grep -E 'librnp|libbotan|libjson-c|libbz2|libsexpp|libz\.so'; then
    echo "check-crypto: the program has a shared library of the pack: it must be linked statically" >&2
    exit 1
fi
