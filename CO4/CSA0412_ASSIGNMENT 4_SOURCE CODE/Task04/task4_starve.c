/* Task 4.3 - a request at cylinder 183 versus a stream of requests near cylinder 60
   arriving every 1 ms. Seek 0.1 ms/cylinder + 1 ms per request. */
#include <stdio.h>
#include <stdlib.h>
#define MAXP 20000
typedef struct { int cyl; double arr; } R;
static void run(const char *name, int use_scan)
{
    static R q[MAXP]; int n = 0;
    int init[8] = {98, 183, 37, 122, 14, 124, 65, 67};
    for (int i = 0; i < 8; i++) { q[n].cyl = init[i]; q[n].arr = 0; n++; }
    int pos = 53, dir = 1; double t = 0, nextarr = 1.0; srand(7);
    while (t < 10000.0) {
        while (nextarr <= t) { q[n].cyl = 55 + rand() % 11; q[n].arr = nextarr; n++; nextarr += 1.0; }
        int pick = -1;
        if (!use_scan) {
            for (int i = 0; i < n; i++) if (pick < 0 || abs(q[i].cyl - pos) < abs(q[pick].cyl - pos)) pick = i;
        } else {
            for (int pass = 0; pass < 2 && pick < 0; pass++) {
                for (int i = 0; i < n; i++) {
                    int d = (q[i].cyl - pos) * dir;
                    if (d >= 0 && (pick < 0 || d < (q[pick].cyl - pos) * dir)) pick = i;
                }
                if (pick < 0) dir = -dir;
            }
        }
        if (pick < 0) { t = nextarr; continue; }
        t += abs(q[pick].cyl - pos) * 0.1 + 1.0; pos = q[pick].cyl;
        if (q[pick].cyl == 183 && q[pick].arr == 0) {
            printf("%-5s: request at cylinder 183 served at t = %.1f ms (waited %.1f ms)\n", name, t, t);
            return;
        }
        q[pick] = q[--n];
    }
    printf("%-5s: request at cylinder 183 NOT served after 10000 ms; %d requests pending\n", name, n);
}
int main(void) { run("SSTF", 0); run("SCAN", 1); return 0; }
