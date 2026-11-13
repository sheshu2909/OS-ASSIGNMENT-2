/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q2 "Complete the Code"
 * Round Robin with quantum 3 on P1..P4 (arrival 0,1,2,3; burst 8,4,9,5).
 * TODO 1 has been completed below; the rest of the program is unchanged.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q2_rr q2_rr.c
 * Run   : ./q2_rr
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#define N 4

int main(int argc, char **argv)
{
    const char *name[N] = {"P1", "P2", "P3", "P4"};
    int arrival[N] = {0, 1, 2, 3};
    int burst[N] = {8, 4, 9, 5};
    /* The supplied program fixes quantum = 3. An optional command-line
     * override lets the same binary be run with quantum 1 and 20, which is
     * what the question asks for; with no argument the behaviour is exactly
     * the specified one. */
    int quantum = (argc > 1) ? atoi(argv[1]) : 3;
    if (quantum < 1) quantum = 3;
    int left[N], finish[N], queue[64], head = 0, tail = 0, added[N] = {0};

    for (int i = 0; i < N; i++) left[i] = burst[i];

    int time = 0, done = 0;
    queue[tail++] = 0;
    added[0] = 1;
    printf("Gantt:");

    while (done < N) {
        int p = queue[head++];

        /* ------------------------------------------------------------------
         * TODO 1 (completed): run p for min(quantum, left[p]) time units.
         * We print the slice, advance the clock, reduce the remaining burst,
         * enqueue every process that has arrived meanwhile (in arrival order),
         * and then put p back at the tail if it still needs CPU time - or
         * record its finishing time and count it as done.
         * ------------------------------------------------------------------ */
        int run = (left[p] < quantum) ? left[p] : quantum; /* min(quantum, left) */
        int newtime = time + run;

        printf(" P%d(%d-%d)", p + 1, time, newtime);
        time = newtime;
        left[p] -= run;

        /* every process that has now arrived and is not yet queued */
        for (int i = 0; i < N; i++) {
            if (!added[i] && arrival[i] <= time) {
                added[i] = 1;
                queue[tail++] = i;
            }
        }

        if (left[p] > 0) {
            queue[tail++] = p;          /* back of the ready queue */
        } else {
            finish[p] = time;           /* terminated */
            done++;
        }
    }

    double tw = 0, tt = 0;
    printf("\nProcess TAT WT\n");
    for (int i = 0; i < N; i++) {
        int tat = finish[i] - arrival[i], wt = tat - burst[i];
        tt += tat;
        tw += wt;
        printf("%-7s %4d %3d\n", name[i], tat, wt);
    }
    printf("Average TAT = %.2f, average WT = %.2f\n", tt / N, tw / N);
    return 0;
}
