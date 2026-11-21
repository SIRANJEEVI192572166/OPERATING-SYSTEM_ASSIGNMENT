/* Alternative concept: LOOK and C-LOOK on the given queue (head 53, cylinders 0-199). */
#include <stdio.h>
#include <stdlib.h>
static int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
static void show(const char *name, const int o[], int n, int head, int extra)
{
    int pos = head, total = 0; printf("%-7s %d", name, head);
    for (int i = 0; i < n; i++) { printf(" %d", o[i]); total += abs(o[i] - pos); pos = o[i]; }
    printf("  -> %d cylinders", total + extra); if (extra) printf(" (incl. %d for the return seek)", extra); printf("\n");
}
int main(void)
{
    int s[8] = {98, 183, 37, 122, 14, 124, 65, 67}, head = 53, o[8], n = 0;
    qsort(s, 8, sizeof s[0], cmp);
    for (int i = 7; i >= 0; i--) if (s[i] < head) o[n++] = s[i];       /* LOOK: down first */
    for (int i = 0; i < 8; i++) if (s[i] > head) o[n++] = s[i];
    show("LOOK", o, n, head, 0);
    n = 0;
    for (int i = 0; i < 8; i++) if (s[i] > head) o[n++] = s[i];        /* C-LOOK: up, then jump */
    int last = o[n - 1];
    for (int i = 0; i < 8; i++) if (s[i] < head) o[n++] = s[i];
    show("C-LOOK", o, n, head, 0);
    printf("C-LOOK return seek %d -> %d = %d cylinders; total with return seek = %d\n", last, s[0], last - s[0], 130 + (last - s[0]) + 23);
    return 0;
}
