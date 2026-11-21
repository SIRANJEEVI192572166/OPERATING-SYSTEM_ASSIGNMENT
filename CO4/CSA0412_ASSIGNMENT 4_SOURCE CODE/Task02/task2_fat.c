#define _POSIX_C_SOURCE 199309L
/* Alternative concept: time to reach block k with an in-memory FAT (O(k)) versus an index (O(1)). */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
int main(void)
{
    unsigned sizes[4] = {1u << 10, 1u << 16, 1u << 20, 1u << 24};
    printf("%-12s %-22s %-22s\n", "File blocks", "FAT walk to last (ms)", "Index look-up (ms)");
    for (int s = 0; s < 4; s++) {
        unsigned n = sizes[s];
        unsigned *fat = malloc(n * sizeof *fat), *perm = malloc(n * sizeof *perm), *idx = malloc(n * sizeof *idx);
        if (!fat || !perm || !idx) return 1;
        for (unsigned i = 0; i < n; i++) perm[i] = i;
        srand(5);
        for (unsigned i = n - 1; i > 0; i--) { unsigned j = ((unsigned)rand() * 32768u + (unsigned)rand()) % (i + 1); unsigned t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
        for (unsigned i = 0; i + 1 < n; i++) { fat[perm[i]] = perm[i + 1]; idx[i] = perm[i]; }   /* fragmented chain */
        fat[perm[n - 1]] = 0xFFFFFFFFu; idx[n - 1] = perm[n - 1];
        double t0 = now(); unsigned b = perm[0];
        for (unsigned i = 0; i < n - 1; i++) b = fat[b];
        double t1 = now(); volatile unsigned r = 0; unsigned reps = 1000000;
        for (unsigned i = 0; i < reps; i++) r = idx[(i * 2654435761u) % n];
        double t2 = now(); (void)r;
        printf("%-12u %-22.4f %-22.9f %s\n", n, t1 - t0, (t2 - t1) / reps, b == perm[n - 1] ? "" : "ERR");
        free(fat); free(perm); free(idx);
    }
    return 0;
}
