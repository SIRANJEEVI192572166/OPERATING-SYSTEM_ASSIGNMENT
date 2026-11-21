#define _POSIX_C_SOURCE 199309L
/* Task 1 - search cost: one flat directory versus a two-level tree.
   12,000 students x 20 files = 240,000 names. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define S 12000
#define F 20
static char flat[S * F][24];
static char dirs[S][12];
static char files[S][F][12];
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec*1e3+t.tv_nsec/1e6; }
static int cmpd(const void *a,const void *b){ return strcmp((const char*)a,(const char*)b); }
int main(void)
{
    for (int s = 0; s < S; s++) {
        sprintf(dirs[s], "R%05d", s);
        for (int f = 0; f < F; f++) { sprintf(files[s][f], "file%02d.doc", f); sprintf(flat[s*F+f], "R%05d_file%02d.doc", s, f); }
    }
    srand(1); int trials = 1000; long cflat = 0, ctree = 0; volatile int sink = 0;
    int ts[1000], tf[1000];
    for (int i = 0; i < trials; i++) { ts[i] = rand() % S; tf[i] = rand() % F; }
    double t0 = now();
    for (int i = 0; i < trials; i++) {
        char key[24]; sprintf(key, "R%05d_file%02d.doc", ts[i], tf[i]);
        for (int k = 0; k < S*F; k++) { cflat++; if (strcmp(flat[k], key) == 0) { sink = k; break; } }
    }
    double t1 = now();
    for (int i = 0; i < trials; i++) {
        char key[12]; sprintf(key, "R%05d", ts[i]); char fk[12]; sprintf(fk, "file%02d.doc", tf[i]);
        int lo = 0, hi = S - 1, mid = 0;
        while (lo <= hi) { mid = (lo + hi) / 2; ctree++; int c = strcmp(dirs[mid], key); if (c == 0) break; if (c < 0) lo = mid + 1; else hi = mid - 1; }
        for (int f = 0; f < F; f++) { ctree++; if (strcmp(files[mid][f], fk) == 0) { sink = f; break; } }
    }
    double t2 = now(); (void)cmpd; (void)sink;
    printf("Names stored: %d (12,000 students x %d files)\n", S*F, F);
    printf("Flat directory (linear search): avg %.0f comparisons, avg %.3f ms per lookup\n", (double)cflat/trials, (t1-t0)/trials);
    printf("Two-level tree (binary + linear): avg %.1f comparisons, avg %.5f ms per lookup\n", (double)ctree/trials, (t2-t1)/trials);
    return 0;
}
