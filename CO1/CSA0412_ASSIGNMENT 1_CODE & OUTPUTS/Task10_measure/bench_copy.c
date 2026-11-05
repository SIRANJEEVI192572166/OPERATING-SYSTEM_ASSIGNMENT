#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
/* usage: bench_copy <file> <bufsize> : copy file with read()/write(); print calls and elapsed ms */
int main(int argc, char **argv)
{
    size_t bs = (size_t)atol(argv[2]);
    char *buf = malloc(bs);
    int in = open(argv[1], O_RDONLY), out = open("bench.out", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    struct timespec a, b; clock_gettime(CLOCK_MONOTONIC, &a);
    long rd = 0; ssize_t n;
    while ((n = read(in, buf, bs)) > 0) { rd++; if (write(out, buf, n) != n) return 1; }
    clock_gettime(CLOCK_MONOTONIC, &b);
    printf("bufsize %7zu : %9ld read() calls, %8.1f ms\n", bs, rd,
           (b.tv_sec-a.tv_sec)*1e3 + (b.tv_nsec-a.tv_nsec)/1e6);
    close(in); close(out); unlink("bench.out"); return 0;
}
