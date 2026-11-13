# Screenshot required - Q1 Processes and Process States

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
gcc -std=c11 -Wall -Wextra -O2 -o q1_processes q1_processes.c
gcc -std=c11 -Wall -Wextra -O2 -pthread -o q1_overhead q1_overhead.c
./q1_processes 6 q1_pids.txt
ps -fp $(cat q1_pids.txt | tr '\n' ',')
for p in $(cat q1_pids.txt); do cat /proc/$p/status | head -4; done
./q1_overhead 200
```

## What the screenshot must show

the terminal must show the compile commands, the six child lines with their PPIDs, the ps output with every child in state S (or R) and the parent's PPID, and the measured fork/thread times. The numbers must match Output/q1_overhead.log.

Requirements for the capture:

- a real terminal window (or the full desktop) with the shell prompt and the command visible;
- the prompt and the program output in the same frame, so the output is visibly the result of that
  command;
- readable text at the delivered resolution - capture at full window size, not a cropped thumbnail.

## Suggested file names

- `Screenshot_1.png` - compile both programs
- `Screenshot_2.png` - run the fork() demo with six children
- `Screenshot_3.png` - show the processes while the children are alive
- `Screenshot_4.png` - run the creation-cost benchmark

## Cross-check after capturing

Compare the numbers in the screenshot with the matching file in `../Output/`. The text transcript
was captured from exactly these commands, so the values must agree; if they differ (for example
because the lab machine is faster or slower), keep the screenshot and note the difference in the
report's Appendix C.
