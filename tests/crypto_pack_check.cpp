// A check of the encryption pack (tools/check-crypto.sh builds and runs it against a prefix made by tools/get-crypto.sh):
// RNP with Botan creates a key in memory, encrypts a buffer (compressed with bzip2, so that bzip2 is exercised too) and
// decrypts it again. Prints one line and returns 0 when the text came back unchanged.
#include <rnp/rnp.h>
#include <cstdio>
#include <cstring>
#include <string>

int main() {
    rnp_ffi_t ffi = nullptr;
    if (rnp_ffi_create(&ffi, "GPG", "GPG")) return 1;
    const char *spec = "{\"primary\":{\"type\":\"EDDSA\",\"userid\":\"check\"},\"sub\":{\"type\":\"ECDH\",\"curve\":\"Curve25519\"}}";
    char *keys = nullptr;
    if (rnp_generate_key_json(ffi, spec, &keys)) return 2;
    rnp_buffer_destroy(keys);
    rnp_key_handle_t key = nullptr;
    if (rnp_locate_key(ffi, "userid", "check", &key)) return 3;
    const std::string text = "a text that goes in and has to come out again, unchanged";
    rnp_input_t in = nullptr;
    rnp_output_t out = nullptr;
    rnp_input_from_memory(&in, reinterpret_cast<const uint8_t *>(text.data()), text.size(), false);
    rnp_output_to_memory(&out, 0);
    rnp_op_encrypt_t enc = nullptr;
    if (rnp_op_encrypt_create(&enc, ffi, in, out) || rnp_op_encrypt_add_recipient(enc, key)) return 4;
    rnp_op_encrypt_set_compression(enc, "BZIP2", 6);
    rnp_op_encrypt_set_armor(enc, true);
    if (rnp_op_encrypt_execute(enc)) return 5;
    uint8_t *buf = nullptr;
    size_t len = 0;
    rnp_output_memory_get_buf(out, &buf, &len, false);
    const std::string cipher(reinterpret_cast<char *>(buf), len);
    rnp_op_encrypt_destroy(enc);
    rnp_input_destroy(in);
    rnp_output_destroy(out);
    rnp_input_t cin = nullptr;
    rnp_output_t pout = nullptr;
    rnp_input_from_memory(&cin, reinterpret_cast<const uint8_t *>(cipher.data()), cipher.size(), false);
    rnp_output_to_memory(&pout, 0);
    if (rnp_decrypt(ffi, cin, pout)) return 6;
    rnp_output_memory_get_buf(pout, &buf, &len, false);
    const bool same = std::string(reinterpret_cast<char *>(buf), len) == text && cipher.find("BEGIN PGP MESSAGE") != std::string::npos;
    std::printf("RNP %s with Botan: key made, %zu bytes encrypted, decrypted text %s\n", rnp_version_string(), cipher.size(),
                same ? "unchanged" : "DIFFERENT");
    rnp_input_destroy(cin);
    rnp_output_destroy(pout);
    rnp_key_handle_destroy(key);
    rnp_ffi_destroy(ffi);
    return same ? 0 : 7;
}
