/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5
 * Peterson's solution for two processes, executed by two threads, with a
 * detector that counts how often both threads are inside the critical section
 * at the same time.
 *
 * Three builds are used in the report:
 *   (default)     volatile flag[] + turn           - the textbook version
 *   -DATOMIC_FIX  C11 atomic flag[] + turn         - same algorithm, but every
 *                 access is seq_cst, which inserts the memory barriers that
 *                 the textbook version lacks
 *   -DNAIVE       only flag[], no turn variable    - not a lock at all, used to
 *                 show that it deadlocks immediately
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_peterson q5_peterson.c
 *         gcc -std=c11 -Wall -Wextra -O2 -pthread -DATOMIC_FIX -o q5_peterson_atomic q5_peterson.c
 *         gcc -std=c11 -Wall -Wextra -O2 -pthread -DNAIVE -o q5_peterson_naive q5_peterson.c
 * Run   : ./q5_peterson [iterations]
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(ATOMIC_FIX)
static atomic_int flag[2] = {0, 0};
static atomic_int turn_v = 0;
#define FLAG_SET(i, v) atomic_store(&flag[i], (v))
#define FLAG_GET(i)    atomic_load(&flag[i])
#define TURN_SET(v)    atomic_store(&turn_v, (v))
#define TURN_GET()     atomic_load(&turn_v)
#else
static volatile int flag[2] = {0, 0};
static volatile int turn_v = 0;
#define FLAG_SET(i, v) (flag[i] = (v))
#define FLAG_GET(i)    (flag[i])
#define TURN_SET(v)    (turn_v = (v))
#define TURN_GET()     (turn_v)
#endif

static atomic_int in_cs;
static atomic_long violations;
static atomic_long entries;
static long ITER = 1000000L;

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void *peterson(void *arg)
{
    int i = (int)(long)arg;
    int j = 1 - i;

    for (long k = 0; k < ITER; k++) {
        FLAG_SET(i, 1);                 /* I want to enter */
        TURN_SET(j);                    /* but you may go first */
#ifdef NAIVE
        while (FLAG_GET(j)) { }         /* BROKEN: no turn, no ordering */
#else
        while (FLAG_GET(j) && TURN_GET() == j) { }   /* wait while you have priority */
#endif
        /* ---- critical section ---- */
        int prev = atomic_fetch_add(&in_cs, 1);
        if (prev != 0) atomic_fetch_add(&violations, 1);
        atomic_fetch_add(&entries, 1);
        atomic_fetch_sub(&in_cs, 1);

        FLAG_SET(i, 0);                 /* leave */
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc > 1) ITER = atol(argv[1]);

    printf("== Q5: Peterson's solution, 2 threads x %ld entries ==\n", ITER);
#if defined(NAIVE)
    printf("variant: NAIVE (only flag[], no turn variable)\n\n");
#elif defined(ATOMIC_FIX)
    printf("variant: Peterson with C11 seq_cst atomics (memory barriers)\n\n");
#else
    printf("variant: Peterson with plain volatile variables\n\n");
#endif
    fflush(stdout);

    pthread_t a, b;
    double t0 = now_ms();
    pthread_create(&a, NULL, peterson, (void *)0L);
    pthread_create(&b, NULL, peterson, (void *)1L);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    double ms = now_ms() - t0;

    printf("critical-section entries   : %ld\n", atomic_load(&entries));
    printf("mutual-exclusion violations: %ld\n", atomic_load(&violations));
    printf("elapsed                    : %.1f ms\n", ms);
    printf("RESULT: %s\n", atomic_load(&violations) == 0 ?
           "mutual exclusion held" : "mutual exclusion BROKEN");
    return 0;
}
