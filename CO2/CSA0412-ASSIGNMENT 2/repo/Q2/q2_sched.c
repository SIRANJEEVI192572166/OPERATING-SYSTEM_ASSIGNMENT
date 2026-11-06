/* CPU-scheduling simulator: FCFS, SJF, SRTF, Priority, Round Robin, MLFQ (unit-time steps) */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXJ 16
#define MAXSEG 512
typedef struct { const char *name; int at, bt, pr; } Job;
typedef struct { int p, s, e; } Seg;
typedef struct { Seg seg[MAXSEG]; int ns; int ct[MAXJ], rt[MAXJ]; } Result;

static void add_seg(Result *r, int p, int s, int e)
{
    if (r->ns && r->seg[r->ns-1].p == p && r->seg[r->ns-1].e == s) r->seg[r->ns-1].e = e;
    else { r->seg[r->ns].p = p; r->seg[r->ns].s = s; r->seg[r->ns].e = e; r->ns++; }
}
static void init(Result *r, int n){ memset(r, 0, sizeof *r); for (int i = 0; i < n; i++) r->rt[i] = -1; }
static void note_start(Result *r, const Job *j, int p, int t){ if (r->rt[p] < 0) r->rt[p] = t - j[p].at; }

/* ---- non-preemptive: pick next by key (0=arrival,1=burst,2=priority) ---- */
static void nonpreemptive(const Job *j, int n, int key, Result *r)
{
    int done[MAXJ] = {0}, t = 0, cnt = 0; init(r, n);
    while (cnt < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (done[i] || j[i].at > t) continue;
            int ki = key == 0 ? j[i].at : key == 1 ? j[i].bt : j[i].pr;
            int kb = best < 0 ? 0 : key == 0 ? j[best].at : key == 1 ? j[best].bt : j[best].pr;
            if (best < 0 || ki < kb || (ki == kb && j[i].at < j[best].at)) best = i;
        }
        if (best < 0) { t++; continue; }
        note_start(r, j, best, t); add_seg(r, best, t, t + j[best].bt);
        t += j[best].bt; r->ct[best] = t; done[best] = 1; cnt++;
    }
}
/* ---- SRTF (preemptive shortest remaining time) ---- */
static void srtf(const Job *j, int n, Result *r)
{
    int left[MAXJ], t = 0, cnt = 0; init(r, n);
    for (int i = 0; i < n; i++) left[i] = j[i].bt;
    while (cnt < n) {
        int best = -1;
        for (int i = 0; i < n; i++) if (left[i] > 0 && j[i].at <= t &&
            (best < 0 || left[i] < left[best] || (left[i] == left[best] && j[i].at < j[best].at))) best = i;
        if (best < 0) { t++; continue; }
        note_start(r, j, best, t); add_seg(r, best, t, t + 1); t++; left[best]--;
        if (!left[best]) { r->ct[best] = t; cnt++; }
    }
}
/* ---- Round Robin, same queue rules as the assignment's program ---- */
static void rr(const Job *j, int n, int q, Result *r)
{
    int left[MAXJ], added[MAXJ] = {0}, queue[1024], head = 0, tail = 0, t = 0, done = 0; init(r, n);
    for (int i = 0; i < n; i++) left[i] = j[i].bt;
    queue[tail++] = 0; added[0] = 1; t = j[0].at;
    while (done < n) {
        if (head == tail) { t++; for (int i = 0; i < n; i++) if (!added[i] && j[i].at <= t) { queue[tail++] = i; added[i] = 1; } continue; }
        int p = queue[head++], run = left[p] < q ? left[p] : q;
        note_start(r, j, p, t); add_seg(r, p, t, t + run); t += run; left[p] -= run;
        for (int i = 0; i < n; i++) if (!added[i] && j[i].at <= t) { queue[tail++] = i; added[i] = 1; }
        if (left[p] > 0) queue[tail++] = p; else { r->ct[p] = t; done++; }
    }
}
/* ---- Multilevel feedback queue: 3 levels, quanta qs[0..2]; last level is FCFS-like (big quantum) ---- */
static void mlfq(const Job *j, int n, const int qs[3], Result *r)
{
    int left[MAXJ], level[MAXJ] = {0}, inq[MAXJ] = {0}, t = 0, done = 0;
    int Q[3][256], hd[3] = {0}, tl[3] = {0}; init(r, n);
    for (int i = 0; i < n; i++) left[i] = j[i].bt;
    while (done < n) {
        for (int i = 0; i < n; i++) if (!inq[i] && left[i] > 0 && j[i].at <= t) { Q[0][tl[0]++] = i; inq[i] = 1; }
        int lv = -1; for (int l = 0; l < 3; l++) if (hd[l] < tl[l]) { lv = l; break; }
        if (lv < 0) { t++; continue; }
        int p = Q[lv][hd[lv]++], used = 0, preempted = 0;
        note_start(r, j, p, t);
        while (used < qs[lv] && left[p] > 0) {
            add_seg(r, p, t, t + 1); t++; used++; left[p]--;
            for (int i = 0; i < n; i++) if (!inq[i] && left[i] > 0 && j[i].at <= t) { Q[0][tl[0]++] = i; inq[i] = 1; }
            int higher = 0; for (int l = 0; l < lv; l++) if (hd[l] < tl[l]) higher = 1;
            if (higher && left[p] > 0) { preempted = 1; break; }
        }
        if (left[p] == 0) { r->ct[p] = t; done++; }
        else if (preempted) Q[lv][tl[lv]++] = p;                       /* keeps its level */
        else { if (lv < 2) level[p] = lv + 1; Q[level[p]][tl[level[p]]++] = p; }   /* used full quantum: demote */
    }
}

