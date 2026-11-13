# Screenshot required - Q2 CPU Scheduling

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
gcc -std=c11 -Wall -Wextra -O2 -o q2_rr q2_rr.c
gcc -std=c11 -Wall -Wextra -O2 -o q2_scheduler q2_scheduler.c
gcc -std=c11 -Wall -Wextra -O2 -o q2_quantum q2_quantum.c
gcc -std=c11 -Wall -Wextra -O2 -o q2_mixed q2_mixed.c
./q2_rr
./q2_rr 1
./q2_rr 20
./q2_scheduler
./q2_quantum
./q2_mixed
```

## What the screenshot must show

the Round Robin Gantt chart and the averages 20.00 / 13.50 must be visible, together with the quantum table (q = 1, 2, 3, 5, 10, 20) and the mixed-workload table.

Requirements for the capture:

- a real terminal window (or the full desktop) with the shell prompt and the command visible;
- the prompt and the program output in the same frame, so the output is visibly the result of that
  command;
- readable text at the delivered resolution - capture at full window size, not a cropped thumbnail.

## Suggested file names

- `Screenshot_1.png` - compile all four programs
- `Screenshot_2.png` - run the completed Round Robin program at quantum 3, then 1 and 20
- `Screenshot_3.png` - run the five-policy calculation and the quantum study
- `Screenshot_4.png` - run the mixed-load comparison

## Cross-check after capturing

Compare the numbers in the screenshot with the matching file in `../Output/`. The text transcript
was captured from exactly these commands, so the values must agree; if they differ (for example
because the lab machine is faster or slower), keep the screenshot and note the difference in the
report's Appendix C.
