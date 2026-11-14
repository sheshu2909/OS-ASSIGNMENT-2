# Screenshots

These are real screen captures of the programs in this folder running in a Windows console.
Each image was taken with `PrintWindow` from the running console window, so the compile line,
the command line and the program output are exactly what the machine printed.  Nothing in these
images was edited.

Machine: Windows 11, Intel Core i7-10610U (8 logical CPUs), programs built with the MSYS2 POSIX
layer using `gcc -std=c11 -Wall -Wextra -O2 -pthread`.

| Image | What it shows |
|---|---|
| `q1_fork_and_ps.png` | Compile `q1_processes.c`, then `./q1_processes.exe 6 q1_pids.txt`.  The program forks three children and prints the /proc state of each one as it runs, waits and is reaped.  While the children sleep the runner also calls `ps -fp $(paste -sd, q1_pids.txt)` and the result shows the first child, whose parent (3083) is the program and whose state is S because it is in `sleep 6`.  The last two lines show the parent reaping the children with their real exit statuses, so no zombie is left behind. |
| `q1_overhead.png` | `./q1_overhead.exe 200`: the cost of `fork()` + `_exit()` + `waitpid()` against `pthread_create()` + `pthread_join()`, a pipe round trip, and the resident memory added by 256 live thread stacks.  Measured values in this run: 41,694.5 us, 463.9 us, 40.6 us and 25.2 kB per stack, a process/thread ratio of 89.9x. |

Microsecond and millisecond figures are wall-clock measurements, so they move a little from run
to run with machine load.  The numbers printed on screen are the numbers from the run that was
photographed; the tables in the report quote the benchmark run whose transcript is in `Output/`.
The ratios and the correctness checks (checksums, totals, exit statuses) are identical either way.
