/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q5 (dining philosophers)
 * Five philosophers, five fork semaphores, 1000 meals each. The deadlock is
 * removed by breaking hold-and-wait: even-numbered philosophers take the left
 * fork first, odd-numbered ones take the right fork first, so a circular wait
 * cannot form. The program completes and reports the maximum number of
 * philosophers eating at the same time (which must stay <= 2 with 5 forks).
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_dining q5_dining.c
 * Run   : ./q5_dining
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define PHIL 5
#define MEALS 1000

static sem_t forks_[PHIL];
static atomic_int eating;
static atomic_int max_eating;
static atomic_long meals_done;

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void note_max(int v)
{
    int cur = atomic_load(&max_eating);
    while (v > cur && !atomic_compare_exchange_weak(&max_eating, &cur, v)) { }
}

static void *philosopher(void *arg)
{
    int i = (int)(long)arg;
    int left = i, right = (i + 1) % PHIL;

    for (int m = 0; m < MEALS; m++) {
        if (i % 2 == 0) {                       /* even: left then right */
            sem_wait(&forks_[left]);
            sem_wait(&forks_[right]);
        } else {                                /* odd: right then left  */
            sem_wait(&forks_[right]);
            sem_wait(&forks_[left]);
        }

        int now = atomic_fetch_add(&eating, 1) + 1;
        note_max(now);
        atomic_fetch_add(&meals_done, 1);
        atomic_fetch_sub(&eating, 1);

        sem_post(&forks_[left]);
        sem_post(&forks_[right]);
    }
    return NULL;
}

int main(void)
{
    pthread_t th[PHIL];
    for (int i = 0; i < PHIL; i++) sem_init(&forks_[i], 0, 1);

    printf("== Q5: dining philosophers, %d philosophers x %d meals ==\n", PHIL, MEALS);

    double t0 = now_ms();
    for (int i = 0; i < PHIL; i++) pthread_create(&th[i], NULL, philosopher, (void *)(long)i);
    for (int i = 0; i < PHIL; i++) pthread_join(th[i], NULL);
    double ms = now_ms() - t0;

    printf("meals completed        : %ld (expected %d)\n",
           atomic_load(&meals_done), PHIL * MEALS);
    printf("max philosophers eating: %d (must be <= 2 with %d forks)\n",
           atomic_load(&max_eating), PHIL);
    printf("elapsed                : %.1f ms\n", ms);
    printf("RESULT: %s\n",
           atomic_load(&meals_done) == PHIL * MEALS ? "no deadlock, no starvation" : "FAILED");

    for (int i = 0; i < PHIL; i++) sem_destroy(&forks_[i]);
    return 0;
}
