/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q6 "Complete the Code"
 * Banker's algorithm: the safety algorithm plus the resource-request algorithm.
 * TODO 1 has been completed; the rest of the program is unchanged.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q6_bankers q6_bankers.c
 * Run   : ./q6_bankers
 * ==========================================================================*/
#include <stdio.h>
#include <string.h>
#define P 5
#define R 3

int avail[R] = {3, 3, 2};
int maxm[P][R] = {{7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
int alloc[P][R] = {{0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}};

/* Returns 1 if the state is safe and stores a safe sequence in seq. */
int is_safe(int seq[P])
{
    int work[R], finish[P] = {0}, count = 0;
    memcpy(work, avail, sizeof work);

    while (count < P) {
        int found = 0;
        for (int p = 0; p < P; p++) {
            if (finish[p]) continue;

            /* ------------------------------------------------------------
             * TODO 1 (completed): a process may finish only if every one of
             * its remaining needs (max - alloc) fits in the current Work
             * vector. If so, release everything it holds back to Work, mark
             * it finished, append it to the safe sequence and note progress.
             * ---------------------------------------------------------- */
            int can_finish = 1;
            for (int r = 0; r < R; r++) {
                if (maxm[p][r] - alloc[p][r] > work[r]) {
                    can_finish = 0;
                    break;
                }
            }
            if (can_finish) {
                for (int r = 0; r < R; r++)
                    work[r] += alloc[p][r];     /* resources are returned */
                finish[p] = 1;
                seq[count++] = p;
                found = 1;
            }
        }
        if (!found) return 0;                   /* no progress -> unsafe */
    }
    return 1;
}

void request(int p, const int req[R])
{
    for (int r = 0; r < R; r++)
        if (req[r] > maxm[p][r] - alloc[p][r] || req[r] > avail[r]) {
            printf("P%d request: must wait (not available)\n", p);
            return;
        }

    for (int r = 0; r < R; r++) {
        avail[r] -= req[r];
        alloc[p][r] += req[r];
    }

    int seq[P];
    if (is_safe(seq)) {
        printf("P%d request: granted, safe sequence", p);
        for (int i = 0; i < P; i++) printf(" P%d", seq[i]);
        printf("\n");
    } else {
        for (int r = 0; r < R; r++) {
            avail[r] += req[r];
            alloc[p][r] -= req[r];
        }
        printf("P%d request: denied (unsafe)\n", p);
    }
}

int main(void)
{
    int seq[P];
    if (is_safe(seq)) {
        printf("initial state safe:");
        for (int i = 0; i < P; i++) printf(" P%d", seq[i]);
        printf("\n");
    }
    request(1, (int[R]){1, 0, 2});
    request(4, (int[R]){3, 3, 0});
    request(0, (int[R]){0, 2, 0});
    return 0;
}
