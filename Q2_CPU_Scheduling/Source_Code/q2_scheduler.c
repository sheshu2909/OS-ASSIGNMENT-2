/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q2
 * Runs the SAME job set through FCFS, SJF, SRTF, non-preemptive priority and
 * Round Robin, printing a Gantt chart, CT/TAT/WT/RT per process and averages.
 *
 * Job set (identical to the "Complete the Code" program):
 *   P1 arrival 0 burst 8 | P2 arrival 1 burst 4
 *   P3 arrival 2 burst 9 | P4 arrival 3 burst 5
 * Priorities (smaller = more important) are stated explicitly because the
 * question does not supply them: P1=2, P2=1, P3=4, P4=3.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q2_scheduler q2_scheduler.c
 * Run   : ./q2_scheduler
 * ==========================================================================*/
#include <stdio.h>
#include <string.h>

#define N 4
#define MAXSEG 128

static const char *const name[N] = {"P1", "P2", "P3", "P4"};
static const int arrival[N] = {0, 1, 2, 3};
static const int burst[N] = {8, 4, 9, 5};
static const int priority[N] = {2, 1, 4, 3};

typedef struct { int pid, start, end; } Seg;

static void add_seg(Seg *s, int *n, int pid, int start, int end)
{
    if (*n > 0 && s[*n - 1].pid == pid && s[*n - 1].end == start) {
        s[*n - 1].end = end;                 /* merge contiguous slices */
        return;
    }
    s[*n].pid = pid;
    s[*n].start = start;
    s[*n].end = end;
    (*n)++;
}

static void report(const char *label, const int comp[N], const int first[N],
                   const Seg *seg, int nseg)
{
    double st = 0.0, sw = 0.0, sr = 0.0;

    printf("\n=== %s ===\n  Gantt: ", label);
    for (int i = 0; i < nseg; i++)
        printf("%s(%d-%d) ", name[seg[i].pid], seg[i].start, seg[i].end);
    printf("\n\n  %-8s %4s %4s %4s %5s %4s %4s\n",
           "Process", "AT", "BT", "CT", "TAT", "WT", "RT");
    for (int i = 0; i < N; i++) {
        int tat = comp[i] - arrival[i];
        int wt = tat - burst[i];
        int rt = first[i] - arrival[i];
        st += tat; sw += wt; sr += rt;
        printf("  %-8s %4d %4d %4d %5d %4d %4d\n",
               name[i], arrival[i], burst[i], comp[i], tat, wt, rt);
    }
    printf("  averages: TAT %.2f  WT %.2f  RT %.2f\n", st / N, sw / N, sr / N);
}

/* ------------------------------- FCFS ---------------------------------- */
static void fcfs(void)
{
    int order[N] = {0, 1, 2, 3};
    int comp[N], first[N];
    Seg seg[MAXSEG];
    int nseg = 0, t = 0;

    for (int i = 0; i < N; i++)                    /* stable sort by arrival */
        for (int j = i + 1; j < N; j++)
            if (arrival[order[j]] < arrival[order[i]]) {
                int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
            }

    for (int k = 0; k < N; k++) {
        int p = order[k];
        if (t < arrival[p]) t = arrival[p];
        first[p] = t;
        add_seg(seg, &nseg, p, t, t + burst[p]);
        t += burst[p];
        comp[p] = t;
    }
    report("FCFS (non-preemptive)", comp, first, seg, nseg);
}

/* --------------------- SJF (non-preemptive) ---------------------------- */
static void sjf(void)
{
    int rem[N], comp[N], first[N];
    Seg seg[MAXSEG];
    int nseg = 0, t = 0, done = 0;

    for (int i = 0; i < N; i++) { rem[i] = burst[i]; first[i] = -1; }

    while (done < N) {
        int pick = -1;
        for (int p = 0; p < N; p++) {
            if (rem[p] <= 0 || arrival[p] > t) continue;
            if (pick < 0 || rem[p] < rem[pick] ||
                (rem[p] == rem[pick] && arrival[p] < arrival[pick]))
                pick = p;
        }
        if (pick < 0) { t++; continue; }        /* CPU idle */

        first[pick] = t;
        add_seg(seg, &nseg, pick, t, t + rem[pick]);
        t += rem[pick];
        comp[pick] = t;
        rem[pick] = 0;
        done++;
    }
    report("SJF (non-preemptive, shortest burst first)", comp, first, seg, nseg);
}

