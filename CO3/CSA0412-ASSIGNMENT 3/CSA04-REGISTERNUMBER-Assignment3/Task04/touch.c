#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#define PG 4096UL
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static long minflt(void){struct rusage r;getrusage(RUSAGE_SELF,&r);return r.ru_minflt;}
int main(int argc, char **argv) {
    size_t mb = argc > 1 ? strtoul(argv[1], 0, 10) : 64; size_t n = mb * 1024 * 1024 / PG;
    char *a = mmap(0, n * PG, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    long f0 = minflt(); double t0 = now();
    for (size_t i = 0; i < n; i++) a[i * PG] = 1;             /* sequential first touch */
    double t1 = now(); long f1 = minflt();
    munmap(a, n * PG);
    a = mmap(0, n * PG, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    srand(1); size_t *perm = malloc(n * sizeof *perm);
    for (size_t i = 0; i < n; i++) perm[i] = i;
    for (size_t i = n - 1; i > 0; i--) { size_t j = rand() % (i + 1); size_t t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
    long f2 = minflt(); double t2 = now();
    for (size_t i = 0; i < n; i++) a[perm[i] * PG] = 1;        /* random first touch */
    double t3 = now(); long f3 = minflt();
    printf("%4zu MB (%zu pages): sequential  faults=%ld  time=%.4f s | random  faults=%ld  time=%.4f s\n", mb, n, f1 - f0, t1 - t0, f3 - f2, t3 - t2);
    return 0;
}
