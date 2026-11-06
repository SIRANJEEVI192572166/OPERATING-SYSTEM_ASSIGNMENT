# CSA04 – Operating Systems – Assignment 2 (CO2)

- Name: ______________________    Register number: ______________________
- Compiler: gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1)
- Linux: Ubuntu 24.04.5 LTS, kernel 6.18.44 (1 vCPU, about 4 GB RAM)
- Compile (general): `gcc -std=c11 -Wall -Wextra -pthread -o NAME NAME.c`
- Compile (benchmarks, Q1/Q3/Q4/Q5/Q6 bench files): `gcc -std=gnu11 -O2 -Wall -Wextra -pthread -o NAME NAME.c`
- Every program prints to the terminal; the saved output is in `out_*.txt` in each folder.

| Folder | Programs | Result |
|---|---|---|
| Q1 | q1_fork.c, q1_bench.c | fork()+ps+/proc evidence; 2,000 processes vs threads (creation time, memory, context switch) |
| Q2 | q2_rr.c (Complete the Code), q2_sched.c | RR output equals expected (avg TAT 20.00, avg WT 13.50); all five algorithms, quantum study, MLFQ, ageing |
| Q3 | q3_cameras.c, q3_race.c, q3_procs.c, work.h, frame_cost.c | speed-up 2.48-2.68 on 1 CPU (limit 2.65); lost updates without protection; multi-process version |
| Q4 | q4_pipe.c (Complete the Code), q4_pipe_noclose.c, q4_shm.c, q4_bench.c | 35.5 average, max 41; hang without close(); shared memory 3.3x faster at 100 MB |
| Q5 | q5_bb.c (Complete the Code), q5_bb_multi.c, q5_seats.c, q5_monitor.c, q5_rw.c, q5_peterson.c, q5_locks.c, q5_atomic.c | sum = 210 in 100 runs; per-coach lock 7.5x global lock; atomics vs spin lock vs mutex |
| Q6 | q6_banker.c (Complete the Code), q6_bench.c, q6_detect.c | safe sequence P1 P3 P4 P0 P2; P4 waits; P0 denied; O(m n^2) confirmed; detection and recovery |

Run example: `cd Q2 && gcc -std=c11 -Wall -Wextra -o q2_rr q2_rr.c && ./q2_rr`
Q4 hang demo: `timeout 3 ./q4_pipe_noclose; echo $?` prints 124.
Q5 race demos: `gcc -std=gnu11 -O2 -pthread -DNO_MUTEX [-DINJECT] -o q5_bb_nomutex q5_bb_multi.c`.
Q3 forced race: `./q3_race yield`.
