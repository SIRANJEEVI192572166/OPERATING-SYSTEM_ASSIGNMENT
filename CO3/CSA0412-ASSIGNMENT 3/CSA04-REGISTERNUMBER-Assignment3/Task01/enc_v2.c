#include "enc.h"
const char *enc_version(void) { return "libenc v2 (XOR key 0x33, security fix)"; }
void encrypt_buf(char *buf, int n) {
    for (int i = 0; i < n; i++) buf[i] ^= 0x33;
}
