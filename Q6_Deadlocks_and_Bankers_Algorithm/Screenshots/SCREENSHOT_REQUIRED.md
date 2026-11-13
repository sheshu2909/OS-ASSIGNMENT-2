# Screenshot required - Q6 Deadlock Handling and the Banker's Algorithm

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
gcc -std=c11 -Wall -Wextra -O2 -o q6_bankers q6_bankers.c
gcc -std=c11 -Wall -Wextra -O2 -o q6_bankers_trace q6_bankers_trace.c
gcc -std=c11 -Wall -Wextra -O2 -o q6_bankers_scale q6_bankers_scale.c
./q6_bankers
./q6_bankers_trace
./q6_bankers_scale
```

## What the screenshot must show

the four required lines (initial state safe P1 P3 P4 P0 P2; P1 granted; P4 must wait; P0 denied as unsafe), the trace showing Work becoming (2,1,0), and the scaling table.

Requirements for the capture:

- a real terminal window (or the full desktop) with the shell prompt and the command visible;
- the prompt and the program output in the same frame, so the output is visibly the result of that
  command;
- readable text at the delivered resolution - capture at full window size, not a cropped thumbnail.

## Suggested file names

- `Screenshot_1.png` - compile all three programs
- `Screenshot_2.png` - run the required program
- `Screenshot_3.png` - run the step-by-step trace and the scaling benchmark

## Cross-check after capturing

Compare the numbers in the screenshot with the matching file in `../Output/`. The text transcript
was captured from exactly these commands, so the values must agree; if they differ (for example
because the lab machine is faster or slower), keep the screenshot and note the difference in the
report's Appendix C.
