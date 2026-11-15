# CSA04 Operating Systems - Assignment 3 (CO3)

Name: <your name>
Register number: <your register number>
Compiler: gcc 13.3.0 (Ubuntu 24.04, x86-64, Linux)
Compile command: gcc -std=c11 -Wall -Wextra <file>.c -o <name>
(extra flags: -fPIC -shared for libenc.so, -static for bank_static, -ldl for bank_dl, -O2 for Q5/exp.c)

Folders Task01 ... Task07 hold sources, command outputs (out_*.txt) and screenshots.
Run each program yourself on your own machine, replace the outputs/screenshots with your own,
and commit task by task (at least one commit per task) before submitting the link.

Q1: Task01/enc.c bank.c bank_dl.c (build commands in out_build.txt)
Q2: paging.c (expected output matches the assignment), frag.c, edge.c, pfcount.c
Q3: seg.c, segv.c, segfrag.c, layout.c
Q4: touch.c, cow.c
Q5: replace.c (completed LRU, prints 15/12/9, 10/8/8, 9/10), exp.c
Q6: command outputs only (lsblk, RAID calculations)
Q7: rt.c (needs permission for mlockall), sim.c
