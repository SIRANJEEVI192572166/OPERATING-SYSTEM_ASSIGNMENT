# CSA04 – Operating Systems – Assignment 4 (CO4)
**Optimising File Storage and Disk Access**

| Field | Value |
|---|---|
| Name | ______________________ |
| Register Number | ______________________ |
| Repository | CSA04-<RegisterNumber>-Assignment4 |
| Operating system | Ubuntu 24.04.5 LTS (Linux kernel 6.18.44) |
| Compiler and version | gcc 13.3.0 |
| Compile command | `gcc -std=c11 -Wall -Wextra -o <name> <name>.c` |

Use `-O1` for the timing programs (`task1_search`, `task2_fat`, `task4_sim`, `task4_starve`, `task6_sim`) and add `-lm` for `task6_sim`. All programs compile with no warnings.

## Repository structure
One folder per task, each containing the C source files, command outputs and screenshots.

```
Task01/  task1_search.c
Task02/  task2_alloc.c  task2_extra.c  task2_fat.c
Task03/  cmds1.txt  cmds2.txt            (command outputs)
Task04/  task4_sched.c  task4_sim.c  task4_starve.c  task4_look.c  sim_out.txt  sched_cmd.txt
Task06/  task6_sim.c  sim_out.txt
output_images/                           (screenshots of every execution)
CSA04_Assignment4_Answers.docx           (full report)
```

## How to run

### Task 1 – File-system design review
```
cd Task01
gcc -std=c11 -Wall -Wextra -O1 -o task1_search task1_search.c
./task1_search
```
**Result:** a flat directory of 240,000 names needs about 123,107 comparisons per look-up; the two-level tree needs about 23.

### Task 2 – File-allocation method
```
cd Task02
gcc -std=c11 -Wall -Wextra -o task2_alloc task2_alloc.c && ./task2_alloc
gcc -std=c11 -Wall -Wextra -o task2_extra task2_extra.c && ./task2_extra
gcc -std=c11 -Wall -Wextra -O1 -o task2_fat task2_fat.c && ./task2_fat
```
**Result:**
- `task2_alloc` (TODO 1, first-fit contiguous) matches the expected output: E contiguous at 19; E indexed with index block 6 and data blocks 7–13, 29–31.
- `task2_extra`: a 14-block file fails contiguous allocation (−1) but succeeds with indexed allocation.
- `task2_fat`: walking an in-memory FAT is O(k) (43.4 ms for 2^20 blocks); an index look-up is O(1).

### Task 3 – Modern file systems
```
cd Task03
truncate -s 64M ext4test.img && mkfs.ext4 -q -b 4096 -F ext4test.img
echo "hello operating systems" > sample.txt
ls -i sample.txt; stat sample.txt; df -i .; df -h .
tune2fs -l ext4test.img
dumpe2fs -h ext4test.img
```
**Result:** block size 4096, 16,384 inodes of 256 bytes, journal enabled (inode 8); `sample.txt` uses one 4 KB block.

### Task 4 – Disk scheduling
```
cd Task04
gcc -std=c11 -Wall -Wextra -o task4_sched task4_sched.c && ./task4_sched
gcc -std=c11 -Wall -Wextra -O1 -o task4_sim task4_sim.c && ./task4_sim
gcc -std=c11 -Wall -Wextra -O1 -o task4_starve task4_starve.c && ./task4_starve
gcc -std=c11 -Wall -Wextra -o task4_look task4_look.c && ./task4_look
cat /sys/block/<disk>/queue/scheduler
```
**Result:**
- `task4_sched` (TODO 1, SSTF) matches the expected output: FCFS 640, SSTF 236, SCAN 236, C-SCAN 183 (+199 for the jump).
- `task4_sim`: average head movement and maximum waiting time for 10,000 random requests at light, medium and heavy load.
- `task4_starve`: SSTF leaves the request at cylinder 183 unserved after 10,000 ms; SCAN serves it after 19 ms.
- `task4_look`: LOOK 208, C-LOOK 322 (153 without the return seek).

### Task 5 – Distributed and cloud storage
Written answers only (comparison tables, replication and consistency, device management, NFS versus replicated file system); see the report.

### Task 6 – Design challenge
```
cd Task06
gcc -std=c11 -Wall -Wextra -O1 -o task6_sim task6_sim.c -lm
./task6_sim
```
**Result:** at the calculated peak of 372 requests/s per node, Design A (6 HDDs) overloads (mean latency about 6.6 s), while Design B (2 SSDs for the hottest 20 % of videos + 4 HDDs) keeps the mean at about 4.6 ms and the 95th percentile at 23.8 ms.

## Commit plan (at least one commit per task)
```
git add Task01 && git commit -m "Task 1: file-system design review and search measurement"
git add Task02 && git commit -m "Task 2: allocation methods, TODO 1, FAT measurement"
git add Task03 && git commit -m "Task 3: modern file systems and command evidence"
git add Task04 && git commit -m "Task 4: disk scheduling, SSTF TODO, simulations"
git add Task06 && git commit -m "Task 6: video-streaming storage design and simulation"
git add README.md CSA04_Assignment4_Answers.docx output_images && git commit -m "Report, README and screenshots"
```
