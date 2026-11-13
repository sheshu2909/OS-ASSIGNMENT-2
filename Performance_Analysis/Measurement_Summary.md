# Summary of measurements

Every value below was produced by the program named beside it and is reproduced verbatim in the matching log file. Nothing here is estimated.

## Q1 - creation and communication cost (q1_overhead)

| Quantity | Value |
|---|---|
| fork() + waitpid(), per child | 39890.1 us |
| pthread_create() + join(), per thread | 660.1 us |
| pipe round trip | 39.5 us |
| resident memory added by a process | 7040 kB |
| resident memory per thread | 27.5 kB |
| process / thread creation ratio | 60.4x |

Source: `Raw_Logs/q1_overhead.log`. Platform: MSYS2 POSIX layer on Windows 11.

## Q2 - time quantum (q2_quantum)

| Quantum | Avg turnaround | Avg waiting | Avg response | Switches |
|---|---|---|---|---|
| 1 | 12.50 | 19.00 | 0.75 | 26 |
| 2 | 12.75 | 19.25 | 2.00 | 14 |
| 3 | 13.50 | 20.00 | 3.00 | 10 |
| 5 | 11.00 | 17.50 | 5.50 | 6 |
| 10 | 8.75 | 15.25 | 8.75 | 4 |
| 20 | 8.75 | 15.25 | 8.75 | 4 |

Source: `Raw_Logs/q2_quantum.log`.

## Q3 - thread pool speed-up (q3_camera)

| Mode | Threads | Time (ms) | Speed-up | Efficiency |
|---|---|---|---|---|
| sequential | 1 | 1051.1 | 1.00 | 1.00 |
| thread per camera | 64 | 378.9 | 2.77 | 0.04 |
| thread pool | 4 | 287.5 | 3.66 | 0.91 |
| thread pool | 8 | 217.1 | 4.84 | 0.61 |
| thread pool | 16 | 214.0 | 4.91 | 0.31 |

Source: `Raw_Logs/q3_camera.log`.

## Q4 - pipe versus shared memory (q4_bench)

| Payload | Pipe (ms) | Pipe MB/s | Shared (ms) | Shared MB/s |
|---|---|---|---|---|
| 1 MB | 1.50 | 668.4 | 1.07 | 931.1 |
| 10 MB | 6.66 | 1502.6 | 17.21 | 581.0 |
| 100 MB | 108.45 | 922.1 | 159.66 | 626.3 |

Source: `Raw_Logs/q4_bench.log`.

## Q5 - lock granularity (q5_lock_granularity)

| Lock scope | Time (ms) | Updates/s | Speed-up |
|---|---|---|---|
| one global lock | 24801.1 | 64513 | 7.51 |
| one lock per train | 3303.4 | 484352 | 7.51 |
| one lock per coach | 393.1 | 4070156 | 63.09 |

Source: `Raw_Logs/q5_lock_granularity.log`.

## Q5 - atomic, spin lock and mutex (q5_atomic_vs_mutex)

| Method | Threads | Time (ms) | ns/update | CPU time (s) |
|---|---|---|---|---|
| mutex | 1 | 15.5 | 154.9 | 0.00 |
| atomic | 1 | 1.4 | 14.2 | 0.00 |
| spinlock | 1 | 2.3 | 23.0 | 0.00 |
| mutex | 4 | 6472.0 | 16180.0 | 4.22 |
| atomic | 4 | 9.5 | 23.8 | 0.05 |
| spinlock | 4 | 41.7 | 104.2 | 0.12 |
| mutex | 16 | 25951.1 | 16219.4 | 16.73 |
| atomic | 16 | 43.3 | 27.1 | 0.22 |
| spinlock | 16 | 336.0 | 210.0 | 1.45 |

Source: `Raw_Logs/q5_atomic_vs_mutex.log`.

## Q6 - safety-check cost (q6_bankers_scale)

| n | m | Repetitions | Per check (us) | ns per need check |
|---|---|---|---|---|
| 10 | 3 | 20000 | 0.26 | 1.55 |
| 100 | 3 | 2000 | 30.54 | 2.02 |
| 1000 | 3 | 200 | 2773.19 | 1.85 |
| 5000 | 3 | 20 | 74423.41 | 1.98 |

| n | m | Per check (us) | ns per need check |
|---|---|---|---|
| 1000 | 3 | 2979.95 | 1.98 |
| 1000 | 6 | 5280.17 | 1.76 |
| 1000 | 12 | 8288.73 | 1.38 |

Source: `Raw_Logs/q6_bankers_scale.log`.

## What is still to be measured

- Native Linux timings for every benchmark (same commands, report Appendix B).
- Terminal screenshots for each task (instructions in each `Screenshots/` folder).