static void report(const char *title, const Job *j, int n, const Result *r)
{
    printf("%s\nGantt:", title);
    for (int i = 0; i < r->ns; i++) printf(" %s(%d-%d)", j[r->seg[i].p].name, r->seg[i].s, r->seg[i].e);
    printf("\nProc  AT  BT  CT  TAT  WT  RT\n");
    double ta = 0, wa = 0, ra = 0;
    for (int i = 0; i < n; i++) {
        int tat = r->ct[i] - j[i].at, wt = tat - j[i].bt;
        printf("%-4s %3d %3d %3d %4d %3d %3d\n", j[i].name, j[i].at, j[i].bt, r->ct[i], tat, wt, r->rt[i]);
        ta += tat; wa += wt; ra += r->rt[i];
    }
    printf("Average: TAT = %.2f  WT = %.2f  RT = %.2f   Context switches = %d\n\n", ta/n, wa/n, ra/n, r->ns - 1);
}
static double avg_wt_group(const Job *j, int n, const Result *r, int lo, int hi)  /* jobs with bt in [lo,hi] */
{ double s = 0; int c = 0; for (int i = 0; i < n; i++) if (j[i].bt >= lo && j[i].bt <= hi) { s += r->ct[i] - j[i].at - j[i].bt; c++; } return c ? s / c : 0; }

/* ---- starvation / ageing demo: preemptive priority, lower number = higher priority ---- */
static void starvation(int ageing)
{
    enum { M = 13 }; int at[M], bt[M], pr[M], left[M], ct[M] = {0}, waited[M] = {0}, t = 0, done = 0;
    at[0] = 0; bt[0] = 4; pr[0] = 9;                          /* L: low priority, burst 4 */
    for (int i = 1; i < M; i++) { at[i] = (i - 1) * 2; bt[i] = 2; pr[i] = 1; }  /* H1..H12: one every 2 units */
    for (int i = 0; i < M; i++) left[i] = bt[i];
    while (done < M) {
        int best = -1, bp = 1 << 30;
        for (int i = 0; i < M; i++) if (left[i] > 0 && at[i] <= t) {
            int eff = pr[i] - (ageing ? waited[i] / 2 : 0);   /* ageing: priority improves by 1 per 2 units waited */
            if (eff < bp) { bp = eff; best = i; }
        }
        if (best < 0) { t++; continue; }
        for (int i = 0; i < M; i++) if (i != best && left[i] > 0 && at[i] <= t) waited[i]++;
        t++; left[best]--;
        if (!left[best]) { ct[best] = t; done++; }
    }
    printf("%s: L (priority 9, burst 4, arrives 0) completes at t = %d, waiting time = %d\n",
           ageing ? "with ageing   " : "without ageing", ct[0], ct[0] - at[0] - bt[0]);
}

