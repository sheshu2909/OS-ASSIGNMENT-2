/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q3 (shared counter evidence)
 * Two or more threads update the SAME shared counter with and without
 * protection:  --nolock (plain ++), --mutex (pthread mutex) and
 * --atomic (C11 atomic_fetch_add).  The unprotected run loses updates, which
 * is exactly why the per-camera counters in the real system need a lock.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q3_race q3_race.c
 * Run   : ./q3_race            (runs all three modes)
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define THREADS 8
#define ITERS   500000L

/* volatile so that the compiler cannot turn the whole loop into one add:
 * every increment must be a real load-modify-store, which is what races */
static volatile long shared_counter;    /* unprotected / mutex-protected */
static atomic_long atomic_counter;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *bump_plain(void *arg)
{
    (void)arg;
    for (long i = 0; i < ITERS; i++) shared_counter++;   /* read-modify-write */
    return NULL;
}

static void *bump_mutex(void *arg)
{
    (void)arg;
    for (long i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&lock);
        shared_counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

static void *bump_atomic(void *arg)
{
    (void)arg;
    for (long i = 0; i < ITERS; i++) atomic_fetch_add(&atomic_counter, 1);
    return NULL;
}

static void run(const char *label, void *(*fn)(void *), int nthr, long *out)
{
    pthread_t th[64];
    shared_counter = 0;
    atomic_store(&atomic_counter, 0);

    for (int i = 0; i < nthr; i++)
        if (pthread_create(&th[i], NULL, fn, NULL) != 0) { perror("pthread_create"); exit(1); }
    for (int i = 0; i < nthr; i++) pthread_join(th[i], NULL);

    long got = (fn == bump_atomic) ? atomic_load(&atomic_counter) : shared_counter;
    long expected = (long)nthr * ITERS;
    *out = got;
    printf("%-22s threads=%2d  expected=%-9ld actual=%-9ld  lost=%ld\n",
           label, nthr, expected, got, expected - got);
}

int main(void)
{
    long v;
    printf("== Q3: shared counter updated by %d threads x %ld increments ==\n\n",
           THREADS, ITERS);

    run("no protection", bump_plain, THREADS, &v);
    run("pthread mutex", bump_mutex, THREADS, &v);
    run("C11 atomic", bump_atomic, THREADS, &v);

    printf("\nThe unprotected counter loses updates because counter++ is three\n");
    printf("machine instructions (load, add, store) and the scheduler can\n");
    printf("interleave two threads between the load and the store.\n");
    return 0;
}
