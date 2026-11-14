# Screenshots

These are real screen captures of the programs in this folder running in a Windows console.
Each image was taken with `PrintWindow` from the running console window, so the compile line,
the command line and the program output are exactly what the machine printed.  Nothing in these
images was edited.

Machine: Windows 11, Intel Core i7-10610U (8 logical CPUs), programs built with the MSYS2 POSIX
layer using `gcc -std=c11 -Wall -Wextra -O2 -pthread`.

| Image | What it shows |
|---|---|
| `q4_pipe_and_shm.png` | `./q4_pipe.exe` and `./q4_shm.exe`: the same producer/analyser pipeline over an anonymous pipe and over a shared `mmap` region.  Both print the same analysis (6 readings, average 35.5, max 41); the shared-memory version also writes the average back where the reader can see it, which the pipe cannot do without a second pipe. |
| `q4_bench.png` | `./q4_bench.exe`: 1 MB, 10 MB and 100 MB moved through a 64 KiB pipe chunk by chunk against the same volume through shared memory (best of three).  The table prints elapsed milliseconds and MB/s for both mechanisms plus the shm/pipe ratio. |

Microsecond and millisecond figures are wall-clock measurements, so they move a little from run
to run with machine load.  The numbers printed on screen are the numbers from the run that was
photographed; the tables in the report quote the benchmark run whose transcript is in `Output/`.
The ratios and the correctness checks (checksums, totals, exit statuses) are identical either way.
