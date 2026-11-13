/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5 deployment decision
 * Railway booking: 16 agent threads make the same 3.2 million seat-booking
 * updates, once with one global lock, once with one lock per train and once
 * with one lock per coach. The program reports wall time, throughput and a
 * correctness check (total seats sold must equal total operations).
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_lock_granularity q5_lock_granularity.c
 * Run   : ./q5_lock_granularity
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TRAINS   8
#define COACHES  8
#define AGENTS   16
#define OPS      100000

static pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t train_lock[TRAINS];
static pthread_mutex_t coach_lock[TRAINS][COACHES];

static long seats[TRAINS][COACHES];
static int g_mode;                      /* 0 global, 1 train, 2 coach */

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static unsigned long xs_rand(unsigned long *s)
{
    unsigned long x = *s;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    return *s = x;
}

static void *agent(void *arg)
{
    unsigned long seed = 0x9E3779B97F4A7C15UL * ((unsigned long)(long)arg + 1);
    for (int i = 0; i < OPS; i++) {
        int tr = (int)(xs_rand(&seed) % TRAINS);
        int co = (int)(xs_rand(&seed) % COACHES);
        if (g_mode == 0) {
            pthread_mutex_lock(&global_lock);
            seats[tr][co]++;
            pthread_mutex_unlock(&global_lock);
        } else if (g_mode == 1) {
            pthread_mutex_lock(&train_lock[tr]);
            seats[tr][co]++;
            pthread_mutex_unlock(&train_lock[tr]);
        } else {
            pthread_mutex_lock(&coach_lock[tr][co]);
            seats[tr][co]++;
            pthread_mutex_unlock(&coach_lock[tr][co]);
        }
    }
    return NULL;
}

static double run_mode(int mode, const char *label)
{
    pthread_t th[AGENTS];
    long total = 0;

    g_mode = mode;
    for (int t = 0; t < TRAINS; t++) {
        pthread_mutex_init(&train_lock[t], NULL);
        for (int c = 0; c < COACHES; c++) {
            pthread_mutex_init(&coach_lock[t][c], NULL);
            seats[t][c] = 0;
        }
    }

    double t0 = now_ms();
    for (int i = 0; i < AGENTS; i++)
        if (pthread_create(&th[i], NULL, agent, (void *)(long)i) != 0) {
            perror("pthread_create"); exit(1);
        }
    for (int i = 0; i < AGENTS; i++) pthread_join(th[i], NULL);
    double ms = now_ms() - t0;

    for (int t = 0; t < TRAINS; t++)
        for (int c = 0; c < COACHES; c++) total += seats[t][c];

    long expected = (long)AGENTS * OPS;
    printf("%-16s %10.1f %14.0f %10ld %s\n", label, ms,
           (double)expected / (ms / 1000.0), total,
           total == expected ? "OK" : "MISMATCH");

    for (int t = 0; t < TRAINS; t++) {
        pthread_mutex_destroy(&train_lock[t]);
        for (int c = 0; c < COACHES; c++) pthread_mutex_destroy(&coach_lock[t][c]);
    }
    return ms;
}

int main(void)
{
    printf("== Q5: booking throughput vs lock granularity ==\n");
    printf("%d trains x %d coaches, %d agent threads, %d updates each\n\n",
           TRAINS, COACHES, AGENTS, OPS);
    printf("%-16s %10s %14s %10s %s\n",
           "lock scope", "time (ms)", "updates/s", "sold", "check");
    printf("--------------------------------------------------------------------\n");

    double g = run_mode(0, "one global lock");
    double t = run_mode(1, "one lock per train");
    double c = run_mode(2, "one lock per coach");

    printf("\nspeed-up = global time / mode time\n");
    printf("global %.1f ms, per-train %.1f ms (%.2fx), per-coach %.1f ms (%.2fx)\n",
           g, t, g / t, c, g / c);
    return 0;
}
