/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5 (test evidence)
 * Several producers and consumers share a 5-slot buffer. Every item carries a
 * unique id, so the program can prove the "exactly once" property: each id
 * must be produced once and consumed once.
 *
 * Build normally : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_buffer_test q5_buffer_test.c
 * Build without the mutex (failure experiment):
 *                  gcc -std=c11 -Wall -Wextra -O2 -pthread -DNOMUTEX -o q5_buffer_test_nomutex q5_buffer_test.c
 *
 * Run: ./q5_buffer_test [producers] [consumers] [repetitions]
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 5
#define ITEMS_PER_PRODUCER 500
#define MAX_PROD 8
#define MAX_ITEM (MAX_PROD * ITEMS_PER_PRODUCER)

static int buffer[SIZE];
static int in_idx = 0, out_idx = 0;
static sem_t empty_sem, full_sem;
static pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;

static int produced[MAX_ITEM + 1];
static int consumed[MAX_ITEM + 1];
static long sum_consumed = 0;

static int g_prod = 4, g_cons = 4;
static int consume_quota;

/* ------------------------- buffer operations --------------------------- */
static void put_item(int item)
{
#ifdef NOMUTEX
    /* deliberately unsynchronised: semaphores bound the count, but the
     * index update itself is a read-modify-write race */
    buffer[in_idx] = item;
    in_idx = (in_idx + 1) % SIZE;
#else
    pthread_mutex_lock(&buffer_lock);
    buffer[in_idx] = item;
    in_idx = (in_idx + 1) % SIZE;
    pthread_mutex_unlock(&buffer_lock);
#endif
}

static int get_item(void)
{
#ifdef NOMUTEX
    int item = buffer[out_idx];
    out_idx = (out_idx + 1) % SIZE;
    return item;
#else
    pthread_mutex_lock(&buffer_lock);
    int item = buffer[out_idx];
    out_idx = (out_idx + 1) % SIZE;
    pthread_mutex_unlock(&buffer_lock);
    return item;
#endif
}

/* ----------------------------- workers --------------------------------- */
static void *producer(void *arg)
{
    int id = (int)(long)arg;
    for (int k = 0; k < ITEMS_PER_PRODUCER; k++) {
        int item = id * ITEMS_PER_PRODUCER + k + 1;
        sem_wait(&empty_sem);
        put_item(item);
        sem_post(&full_sem);

        pthread_mutex_lock(&stats_lock);
        produced[item]++;
        pthread_mutex_unlock(&stats_lock);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    for (int k = 0; k < consume_quota; k++) {
        sem_wait(&full_sem);
        int item = get_item();
        sem_post(&empty_sem);

        pthread_mutex_lock(&stats_lock);
        consumed[item]++;
        sum_consumed += item;
        pthread_mutex_unlock(&stats_lock);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc > 1) g_prod = atoi(argv[1]);
    if (argc > 2) g_cons = atoi(argv[2]);
    int reps = (argc > 3) ? atoi(argv[3]) : 20;

    if (g_prod < 1 || g_prod > MAX_PROD) g_prod = 4;
    if (g_cons < 1 || g_cons > MAX_PROD) g_cons = 4;

    int total_items = g_prod * ITEMS_PER_PRODUCER;
    if (total_items % g_cons != 0) {
        printf("total items (%d) must be divisible by consumers (%d)\n",
               total_items, g_cons);
        return 1;
    }
    consume_quota = total_items / g_cons;

    long expected_sum = (long)total_items * (total_items + 1) / 2;
    int passes = 0;
    int duplicates_seen = 0, missing_seen = 0;

#ifdef NOMUTEX
    printf("== Q5: %d producers / %d consumers, mutex REMOVED ==\n", g_prod, g_cons);
#else
    printf("== Q5: %d producers / %d consumers, mutex in place ==\n", g_prod, g_cons);
#endif
    printf("buffer %d slots, %d items per producer, %d repetitions\n\n",
           SIZE, ITEMS_PER_PRODUCER, reps);

    for (int rep = 0; rep < reps; rep++) {
        pthread_t prod[MAX_PROD], cons[MAX_PROD];

        sem_init(&empty_sem, 0, SIZE);
        sem_init(&full_sem, 0, 0);
        memset(produced, 0, sizeof produced);
        memset(consumed, 0, sizeof consumed);
        sum_consumed = 0;
        in_idx = out_idx = 0;

        for (int i = 0; i < g_prod; i++)
            pthread_create(&prod[i], NULL, producer, (void *)(long)i);
        for (int i = 0; i < g_cons; i++)
            pthread_create(&cons[i], NULL, consumer, NULL);
        for (int i = 0; i < g_prod; i++) pthread_join(prod[i], NULL);
        for (int i = 0; i < g_cons; i++) pthread_join(cons[i], NULL);

        int produced_ok = 0, consumed_ok = 0, duplicates = 0, missing = 0;
        for (int i = 1; i <= total_items; i++) {
            if (produced[i] == 1) produced_ok++;
            if (consumed[i] == 1) consumed_ok++;
            if (consumed[i] > 1) duplicates += consumed[i] - 1;
            if (consumed[i] == 0) missing++;
        }

        if (rep < 3) {
            printf("run %d: produced exactly once %d/%d, consumed exactly once %d/%d, "
                   "duplicates %d, missing %d, sum %ld (expected %ld)\n",
                   rep + 1, produced_ok, total_items, consumed_ok, total_items,
                   duplicates, missing, sum_consumed, expected_sum);
        }

        int ok = (produced_ok == total_items && consumed_ok == total_items &&
                  duplicates == 0 && missing == 0 && sum_consumed == expected_sum);
        if (ok) passes++;
        else {
            if (rep >= 3) {
                printf("run %d FAILED: consumed once %d/%d, duplicates %d, missing %d, "
                       "sum %ld (expected %ld)\n", rep + 1, consumed_ok, total_items,
                       duplicates, missing, sum_consumed, expected_sum);
            }
            duplicates_seen += duplicates;
            missing_seen += missing;
        }

        sem_destroy(&empty_sem);
        sem_destroy(&full_sem);
    }

    printf("\nRESULT: %d/%d runs correct (%s)\n", passes, reps,
           passes == reps ? "PASS" : "FAIL");
    if (passes != reps)
        printf("total duplicate consumptions %d, total lost items %d\n",
               duplicates_seen, missing_seen);
    return passes == reps ? 0 : 1;
}
