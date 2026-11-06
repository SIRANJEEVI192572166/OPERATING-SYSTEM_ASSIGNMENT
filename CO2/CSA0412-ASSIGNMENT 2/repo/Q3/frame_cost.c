#include "work.h"
int main(void)
{
    double t0 = now(); long s = 0;
    for (int c = 0; c < CAMERAS; c++) { unsigned long h = 2166136261UL + (unsigned long)c; for (unsigned long i = 0; i < CPU_ITERS; i++) h = (h ^ i) * 16777619UL; s += (int)(h % 5) + 1; }
    double cpu = now() - t0; t0 = now();
    for (int c = 0; c < CAMERAS; c++) nanosleep(&(struct timespec){0, IO_NS}, NULL);
    double io = now() - t0;
    printf("CPU work for 64 frames : %.1f ms (%.2f ms per frame)\nI/O wait for 64 frames : %.1f ms (%.2f ms per frame)\nTheoretical speed-up limit on 1 CPU = (CPU+I/O)/CPU = %.2f   (plates checksum %ld)\n", cpu*1e3, cpu*1e3/64, io*1e3, io*1e3/64, (cpu+io)/cpu, s);
    return 0;
}
