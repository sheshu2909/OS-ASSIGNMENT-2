/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q6-E (measurement)
 * Times the Banker's safety check for n = 10, 100, 1000 and 5000 processes
 * (m = 3 resource types) and for m = 3, 6, 12 at n = 1000.
 *
 * The generated state is the worst case for the algorithm: process p needs
 * (n - p) units of every resource while holding one, and one unit is free.
 * Exactly one process can finish per full scan of the process list, so the
 * number of need-checks is about m * n * (n + 1) / 2 - the quadratic term the
 * algorithm is famous for. Dividing the measured time by that count must give
 * a nearly constant nanoseconds-per-check figure if the prediction is right.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q6_bankers_scale q6_bankers_scale.c
 * Run   : ./q6_bankers_scale
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

/* Safety algorithm: O(m * n^2) in the worst case. Scratch buffers are passed
 * in so that malloc cost is not part of the measurement. */
static int is_safe(int n, int m, const int *maxm, const int *alloc,
                   const int *avail, int *work, int *finish, int *seq)
{
    int count = 0;

    memcpy(work, avail, (size_t)m * sizeof(int));
    memset(finish, 0, (size_t)n * sizeof(int));

    while (count < n) {
        int found = 0;
        for (int p = 0; p < n; p++) {
            if (finish[p]) continue;

            int ok = 1;
            for (int r = 0; r < m; r++) {
                if (maxm[p * m + r] - alloc[p * m + r] > work[r]) { ok = 0; break; }
            }
            if (!ok) continue;

            for (int r = 0; r < m; r++) work[r] += alloc[p * m + r];
            finish[p] = 1;
            seq[count++] = p;
            found = 1;
        }
        if (!found) return 0;               /* unsafe */
    }
    return 1;
}

static void build_worst_case(int n, int m, int *maxm, int *alloc, int *avail)
{
    /* Process p needs n-p units of the LAST resource and nothing of the
     * others. Only one process can finish per full scan, so the algorithm
     * performs about n*(n+1)/2 scans AND every failing test walks all m
     * resources before it gives up - exactly the worst case for m*n^2. */
    for (int p = 0; p < n; p++)
        for (int r = 0; r < m; r++) {
            if (r == m - 1) {
                maxm[p * m + r] = n - p + 1;    /* need = n - p */
                alloc[p * m + r] = 1;
            } else {
                maxm[p * m + r] = 1;            /* need = 0 */
                alloc[p * m + r] = 1;
            }
        }
    for (int r = 0; r < m; r++) avail[r] = 1;
}

static void measure(int n, int m, int reps)
{
    int *maxm = malloc((size_t)n * m * sizeof(int));
    int *alloc = malloc((size_t)n * m * sizeof(int));
    int *avail = malloc((size_t)m * sizeof(int));
    int *work = malloc((size_t)m * sizeof(int));
    int *finish = malloc((size_t)n * sizeof(int));
    int *seq = malloc((size_t)n * sizeof(int));

    if (!maxm || !alloc || !avail || !work || !finish || !seq) {
        printf("out of memory for n=%d\n", n);
        return;
    }
    build_worst_case(n, m, maxm, alloc, avail);

    volatile int sink = 0;
    double t0 = now_ms();
    for (int k = 0; k < reps; k++)
        sink += is_safe(n, m, maxm, alloc, avail, work, finish, seq);
    double total = now_ms() - t0;
    double per_us = total * 1000.0 / reps;

    double checks = (double)m * n * (n + 1) / 2.0;
    printf("%6d %6d %8d %11.2f %14.2f %13.2f %8d\n",
           n, m, reps, total, per_us, per_us * 1000.0 / checks, sink / reps);

    free(maxm); free(alloc); free(avail);
    free(work); free(finish); free(seq);
}

int main(void)
{
    printf("== Q6-E: Banker's safety check, measured against O(m * n^2) ==\n\n");
    printf("%6s %6s %8s %11s %14s %13s %8s\n",
           "n", "m", "reps", "total (ms)", "per check(us)", "ns per need", "safe");
    printf("-------------------------------------------------------------------------\n");

    measure(10, 3, 20000);
    measure(100, 3, 2000);
    measure(1000, 3, 200);
    measure(5000, 3, 20);

    printf("\nEffect of the number of resource types (n = 1000):\n");
    printf("%6s %6s %8s %11s %14s %13s %8s\n",
           "n", "m", "reps", "total (ms)", "per check(us)", "ns per need", "safe");
    printf("-------------------------------------------------------------------------\n");
    measure(1000, 3, 200);
    measure(1000, 6, 200);
    measure(1000, 12, 200);

    printf("\nIf ns-per-need stays roughly constant, the measured growth is the\n");
    printf("predicted O(m * n^2); each tenfold increase in n should cost about\n");
    printf("100 times more per check.\n");
    return 0;
}