int main(void)
{
    Job A[4] = {{"P1",0,8,2},{"P2",1,4,4},{"P3",2,9,1},{"P4",3,5,3}};
    Result r;
    printf("=========== SET A (lower priority number = higher priority) ===========\n");
    nonpreemptive(A, 4, 0, &r); report("--- FCFS ---", A, 4, &r);
    nonpreemptive(A, 4, 1, &r); report("--- SJF (non-preemptive) ---", A, 4, &r);
    srtf(A, 4, &r);             report("--- SRTF (preemptive SJF) ---", A, 4, &r);
    nonpreemptive(A, 4, 2, &r); report("--- Priority (non-preemptive) ---", A, 4, &r);
    rr(A, 4, 3, &r);            report("--- Round Robin, q = 3 ---", A, 4, &r);

    printf("=========== QUANTUM STUDY (Set A, Round Robin) ===========\n");
    printf("q    avgTAT  avgWT  avgRT  context-switches\n");
    int qs[4] = {1, 3, 5, 20};
    for (int k = 0; k < 4; k++) {
        rr(A, 4, qs[k], &r); double ta = 0, wa = 0, ra = 0;
        for (int i = 0; i < 4; i++) { int tat = r.ct[i] - A[i].at; ta += tat; wa += tat - A[i].bt; ra += r.rt[i]; }
        printf("%-4d %6.2f %6.2f %6.2f  %d\n", qs[k], ta/4, wa/4, ra/4, r.ns - 1);
    }

    printf("\n=========== SET B: VIDEO SERVICE (2 films + 4 thumbnails) ===========\n");
    Job B[6] = {{"Film1",0,40,2},{"Film2",1,30,2},{"Th1",5,2,1},{"Th2",6,2,1},{"Th3",7,2,1},{"Th4",8,2,1}};
    int mq[3] = {2, 4, 8};
    printf("Policy            avgWT(thumbnails)  avgWT(films)  avgWT(all)  avgRT(thumbnails)  switches\n");
    const char *nm[6] = {"FCFS", "SJF", "SRTF", "RR q=3", "RR q=20", "MLFQ 2/4/8"};
    for (int k = 0; k < 6; k++) {
        if (k == 0) nonpreemptive(B, 6, 0, &r); else if (k == 1) nonpreemptive(B, 6, 1, &r);
        else if (k == 2) srtf(B, 6, &r); else if (k == 3) rr(B, 6, 3, &r);
        else if (k == 4) rr(B, 6, 20, &r); else mlfq(B, 6, mq, &r);
        double rts = 0; for (int i = 2; i < 6; i++) rts += r.rt[i];
        printf("%-16s %12.2f %14.2f %12.2f %14.2f %12d\n", nm[k], avg_wt_group(B,6,&r,1,5), avg_wt_group(B,6,&r,10,100),
               avg_wt_group(B,6,&r,1,100), rts/4, r.ns - 1);
    }
    mlfq(B, 6, mq, &r); printf("\n"); report("--- MLFQ (quanta 2/4/8) on Set B ---", B, 6, &r);

    printf("=========== STARVATION AND AGEING (preemptive priority) ===========\n");
    starvation(0); starvation(1);
    return 0;
}