/* ----------------------------- SRTF ------------------------------------ */
static void srtf(void)
{
    int rem[N], comp[N], first[N];
    Seg seg[MAXSEG];
    int nseg = 0, t = 0, done = 0;

    for (int i = 0; i < N; i++) { rem[i] = burst[i]; first[i] = -1; }

    while (done < N) {
        int pick = -1;
        for (int p = 0; p < N; p++) {
            if (rem[p] <= 0 || arrival[p] > t) continue;
            if (pick < 0 || rem[p] < rem[pick] ||
                (rem[p] == rem[pick] && arrival[p] < arrival[pick]))
                pick = p;
        }
        if (pick < 0) { t++; continue; }

        if (first[pick] < 0) first[pick] = t;
        add_seg(seg, &nseg, pick, t, t + 1);    /* one time unit */
        rem[pick]--;
        t++;
        if (rem[pick] == 0) { comp[pick] = t; done++; }
    }
    report("SRTF (preemptive shortest remaining time first)", comp, first, seg, nseg);
}

/* --------------------- Priority (non-preemptive) ----------------------- */
static void prio(void)
{
    int rem[N], comp[N], first[N];
    Seg seg[MAXSEG];
    int nseg = 0, t = 0, done = 0;

    for (int i = 0; i < N; i++) { rem[i] = burst[i]; first[i] = -1; }

    while (done < N) {
        int pick = -1;
        for (int p = 0; p < N; p++) {
            if (rem[p] <= 0 || arrival[p] > t) continue;
            if (pick < 0 || priority[p] < priority[pick] ||
                (priority[p] == priority[pick] && arrival[p] < arrival[pick]))
                pick = p;
        }
        if (pick < 0) { t++; continue; }

        first[pick] = t;
        add_seg(seg, &nseg, pick, t, t + rem[pick]);
        t += rem[pick];
        comp[pick] = t;
        rem[pick] = 0;
        done++;
    }
    report("Priority (non-preemptive, 1 = highest)", comp, first, seg, nseg);
}

/* ------------------------------ RR ------------------------------------- */
static void rr(int quantum)
{
    int rem[N], comp[N], first[N], added[N] = {0};
    int queue[256], head = 0, tail = 0;
    Seg seg[MAXSEG];
    int nseg = 0, t = 0, done = 0, slices = 0;
    char label[80];

    for (int i = 0; i < N; i++) { rem[i] = burst[i]; first[i] = -1; }
    queue[tail++] = 0;
    added[0] = 1;

    while (done < N) {
        int p = queue[head++];
        int run = (rem[p] < quantum) ? rem[p] : quantum;

        if (first[p] < 0) first[p] = t;
        add_seg(seg, &nseg, p, t, t + run);
        t += run;
        rem[p] -= run;
        slices++;

        for (int i = 0; i < N; i++)
            if (!added[i] && arrival[i] <= t) { added[i] = 1; queue[tail++] = i; }

        if (rem[p] > 0) queue[tail++] = p;
        else { comp[p] = t; done++; }
    }
    snprintf(label, sizeof label, "Round Robin (quantum = %d), %d slices", quantum, slices);
    report(label, comp, first, seg, nseg);
}

int main(void)
{
    printf("Job set: P1(AT0,BT8) P2(AT1,BT4) P3(AT2,BT9) P4(AT3,BT5)\n");
    printf("Priorities (1 = highest): P1=2 P2=1 P3=4 P4=3\n");

    fcfs();
    sjf();
    srtf();
    prio();
    rr(3);

    printf("\nAll five policies above ran on the same job set, so the averages\n");
    printf("are directly comparable.\n");
    return 0;
}
