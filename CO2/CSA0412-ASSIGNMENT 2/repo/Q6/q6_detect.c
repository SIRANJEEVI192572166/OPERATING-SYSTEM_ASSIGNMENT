/* Deadlock detection (and recovery by aborting the cheapest deadlocked job) for the research cluster */
#include <stdio.h>
#include <string.h>
#define N 4          /* jobs      J1..J4            */
#define M 3          /* resources GPU, LIC, STO     */

static const char *rname[M] = {"GPU", "LIC", "STO"};
static int avail[M] = {0, 0, 0};
static int alloc[N][M] = {{1,0,0}, {0,1,0}, {0,0,1}, {0,0,0}};      /* J1 holds GPU, J2 holds LIC, J3 holds STO */
static int request[N][M] = {{0,1,0}, {0,0,1}, {1,0,0}, {0,0,0}};    /* J1 wants LIC, J2 wants STO, J3 wants GPU */
static int lost_minutes[N] = {40, 5, 90, 1};                         /* work lost if the job is aborted           */

/* returns the number of deadlocked jobs; marks them in dead[] */
static int detect(int dead[N])
{
    int work[M], finish[N], changed = 1, k = 0;
    memcpy(work, avail, sizeof work);
    for (int i = 0; i < N; i++) { int z = 1; for (int r = 0; r < M; r++) if (alloc[i][r]) z = 0; finish[i] = z; }
    while (changed) {                                   /* O(m * n^2) */
        changed = 0;
        for (int i = 0; i < N; i++) {
            if (finish[i]) continue;
            int ok = 1; for (int r = 0; r < M; r++) if (request[i][r] > work[r]) { ok = 0; break; }
            if (ok) { for (int r = 0; r < M; r++) work[r] += alloc[i][r]; finish[i] = 1; changed = 1; }
        }
    }
    for (int i = 0; i < N; i++) { dead[i] = !finish[i]; k += dead[i]; }
    return k;
}
int main(void)
{
    int dead[N], k = detect(dead);
    printf("Detection run 1: %d deadlocked job(s):", k);
    for (int i = 0; i < N; i++) if (dead[i]) printf(" J%d", i + 1);
    printf("\n");
    if (k) {
        int victim = -1;
        for (int i = 0; i < N; i++) if (dead[i] && (victim < 0 || lost_minutes[i] < lost_minutes[victim])) victim = i;
        printf("Recovery: abort J%d (cheapest victim, %d minutes of work lost) and release", victim + 1, lost_minutes[victim]);
        for (int r = 0; r < M; r++) if (alloc[victim][r]) { printf(" %s", rname[r]); avail[r] += alloc[victim][r]; alloc[victim][r] = 0; }
        printf("\n");
        for (int r = 0; r < M; r++) request[victim][r] = 0;
        k = detect(dead);
        printf("Detection run 2: %d deadlocked job(s)%s\n", k, k ? "" : " - the cluster can continue");
    }
    return 0;
}
