/* Supplementary demo: a file that really cannot be stored contiguously. */
#include <stdio.h>
#include <string.h>
#define BLOCKS 32
char disk[BLOCKS];
int alloc_contiguous(char file, int size)
{
    int run = 0;
    for (int b = 0; b < BLOCKS; b++) {
        if (disk[b] == '.') {
            if (++run == size) {
                for (int i = b - size + 1; i <= b; i++) disk[i] = file;
                return b - size + 1;
            }
        } else run = 0;
    }
    return -1;
}
int alloc_indexed(char file, int size, int index[])
{
    int got = 0, idx = -1;
    for (int b = 0; b < BLOCKS && got <= size; b++) {
        if (disk[b] != '.') continue;
        if (idx < 0) idx = b; else index[got++] = b;
        if (got == size) break;
    }
    if (idx < 0 || got < size) return -1;
    disk[idx] = (char)(file + 32);
    for (int i = 0; i < size; i++) disk[index[i]] = file;
    return idx;
}
void release(char file) { for (int b = 0; b < BLOCKS; b++) if (disk[b] == file) disk[b] = '.'; }
int main(void)
{
    memset(disk, '.', BLOCKS);
    alloc_contiguous('A', 6); alloc_contiguous('B', 8);
    alloc_contiguous('C', 5); alloc_contiguous('D', 7);
    release('B'); release('D');
    int free_blocks = 0;
    for (int b = 0; b < BLOCKS; b++) if (disk[b] == '.') free_blocks++;
    printf("%.32s  (%d blocks free; holes of 8 and 13)\n", disk, free_blocks);
    printf("F (14 blocks) contiguous: start %d\n", alloc_contiguous('F', 14));
    int index[16];
    int ib = alloc_indexed('F', 14, index);
    printf("F (14 blocks) indexed: index block %d, data blocks", ib);
    for (int i = 0; i < 14; i++) printf(" %d", index[i]);
    printf("\n%.32s\n", disk);
    return 0;
}
