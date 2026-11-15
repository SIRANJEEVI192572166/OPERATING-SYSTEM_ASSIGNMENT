#include "enc.h"
const char *enc_version(void) { return "libenc v1 (XOR key 0x5A)"; }
void encrypt_buf(char *buf, int n) {
    for (int i = 0; i < n; i++) buf[i] ^= 0x5A;
}
