/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5 (monitor version)
 * The same bounded buffer written as a MONITOR: one mutex plus two condition
 * variables, not_full and not_empty. The buffer indices are local to the
 * monitor functions, so a caller cannot touch them without holding the lock -
 * this is what makes the monitor solution easier to get right than raw
 * semaphores.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_monitor q5_monitor.c
 * Run   : ./q5_monitor
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define SIZE 5
#define PRODUCERS 4
#define CONSUMERS 4
#define PER_PRODUCER 500
#define TOTAL (PRODUCERS * PER_PRODUCER)

static int buffer[SIZE];
static int count = 0, in_idx = 0, out_idx = 0;

static pthread_mutex_t monitor_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;
static int consumed_count[TOTAL + 1];
static long sum_consumed;

/* ------------------------- monitor procedures -------------------------- */
static void monitor_put(int item)
{
    pthread_mutex_lock(&monitor_lock);
    while (count == SIZE)
        pthread_cond_wait(&not_full, &monitor_lock);    /* release + sleep */
    buffer[in_idx] = item;
    in_idx = (in_idx + 1) % SIZE;
    count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&monitor_lock);
}

static int monitor_get(void)
{
    pthread_mutex_lock(&monitor_lock);
    while (count == 0)
        pthread_cond_wait(&not_empty, &monitor_lock);
    int item = buffer[out_idx];
    out_idx = (out_idx + 1) % SIZE;
    count--;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&monitor_lock);
    return item;
}

/* ------------------------------ workers -------------------------------- */
static void *producer(void *arg)
{
    int id = (int)(long)arg;
    for (int k = 0; k < PER_PRODUCER; k++)
        monitor_put(id * PER_PRODUCER + k + 1);
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    for (int k = 0; k < TOTAL / CONSUMERS; k++) {
        int item = monitor_get();
        pthread_mutex_lock(&stats_lock);
        consumed_count[item]++;
        sum_consumed += item;
        pthread_mutex_unlock(&stats_lock);
    }
    return NULL;
}

int main(void)
{
    pthread_t prod[PRODUCERS], cons[CONSUMERS];

    printf("== Q5: bounded buffer as a monitor (%d producers / %d consumers) ==\n\n",
           PRODUCERS, CONSUMERS);

    for (int i = 0; i < PRODUCERS; i++) pthread_create(&prod[i], NULL, producer, (void *)(long)i);
    for (int i = 0; i < CONSUMERS; i++) pthread_create(&cons[i], NULL, consumer, NULL);
    for (int i = 0; i < PRODUCERS; i++) pthread_join(prod[i], NULL);
    for (int i = 0; i < CONSUMERS; i++) pthread_join(cons[i], NULL);

    int ok = 0, dup = 0, miss = 0;
    for (int i = 1; i <= TOTAL; i++) {
        if (consumed_count[i] == 1) ok++;
        if (consumed_count[i] > 1) dup += consumed_count[i] - 1;
        if (consumed_count[i] == 0) miss++;
    }

    long expected = (long)TOTAL * (TOTAL + 1) / 2;
    printf("items consumed exactly once : %d/%d\n", ok, TOTAL);
    printf("duplicates / missing        : %d / %d\n", dup, miss);
    printf("sum                         : %ld (expected %ld)\n", sum_consumed, expected);
    printf("RESULT: %s\n", (ok == TOTAL && dup == 0 && miss == 0 && sum_consumed == expected)
           ? "PASS" : "FAIL");
    return 0;
}
