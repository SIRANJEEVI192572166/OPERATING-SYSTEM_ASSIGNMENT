#include <stdio.h>
#include <stdlib.h>
int main(void) {
    srand(42);
    const int N = 100; long size[100];
    for (int i = 0; i < N; i++) size[i] = 1 + rand() % 1000000;   /* 1 B .. ~1 MB */
    long ps[3] = {1024, 4096, 2L*1024*1024};
    const char *nm[3] = {"1 KB", "4 KB", "2 MB"};
    long total = 0; for (int i = 0; i < N; i++) total += size[i];
    printf("100 processes, random sizes 1..1,000,000 bytes (seed 42), total = %ld bytes\n\n", total);
    printf("%-9s %-14s %-18s %-16s %-10s\n", "Page", "Pages used", "Internal frag (B)", "Avg per process", "% of memory");
    for (int k = 0; k < 3; k++) {
        long pages = 0, frag = 0;
        for (int i = 0; i < N; i++) { long n = (size[i] + ps[k] - 1) / ps[k]; pages += n; frag += n * ps[k] - size[i]; }
        printf("%-9s %-14ld %-18ld %-16.1f %-10.3f\n", nm[k], pages, frag, (double)frag / N, 100.0 * frag / (pages * ps[k]));
    }
    return 0;
}
