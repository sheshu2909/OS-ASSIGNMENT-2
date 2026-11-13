# Screenshot required - Q5 Process Synchronisation, Semaphores and Monitors

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
gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_bounded_buffer q5_bounded_buffer.c
gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_buffer_test q5_buffer_test.c
./q5_bounded_buffer
for i in $(seq 100); do ./q5_bounded_buffer; done | sort | uniq -c
./q5_buffer_test 1 1 100
./q5_buffer_test 4 4 100
gcc -std=c11 -Wall -Wextra -O2 -pthread -DNOMUTEX -o q5_buffer_test_nomutex q5_buffer_test.c
./q5_buffer_test_nomutex 4 4 20
gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_peterson q5_peterson.c
./q5_peterson 1000000
gcc -std=c11 -Wall -Wextra -O2 -pthread -DATOMIC_FIX -o q5_peterson_atomic q5_peterson.c
./q5_peterson_atomic 1000000
gcc -std=c11 -Wall -Wextra -O2 -pthread -DNAIVE -o q5_peterson_naive q5_peterson.c
timeout 10 ./q5_peterson_naive 200000; echo "exit status: $?"
./q5_rw
./q5_dining
./q5_monitor
./q5_lock_granularity
./q5_atomic_vs_mutex
```

## What the screenshot must show

the required line "items consumed: 20, sum = 210 (expected 210)", the 100/100 PASS lines, the failure summary of the no-mutex run, the Peterson violation counts (2, 0, deadlock), the readers-writers zero violations, the dining count, and the two benchmark tables.

Requirements for the capture:

- a real terminal window (or the full desktop) with the shell prompt and the command visible;
- the prompt and the program output in the same frame, so the output is visibly the result of that
  command;
- readable text at the delivered resolution - capture at full window size, not a cropped thumbnail.

## Suggested file names

- `Screenshot_1.png` - compile the supplied program and the test harness
- `Screenshot_2.png` - run the required program once, then 100 times
- `Screenshot_3.png` - run the 1/1 and 4/4 tests, then the no-mutex build
- `Screenshot_4.png` - compile and run the no-mutex failure test
- `Screenshot_5.png` - run Peterson's solution in all three builds
- `Screenshot_6.png` - run readers-writers, dining philosophers and the monitor
- `Screenshot_7.png` - run the lock-granularity and atomic/mutex benchmarks

## Cross-check after capturing

Compare the numbers in the screenshot with the matching file in `../Output/`. The text transcript
was captured from exactly these commands, so the values must agree; if they differ (for example
because the lab machine is faster or slower), keep the screenshot and note the difference in the
report's Appendix C.
