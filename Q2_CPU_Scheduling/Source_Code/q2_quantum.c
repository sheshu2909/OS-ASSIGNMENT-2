/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q2 quantum study
 * Runs Round Robin on the Q2 job set for quantum = 1, 2, 3, 5, 10, 20 and
 * reports average waiting time, average turnaround time, average response
 * time and the number of time slices (each slice boundary is one context
 * switch).
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q2_quantum q2_quantum.c
 * Run   : ./q2_quantum
 * ==========================================================================*/
#include <stdio.h>

#define N 4

static const int arrival[N] = {0, 1, 2, 3};
static const int burst[N] = {8, 4, 9, 5};

static void simulate(int quantum, double *avg_tat, double *avg_wt, double *avg_rt,
                     int *slices, int *maxq)
{
    int rem[N], comp[N], first[N], added[N] = {0};
    int queue[256], head = 0, tail = 0;
    int t = 0, done = 0;

    for (int i = 0; i < N; i++) { rem[i] = burst[i]; first[i] = -1; }
    queue[tail++] = 0;
    added[0] = 1;
    *slices = 0;
    *maxq = 1;

    while (done < N) {
        int p = queue[head++];
        int run = (rem[p] < quantum) ? rem[p] : quantum;

        if (first[p] < 0) first[p] = t;
        t += run;
        rem[p] -= run;
        (*slices)++;

        for (int i = 0; i < N; i++)
            if (!added[i] && arrival[i] <= t) { added[i] = 1; queue[tail++] = i; }

        if (tail - head > *maxq) *maxq = tail - head;

        if (rem[p] > 0) queue[tail++] = p;
        else { comp[p] = t; done++; }
    }

    double st = 0, sw = 0, sr = 0;
    for (int i = 0; i < N; i++) {
        int tat = comp[i] - arrival[i];
        st += tat;
        sw += tat - burst[i];
        sr += first[i] - arrival[i];
    }
    *avg_tat = st / N;
    *avg_wt = sw / N;
    *avg_rt = sr / N;
}

int main(void)
{
    const int quanta[] = {1, 2, 3, 5, 10, 20};

    printf("Q2 quantum study - Round Robin on P1..P4 (AT 0,1,2,3 / BT 8,4,9,5)\n\n");
    printf("%-8s %8s %8s %8s %9s %8s\n",
           "quantum", "avg WT", "avg TAT", "avg RT", "slices", "max rq");
    printf("--------------------------------------------------------------\n");

    for (unsigned k = 0; k < sizeof quanta / sizeof quanta[0]; k++) {
        double at, aw, ar;
        int sl, mq;
        simulate(quanta[k], &at, &aw, &ar, &sl, &mq);
        printf("%-8d %8.2f %8.2f %8.2f %9d %8d\n",
               quanta[k], aw, at, ar, sl, mq);
    }

    printf("\nslices = number of time slices executed = number of context switches\n");
    printf("max rq = maximum ready-queue length observed\n");
    return 0;
}
