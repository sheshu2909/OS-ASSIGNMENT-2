/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5 alternative concept
 * Protects one shared counter with (a) a pthread mutex, (b) C11
 * atomic_fetch_add and (c) a hand-written spin lock, for 1, 4 and 16 threads.
 * Reports wall time, nanoseconds per update and the CPU time burned, which
 * shows that spinning wastes cycles when the critical section is long.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_atomic_vs_mutex q5_atomic_vs_mutex.c
 * Run   : ./q5_atomic_vs_mutex
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define ITERS 100000L
#define MAXT  16

static long counter;
static atomic_long atomic_counter;
static atomic_flag spin_flag = ATOMIC_FLAG_INIT;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int g_mode;                       /* 0 mutex, 1 atomic, 2 spin */

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void *worker(void *arg)
{
    (void)arg;
    for (long i = 0; i < ITERS; i++) {
        if (g_mode == 0) {
            pthread_mutex_lock(&lock);
            counter++;
            pthread_mutex_unlock(&lock);
        } else if (g_mode == 1) {
            atomic_fetch_add(&atomic_counter, 1);
        } else {
            while (atomic_flag_test_and_set(&spin_flag)) {
                /* spin: this thread burns CPU instead of sleeping */
            }
            counter++;
            atomic_flag_clear(&spin_flag);
        }
    }
    return NULL;
}

static void run(int mode, const char *label, int nthr)
{
    pthread_t th[MAXT];
    counter = 0;
    atomic_store(&atomic_counter, 0);
    g_mode = mode;

    clock_t cpu0 = clock();
    double t0 = now_ms();
    for (int i = 0; i < nthr; i++)
        if (pthread_create(&th[i], NULL, worker, NULL) != 0) { perror("pthread_create"); return; }
    for (int i = 0; i < nthr; i++) pthread_join(th[i], NULL);
    double ms = now_ms() - t0;
    double cpu_s = (double)(clock() - cpu0) / CLOCKS_PER_SEC;

    long ops = (long)nthr * ITERS;
    long got = (mode == 1) ? atomic_load(&atomic_counter) : counter;
    printf("%-10s %7d %11.1f %12.1f %13.2f %s\n",
           label, nthr, ms, ms * 1e6 / (double)ops, cpu_s,
           got == ops ? "OK" : "MISMATCH");
}

int main(void)
{
    printf("== Q5: cost of one protected increment, %ld updates per thread ==\n\n", ITERS);
    printf("%-10s %7s %11s %12s %13s %s\n",
           "method", "threads", "time (ms)", "ns/update", "cpu time (s)", "check");
    printf("--------------------------------------------------------------------\n");

    const int sizes[] = {1, 4, 16};
    for (unsigned k = 0; k < sizeof sizes / sizeof sizes[0]; k++) {
        run(0, "mutex", sizes[k]);
        run(1, "atomic", sizes[k]);
        run(2, "spinlock", sizes[k]);
    }

    printf("\nns/update = wall time divided by total updates\n");
    printf("cpu time is summed over all threads; spinlock numbers above 1.0 s per\n");
    printf("second of wall time mean cores were burning cycles while waiting.\n");
    return 0;
}
