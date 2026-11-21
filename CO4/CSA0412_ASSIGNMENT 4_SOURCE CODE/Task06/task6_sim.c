/* Task 6 Stage 5 - one streaming node, 2 MiB chunk requests, Poisson arrivals.
   Design A: 6 HDD only.   Design B: 2 NVMe SSD (hot 20% of videos) + 4 HDD (rest).
   Video popularity: Zipf(1.0) over 10,000 videos. HDD chunk = 8 ms seek + 4.17 ms rotation
   + 2 MiB / 180 MB/s; SSD chunk = 0.1 ms + 2 MiB / 3000 MB/s. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define NV 10000
#define HOT 2000
#define REQS 10000
static double hdd_ms(void) { return 8.0 + 4.17 + 2.097152 / 180.0 * 1000.0; }
static double ssd_ms(void) { return 0.1 + 2.097152 / 3000.0 * 1000.0; }
static double urand(void) { return (rand() + 1.0) / (RAND_MAX + 2.0); }
static int cmpd(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }
static double cdf[NV];
static int pick_video(void) { double u = urand() * cdf[NV - 1]; int lo = 0, hi = NV - 1;
    while (lo < hi) { int m = (lo + hi) / 2; if (cdf[m] < u) lo = m + 1; else hi = m; } return lo; }
static void sim(const char *name, double rate, int nh, int ns)
{
    double hf[16] = {0}, sf[16] = {0}, lat[REQS], t = 0, hbusy = 0, sbusy = 0; int hits = 0;
    srand(2024);
    for (int i = 0; i < REQS; i++) {
        t += -log(urand()) / rate * 1000.0;                  /* ms */
        int v = pick_video(); double fin, svc;
        if (ns > 0 && v < HOT) {
            int b = 0; for (int k = 1; k < ns; k++) if (sf[k] < sf[b]) b = k;
            double st = sf[b] > t ? sf[b] : t; svc = ssd_ms(); fin = st + svc; sf[b] = fin; sbusy += svc; hits++;
        } else {
            int b = 0; for (int k = 1; k < nh; k++) if (hf[k] < hf[b]) b = k;
            double st = hf[b] > t ? hf[b] : t; svc = hdd_ms(); fin = st + svc; hf[b] = fin; hbusy += svc;
        }
        lat[i] = fin - t;
    }
    double sum = 0, mx = 0; for (int i = 0; i < REQS; i++) { sum += lat[i]; if (lat[i] > mx) mx = lat[i]; }
    qsort(lat, REQS, sizeof lat[0], cmpd);
    printf("%-9s %5.0f/s  mean %8.1f  p95 %8.1f  max %8.1f  SSD hits %4.1f%%  HDD util %5.1f%%  SSD util %4.1f%%\n",
           name, rate, sum / REQS, lat[(int)(0.95 * REQS)], mx, 100.0 * hits / REQS,
           100.0 * hbusy / (t * nh), ns ? 100.0 * sbusy / (t * ns) : 0.0);
}
int main(void)
{
    double s = 0; for (int i = 0; i < NV; i++) { s += 1.0 / (i + 1); cdf[i] = s; }
    printf("Per-node chunk requests: 10,000 | latency = queue wait + service (ms) to deliver the first 2 MiB chunk\n");
    printf("HDD service %.2f ms, SSD service %.2f ms\n\n", hdd_ms(), ssd_ms());
    double rates[3] = {150, 200, 372};
    for (int r = 0; r < 3; r++) {
        sim("Design A", rates[r], 6, 0);
        sim("Design B", rates[r], 4, 2);
        printf("\n");
    }
    return 0;
}
