#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static long field(const char *key) {
    FILE *f = fopen("/proc/self/status", "r"); char l[256]; long v = -1;
    while (f && fgets(l, sizeof l, f)) if (!strncmp(l, key, strlen(key))) { v = atol(l + strlen(key) + 1); break; }
    if (f) fclose(f);
    return v;
}
static long flt(void){struct rusage r;getrusage(RUSAGE_SELF,&r);return r.ru_minflt;}
int main(void) {
    size_t sz = 128UL << 20; char *buf = malloc(sz); memset(buf, 7, sz);
    printf("parent: allocated and filled %zu MB, RssAnon = %ld kB\n", sz >> 20, field("RssAnon:"));
    fflush(stdout); double t = now(); pid_t p = fork();
    if (p == 0) {
        printf("child : right after fork()     RssAnon = %ld kB (own view), minor faults so far = %ld\n", field("RssAnon:"), flt());
        long f0 = flt(); for (size_t i = 0; i < sz; i += 4096) (void)*(volatile char *)(buf + i);
        printf("child : after READING all pages  minor faults added = %ld  (pages stay shared)\n", flt() - f0);
        f0 = flt(); double t1 = now(); for (size_t i = 0; i < sz; i += 4096) buf[i] = 9;
        printf("child : after WRITING all pages  minor faults added = %ld  (one copy-on-write fault per page), write time = %.3f s\n", flt() - f0, now() - t1);
        fflush(stdout); _exit(0);
    }
    double tf = now() - t; waitpid(p, 0, 0);
    printf("parent: fork() of a %zu MB process took %.3f ms (only page tables are copied)\n", sz >> 20, tf * 1000);
    return 0;
}
