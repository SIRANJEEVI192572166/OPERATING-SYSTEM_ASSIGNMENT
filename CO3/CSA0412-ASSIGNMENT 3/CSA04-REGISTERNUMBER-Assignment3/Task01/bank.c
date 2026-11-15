#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "enc.h"

static void show_maps(void) {
    FILE *f = fopen("/proc/self/maps", "r");
    char line[512];
    while (f && fgets(line, sizeof line, f))
        if (strstr(line, "bank") || strstr(line, "libenc") || strstr(line, "libc.so") ||
            strstr(line, "ld-linux") || strstr(line, "[stack]") || strstr(line, "[heap]"))
            fputs(line, stdout);
    if (f) fclose(f);
}

int main(int argc, char **argv) {
    char msg[] = "ACCT:1001 BAL:50000";
    int n = (int)strlen(msg);
    encrypt_buf(msg, n);
    printf("[bank] %s\n[bank] cipher bytes:", enc_version());
    for (int i = 0; i < n; i++) printf(" %02x", (unsigned char)msg[i]);
    printf("\n");
    if (argc > 1 && strcmp(argv[1], "maps") == 0) show_maps();
    if (argc > 1 && strcmp(argv[1], "sleep") == 0) { pause(); }
    return 0;
}
