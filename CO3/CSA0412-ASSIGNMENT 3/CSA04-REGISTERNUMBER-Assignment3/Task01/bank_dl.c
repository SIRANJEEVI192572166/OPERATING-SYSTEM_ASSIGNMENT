#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

int main(int argc, char **argv) {
    char msg[] = "ACCT:1001 BAL:50000";
    int n = (int)strlen(msg);
    int need_crypto = (argc > 1 && strcmp(argv[1], "nocrypto") == 0) ? 0 : 1;
    if (!need_crypto) { printf("[bank_dl] encryption not needed - library never loaded\n"); return 0; }

    void *h = dlopen("./libenc.so", RTLD_NOW);
    if (!h) { fprintf(stderr, "[bank_dl] cannot load library: %s\n[bank_dl] falling back: refusing to send data in clear text\n", dlerror()); return 2; }
    void (*enc)(char *, int) = (void (*)(char *, int))dlsym(h, "encrypt_buf");
    const char *(*ver)(void) = (const char *(*)(void))dlsym(h, "enc_version");
    if (!enc || !ver) { fprintf(stderr, "[bank_dl] dlsym failed: %s\n", dlerror()); dlclose(h); return 3; }
    enc(msg, n);
    printf("[bank_dl] loaded %s at run time via dlopen()/dlsym()\n[bank_dl] cipher bytes:", ver());
    for (int i = 0; i < n; i++) printf(" %02x", (unsigned char)msg[i]);
    printf("\n");
    printf("[bank_dl] address of encrypt_buf = %p\n", (void *)enc);
    dlclose(h);
    return 0;
}
