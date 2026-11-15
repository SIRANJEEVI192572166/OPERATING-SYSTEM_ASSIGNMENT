#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#define PG 4096UL
#define BYTES (96UL << 20)
static long long ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000000000LL+t.tv_nsec;}
static long flt(void){struct rusage r;getrusage(RUSAGE_SELF,&r);return r.ru_minflt;}
int main(int argc, char **argv) {
    int lock = argc > 1 && !strcmp(argv[1], "lock");
    if (lock) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) { perror("mlockall"); return 1; }
    }
    char *buf = mmap(0, BYTES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (lock) memset(buf, 0, BYTES);            /* pre-fault while locked: all pages resident BEFORE the real-time phase */
    size_t n = BYTES / PG; long long worst = 0, sum = 0; long f0 = flt();
    for (size_t i = 0; i < n; i++) {             /* time-critical loop: one page of work per cycle */
        long long t0 = ns(); buf[i * PG] += 1; long long d = ns() - t0;
        if (d > worst) worst = d;
        sum += d;
    }
    printf("%-9s cycles=%zu  page faults in loop=%ld  mean=%lld ns  WORST-CASE delay=%lld ns\n",
           lock ? "mlockall" : "no lock", n, flt() - f0, sum / (long long)n, worst);
    return 0;
}
