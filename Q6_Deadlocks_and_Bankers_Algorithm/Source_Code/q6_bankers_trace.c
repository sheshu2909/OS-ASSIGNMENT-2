/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q6 (hand-analysis support)
 * Prints the Need matrix and then traces the safety algorithm step by step
 * for (a) the initial state and (b) the state after tentatively granting
 * P0's request (0,2,0), showing exactly which resource blocks every process.
 * This is the programmatic version of the "show by hand why the request of
 * P0 for (0,2,0) leads to an unsafe state" requirement.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q6_bankers_trace q6_bankers_trace.c
 * Run   : ./q6_bankers_trace
 * ==========================================================================*/
#include <stdio.h>
#include <string.h>

#define P 5
#define R 3

static int maxm[P][R] = {{7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
static int alloc[P][R] = {{0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}};

static void print_work(const char *tag, const int work[R])
{
    printf("  %-22s Work = (%d, %d, %d)\n", tag, work[0], work[1], work[2]);
}

static void print_matrix(const char *title, int m[P][R])
{
    printf("%s\n        R0  R1  R2\n", title);
    for (int p = 0; p < P; p++)
        printf("  P%d    %2d  %2d  %2d\n", p, m[p][0], m[p][1], m[p][2]);
}

/* Traces the safety algorithm; returns 1 if the state is safe. */
static int trace_safe(const char *title, int avail[R], int maxx[P][R], int allocx[P][R])
{
    int work[R], finish[P] = {0}, count = 0, step = 0;
    memcpy(work, avail, sizeof work);

    printf("\n--- %s ---\n", title);
    print_work("initial", work);

    while (count < P) {
        int found = 0;
        for (int p = 0; p < P; p++) {
            if (finish[p]) continue;
            int ok = 1, bad = -1;
            for (int r = 0; r < R; r++)
                if (maxx[p][r] - allocx[p][r] > work[r]) { ok = 0; bad = r; }

            if (!ok) {
                if (step == 0)
                    printf("  P%d cannot run: needs (%d,%d,%d), R%d is short\n", p,
                           maxx[p][0] - allocx[p][0], maxx[p][1] - allocx[p][1],
                           maxx[p][2] - allocx[p][2], bad);
                continue;
            }
            step++;
            printf("  step %d: P%d needs (%d,%d,%d) <= Work (%d,%d,%d) -> P%d finishes\n",
                   step, p, maxx[p][0] - allocx[p][0], maxx[p][1] - allocx[p][1],
                   maxx[p][2] - allocx[p][2], work[0], work[1], work[2], p);
            for (int r = 0; r < R; r++) work[r] += allocx[p][r];
            finish[p] = 1;
            count++;
            found = 1;
            print_work("        new", work);
        }
        if (!found) {
            printf("  no unfinished process can be satisfied -> UNSAFE state\n");
            return 0;
        }
    }
    printf("  all processes finished -> SAFE state\n");
    return 1;
}

int main(void)
{
    int avail[R] = {3, 3, 2};
    int need[P][R];

    printf("== Q6: Banker's algorithm, step-by-step trace ==\n");
    for (int p = 0; p < P; p++)
        for (int r = 0; r < R; r++)
            need[p][r] = maxm[p][r] - alloc[p][r];

    print_matrix("Max matrix:", maxm);
    print_matrix("Allocation matrix:", alloc);
    print_matrix("Need matrix (Max - Allocation):", need);

    trace_safe("initial state", avail, maxm, alloc);

    /* ---- request 1: P1 asks for (1,0,2) and it is granted ---- */
    avail[0] -= 1;
    avail[2] -= 2;
    alloc[1][0] += 1;
    alloc[1][2] += 2;
    printf("\nP1 asked for (1, 0, 2): granted, Available becomes (%d, %d, %d)\n",
           avail[0], avail[1], avail[2]);
    trace_safe("state after granting P1 (1,0,2)", avail, maxm, alloc);

    /* ---- request 2: P4 asks for (3,3,0) and must wait ---- */
    printf("\nP4 asks for (3, 3, 0): it is within P4's remaining need (4,3,1),\n");
    printf("but Available[0] = %d < 3, so the request must wait.\n", avail[0]);

    /* ---- request 3: P0 asks for (0,2,0): tentative grant, then test ---- */
    int av2[R], al2[P][R];
    memcpy(av2, avail, sizeof av2);
    memcpy(al2, alloc, sizeof al2);
    av2[1] -= 2;
    al2[0][1] += 2;

    printf("\nTentatively granting P0 (0, 2, 0):\n");
    printf("  Available becomes (%d, %d, %d), Allocation[P0] becomes (%d, %d, %d)\n",
           av2[0], av2[1], av2[2], al2[0][0], al2[0][1], al2[0][2]);

    int safe = trace_safe("state after granting P0 (0,2,0)", av2, maxm, al2);
    printf("\nDECISION: P0's (0,2,0) request is %s\n",
           safe ? "granted" : "denied (unsafe) and the old state is restored");
    return 0;
}
