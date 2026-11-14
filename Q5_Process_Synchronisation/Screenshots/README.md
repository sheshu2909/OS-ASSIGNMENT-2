# Screenshots

These are real screen captures of the programs in this folder running in a Windows console.
Each image was taken with `PrintWindow` from the running console window, so the compile line,
the command line and the program output are exactly what the machine printed.  Nothing in these
images was edited.

Machine: Windows 11, Intel Core i7-10610U (8 logical CPUs), programs built with the MSYS2 POSIX
layer using `gcc -std=c11 -Wall -Wextra -O2 -pthread`.

| Image | What it shows |
|---|---|
| `q5_bounded_buffer.png` | `./q5_bounded_buffer.exe` (the required program): 20 items consumed, sum 210, which is the expected 1+2+...+20.  The lower half repeats the whole program 100 times and tallies the printed line: all 100 runs report `sum = 210`, so no producer or consumer is starved and nothing is duplicated or lost. |
| `q5_locks.png` | `./q5_lock_granularity.exe` (8 trains x 8 coaches, 16 agent threads, 100,000 updates each): one global lock 21,774.7 ms, one lock per train 2,316.7 ms (9.40x) and one lock per coach 409.7 ms (53.14x), all with the sold counter checked at 1,600,000.  Below it, `./q5_atomic_vs_mutex.exe | tail -14` compares mutex, C11 atomic and spinlock at 1, 4 and 16 threads: the mutex degrades sharply with contention while the atomic update stays in the nanosecond range. |

Microsecond and millisecond figures are wall-clock measurements, so they move a little from run
to run with machine load.  The numbers printed on screen are the numbers from the run that was
photographed; the tables in the report quote the benchmark run whose transcript is in `Output/`.
The ratios and the correctness checks (checksums, totals, exit statuses) are identical either way.
