#include <stdio.h>
#include <stdlib.h>
#define MEM 2097152
#define N 20
int main(void) {
    srand(7); int size[N], base[N]; int used = 0;
    static char map[MEM]; /* 1 = allocated */
    for (int i = 0; i < N; i++) { size[i] = 10000 + rand() % 90000; base[i] = used; used += size[i];
        for (int b = base[i]; b < base[i] + size[i]; b++) map[b] = 1; }
    printf("Loaded 20 segments (10 KB - 100 KB), first-fit, total used = %d B of %d B\n", used, MEM);
    int removed = 0, freed = 0;
    for (int i = 1; i < N; i += 2) { for (int b = base[i]; b < base[i] + size[i]; b++) map[b] = 0; removed++; freed += size[i]; }
    printf("Removed %d segments (the odd-numbered ones), freed = %d B\n", removed, freed);
    int holes = 0, largest = 0, cur = 0, total_free = 0;
    for (int b = 0; b <= MEM; b++) {
        if (b < MEM && !map[b]) { cur++; }
        else if (cur) { holes++; total_free += cur; if (cur > largest) largest = cur; cur = 0; }
    }
    printf("Free holes = %d, total free = %d B, largest hole = %d B\n", holes, total_free, largest);
    printf("External fragmentation = 1 - largest/total_free = %.1f %%\n", 100.0 * (1.0 - (double)largest / total_free));
    int need = largest + 1;
    printf("A new %d B segment cannot be placed contiguously inside holes, although %d B are free.\n", need, total_free);
    return 0;
}
