/* Timing of the Banker's safety algorithm for n processes and m resource types */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }

static int safe(int n, int m, const int *avail, const int *need, const int *alloc, int *seq)
{
    int *work = malloc(m * sizeof *work), *finish = calloc(n, sizeof *finish), count = 0;
    memcpy(work, avail, m * sizeof *work);
    while (count < n) {
        int found = 0;
        for (int p = 0; p < n; p++) {
            if (finish[p]) continue;
            int ok = 1; for (int r = 0; r < m; r++) if (need[p*m + r] > work[r]) { ok = 0; break; }
            if (ok) { for (int r = 0; r < m; r++) work[r] += alloc[p*m + r]; finish[p] = 1; seq[count++] = p; found = 1; }
        }
        if (!found) { free(work); free(finish); return 0; }
    }
    free(work); free(finish); return 1;
}
/* best = 1: plenty of resources, every process can finish in the first pass.
   best = 0: worst case chain - process i can only finish after process i+1, so one process finishes per pass */
static void build(int n, int m, int best, int *avail, int *need, int *alloc)
{
    for (int r = 0; r < m; r++) avail[r] = best ? 1000000 : 1;
    for (int p = 0; p < n; p++) for (int r = 0; r < m; r++) { alloc[p*m + r] = 1; need[p*m + r] = (best || r < m - 1) ? 1 : 1 + (n - 1 - p); }
}
int main(void)
{
    int ns[3] = {10, 100, 1000}, m = 3;
    printf("Safety algorithm, m = %d resource types\n", m);
    printf("%-16s %6s %14s %16s\n", "Case", "n", "us per check", "growth vs n/10");
    for (int c = 0; c < 2; c++) {
        double prev = 0;
        for (int k = 0; k < 3; k++) {
            int n = ns[k];
            int *avail = malloc(m * sizeof(int)), *need = malloc(n*m*sizeof(int)), *alloc = malloc(n*m*sizeof(int)), *seq = malloc(n*sizeof(int));
            build(n, m, c == 0, avail, need, alloc);
            long reps = (long)(2e7 / ((c == 0 ? (double)n : (double)n * n) * m)); if (reps < 3) reps = 3;
            int ok = 0; double t0 = now();
            for (long i = 0; i < reps; i++) ok += safe(n, m, avail, need, alloc, seq);
            double us = (now() - t0) * 1e6 / reps; char g[32];
            if (prev > 0) snprintf(g, sizeof g, "x%.1f", us / prev); else snprintf(g, sizeof g, "-");
            printf("%-16s %6d %14.2f %16s%s\n", c == 0 ? "Best (1 pass)" : "Worst (n passes)", n, us, g, ok == reps ? "" : "  UNSAFE?");
            prev = us; free(avail); free(need); free(alloc); free(seq);
        }
    }
    printf("\nPrediction: best case O(n*m) -> x10 when n grows x10; worst case O(n^2*m) -> x100.\n");
    printf("\nGrowth with m (n = 100, worst case):\n%-4s %14s\n", "m", "us per check");
    int n = 100; for (int mm = 3; mm <= 24; mm *= 2) {
        int *avail = malloc(mm*sizeof(int)), *need = malloc(n*mm*sizeof(int)), *alloc = malloc(n*mm*sizeof(int)), *seq = malloc(n*sizeof(int));
        build(n, mm, 0, avail, need, alloc); long reps = 2000; double t0 = now();
        for (long i = 0; i < reps; i++) safe(n, mm, avail, need, alloc, seq);
        printf("%-4d %14.2f\n", mm, (now() - t0) * 1e6 / reps); free(avail); free(need); free(alloc); free(seq);
    }
    return 0;
}
