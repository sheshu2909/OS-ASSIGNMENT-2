# Screenshots

These are real screen captures of the programs in this folder running in a Windows console.
Each image was taken with `PrintWindow` from the running console window, so the compile line,
the command line and the program output are exactly what the machine printed.  Nothing in these
images was edited.

Machine: Windows 11, Intel Core i7-10610U (8 logical CPUs), programs built with the MSYS2 POSIX
layer using `gcc -std=c11 -Wall -Wextra -O2 -pthread`.

| Image | What it shows |
|---|---|
| `q3_threads.png` | `./q3_camera.exe 40 150000`: number-plate reader, 64 cameras x 40 frames, sequential against one-thread-per-camera and a pool of 4, 8 and 16 threads.  This run gave 1086.9 ms sequential and speed-ups of 3.90x / 3.97x / 6.62x / 6.70x, and every parallel run reports `checksum OK`, so the pool produces the sequential result. |
| `q3_race.png` | `./q3_race.exe`: 8 threads x 500,000 increments of one shared counter.  Without protection the counter ends at 2,007,057 of the expected 4,000,000 and the program prints how many updates were lost; the `pthread_mutex_t` version and the C11 `atomic_int` version both end at exactly 4,000,000. |

Microsecond and millisecond figures are wall-clock measurements, so they move a little from run
to run with machine load.  The numbers printed on screen are the numbers from the run that was
photographed; the tables in the report quote the benchmark run whose transcript is in `Output/`.
The ratios and the correctness checks (checksums, totals, exit statuses) are identical either way.
