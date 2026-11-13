/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5 (readers-writers)
 * Readers-writers with semaphores: any number of readers may read together,
 * a writer needs exclusive access. A detector counts violations of the
 * invariant "no reader while a writer is inside".
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_rw q5_rw.c
 * Run   : ./q5_rw
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define READERS 8
#define WRITERS 4
#define R_ITERS 20000
#define W_ITERS 5000

static sem_t rw_mutex;              /* held by the first reader / the writer */
static sem_t read_mutex;            /* protects read_count                  */
static int read_count = 0;

static atomic_int readers_active;
static atomic_int writer_active;
static atomic_long violations;
static atomic_int max_readers;
static long shared_record;

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void note_max(int v)
{
    int cur = atomic_load(&max_readers);
    while (v > cur && !atomic_compare_exchange_weak(&max_readers, &cur, v)) { }
}

static void *reader(void *arg)
{
    (void)arg;
    for (int i = 0; i < R_ITERS; i++) {
        sem_wait(&read_mutex);
        read_count++;
        if (read_count == 1) sem_wait(&rw_mutex);   /* first reader locks out writers */
        sem_post(&read_mutex);

        int now = atomic_fetch_add(&readers_active, 1) + 1;
        note_max(now);
        if (atomic_load(&writer_active)) atomic_fetch_add(&violations, 1);
        (void)shared_record;                        /* the "read" */
        sched_yield();
        atomic_fetch_sub(&readers_active, 1);

        sem_wait(&read_mutex);
        read_count--;
        if (read_count == 0) sem_post(&rw_mutex);   /* last reader lets writers in */
        sem_post(&read_mutex);
    }
    return NULL;
}

static void *writer(void *arg)
{
    (void)arg;
    for (int i = 0; i < W_ITERS; i++) {
        sem_wait(&rw_mutex);                        /* exclusive */
        atomic_store(&writer_active, 1);
        if (atomic_load(&readers_active) != 0) atomic_fetch_add(&violations, 1);
        shared_record++;                            /* the "write" */
        atomic_store(&writer_active, 0);
        sem_post(&rw_mutex);
    }
    return NULL;
}

int main(void)
{
    pthread_t rd[READERS], wr[WRITERS];
    sem_init(&rw_mutex, 0, 1);
    sem_init(&read_mutex, 0, 1);

    printf("== Q5: readers-writers with semaphores ==\n");
    printf("%d readers x %d reads, %d writers x %d writes\n\n",
           READERS, R_ITERS, WRITERS, W_ITERS);

    double t0 = now_ms();
    for (int i = 0; i < READERS; i++) pthread_create(&rd[i], NULL, reader, NULL);
    for (int i = 0; i < WRITERS; i++) pthread_create(&wr[i], NULL, writer, NULL);
    for (int i = 0; i < READERS; i++) pthread_join(rd[i], NULL);
    for (int i = 0; i < WRITERS; i++) pthread_join(wr[i], NULL);
    double ms = now_ms() - t0;

    printf("elapsed                    : %.1f ms\n", ms);
    printf("writes applied             : %ld (expected %d)\n",
           shared_record, WRITERS * W_ITERS);
    printf("maximum concurrent readers : %d\n", atomic_load(&max_readers));
    printf("invariant violations       : %ld\n", atomic_load(&violations));
    printf("RESULT: %s\n", atomic_load(&violations) == 0 ?
           "writer exclusion held, readers ran concurrently" : "VIOLATION");

    sem_destroy(&rw_mutex);
    sem_destroy(&read_mutex);
    return 0;
}
