/* Task 4.2 - disk scheduling simulation, closed queue of depth Q.
   Seek = 0.1 ms per cylinder, fixed 1 ms rotation+transfer per request. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CYL 200
#define TOTAL 10000
#define MAXQ 256
enum { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, NALG };
static const char *names[NALG] = {"FCFS","SSTF","SCAN","C-SCAN","LOOK","C-LOOK"};
static int reqs[TOTAL + MAXQ];

typedef struct { int cyl; double arr; int seq; } Req;

static void run(int alg, int Q, double *avg_move, double *max_wait, double *avg_wait)
{
    Req q[MAXQ]; int n = 0, next = 0, served = 0;
    int pos = 100, dir = 1; double t = 0, moved = 0, wsum = 0, wmax = 0;
    for (; next < Q; next++) { q[n].cyl = reqs[next]; q[n].arr = 0; q[n].seq = next; n++; }
    while (served < TOTAL) {
        int pick = -1, extra = 0;
        if (alg == FCFS) {
            for (int i = 0; i < n; i++) if (pick < 0 || q[i].seq < q[pick].seq) pick = i;
        } else if (alg == SSTF) {
            for (int i = 0; i < n; i++)
                if (pick < 0 || abs(q[i].cyl - pos) < abs(q[pick].cyl - pos)) pick = i;
        } else if (alg == SCAN || alg == LOOK) {
            for (int pass = 0; pass < 2 && pick < 0; pass++) {
                for (int i = 0; i < n; i++) {
                    int d = (q[i].cyl - pos) * dir;
                    if (d >= 0 && (pick < 0 || d < (q[pick].cyl - pos) * dir)) pick = i;
                }
                if (pick < 0) {                       /* nothing ahead: turn round */
                    if (alg == SCAN) { int end = dir > 0 ? CYL - 1 : 0; extra += abs(end - pos); pos = end; }
                    dir = -dir;
                }
            }
        } else if (alg == CSCAN) {
            for (int i = 0; i < n; i++)
                if (q[i].cyl >= pos && (pick < 0 || q[i].cyl < q[pick].cyl)) pick = i;
            if (pick < 0) {                           /* go to the end, jump to 0 */
                extra += (CYL - 1 - pos) + (CYL - 1); pos = 0;
                for (int i = 0; i < n; i++) if (pick < 0 || q[i].cyl < q[pick].cyl) pick = i;
            }
        } else {                                      /* C-LOOK */
            for (int i = 0; i < n; i++)
                if (q[i].cyl >= pos && (pick < 0 || q[i].cyl < q[pick].cyl)) pick = i;
            if (pick < 0) {
                for (int i = 0; i < n; i++) if (pick < 0 || q[i].cyl < q[pick].cyl) pick = i;
                extra += abs(pos - q[pick].cyl);      /* return seek to lowest request */
                pos = q[pick].cyl;
            }
        }
        int d = abs(q[pick].cyl - pos) + extra;
        double start = t;
        t += d * 0.1 + 1.0;
        double w = start - q[pick].arr;
        wsum += w; if (w > wmax) wmax = w;
        moved += d; pos = q[pick].cyl; served++;
        if (next < TOTAL + Q) { q[pick].cyl = reqs[next]; q[pick].arr = t; q[pick].seq = next; next++; }
        else q[pick] = q[--n];
    }
    *avg_move = moved / TOTAL; *max_wait = wmax; *avg_wait = wsum / TOTAL;
}

int main(void)
{
    srand(42);
    for (int i = 0; i < TOTAL + MAXQ; i++) reqs[i] = rand() % CYL;
    int loads[3] = {5, 20, 100};
    const char *lname[3] = {"Light (queue depth 5)", "Medium (queue depth 20)", "Heavy (queue depth 100)"};
    for (int l = 0; l < 3; l++) {
        printf("\n%s - 10,000 random requests, cylinders 0-199\n", lname[l]);
        printf("%-8s %12s %14s %14s\n", "Algo", "Avg move/req", "Avg wait (ms)", "Max wait (ms)");
        for (int a = 0; a < NALG; a++) {
            double am, mw, aw; run(a, loads[l], &am, &mw, &aw);
            printf("%-8s %12.2f %14.1f %14.1f\n", names[a], am, aw, mw);
        }
    }
    return 0;
}
