# Screenshots

These are real screen captures of the programs in this folder running in a Windows console.
Each image was taken with `PrintWindow` from the running console window, so the compile line,
the command line and the program output are exactly what the machine printed.  Nothing in these
images was edited.

Machine: Windows 11, Intel Core i7-10610U (8 logical CPUs), programs built with the MSYS2 POSIX
layer using `gcc -std=c11 -Wall -Wextra -O2 -pthread`.

| Image | What it shows |
|---|---|
| `q2_round_robin.png` | `./q2_rr.exe 3`, `./q2_rr.exe 1` and `./q2_rr.exe 20` on the required job set (P1..P4, arrival 0/1/2/3, burst 8/4/9/5).  Gantt chart, per-process turnaround and waiting time, and the averages 20.00 / 19.00 / 15.25 are printed for the three quantum values, which is the core of the quantum discussion in the report. |
| `q2_policies.png` | `./q2_scheduler.exe | tail -26`: the last policies of the five-policy comparison on the same job set (FCFS, SJF, SRTF, Priority, Round Robin q = 3).  Priority averages 14.25 / 7.75 and Round Robin 20.00 / 13.50, the same numbers as in the report tables. |

Microsecond and millisecond figures are wall-clock measurements, so they move a little from run
to run with machine load.  The numbers printed on screen are the numbers from the run that was
photographed; the tables in the report quote the benchmark run whose transcript is in `Output/`.
The ratios and the correctness checks (checksums, totals, exit statuses) are identical either way.
