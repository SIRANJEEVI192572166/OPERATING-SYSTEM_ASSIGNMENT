#include <stdio.h>
/* same job with the C library: fopen/fread/fwrite, 1 byte per call */
int main(int argc, char **argv)
{
    FILE *in = fopen(argv[1], "rb"), *out = fopen("stdio.out", "wb");
    int c; long n = 0;
    char b;
    while (fread(&b, 1, 1, in) == 1) { fwrite(&b, 1, 1, out); n++; }
    fclose(in); fclose(out); remove("stdio.out");
    printf("fread/fwrite copy: %ld bytes processed with 1-byte library calls\n", n);
    return 0;
}
