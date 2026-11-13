# Screenshot required - Q3 Threads

**No screenshot image is included in this folder.** The programs were compiled and executed and
their output was captured as text (see `../Output/`), but the machine used to build this package
(Windows 11 with the MSYS2 POSIX layer, driven without a graphical desktop session) could not
produce a genuine terminal screenshot. Rather than fabricate one, this file states exactly what
must be captured.

> A fabricated or simulated terminal image must not be submitted. A missing screenshot that is
> declared is acceptable; a fake one is misconduct.

## What to run

Run these on the lab machine (Linux, or the same MSYS2 shell) in the `Source_Code/` directory:

```
gcc -std=c11 -Wall -Wextra -O2 -pthread -o q3_camera q3_camera.c
gcc -std=c11 -Wall -Wextra -O2 -pthread -o q3_race q3_race.c
./q3_camera 40 150000
./q3_race
```

## What the screenshot must show

the five timed modes (sequential, one thread per camera, pools of 4, 8 and 16) with the same checksum, and the race table showing the lost updates against the mutex and atomic runs.

Requirements for the capture:

- a real terminal window (or the full desktop) with the shell prompt and the command visible;
- the prompt and the program output in the same frame, so the output is visibly the result of that
  command;
- readable text at the delivered resolution - capture at full window size, not a cropped thumbnail.

## Suggested file names

- `Screenshot_1.png` - compile both programs
- `Screenshot_2.png` - run the camera pool for every mode
- `Screenshot_3.png` - run the shared-counter comparison

## Cross-check after capturing

Compare the numbers in the screenshot with the matching file in `../Output/`. The text transcript
was captured from exactly these commands, so the values must agree; if they differ (for example
because the lab machine is faster or slower), keep the screenshot and note the difference in the
report's Appendix C.
