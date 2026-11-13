/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q4
 * Same weather-station pipeline through SHARED MEMORY instead of a pipe.
 * The readings live in one mmap(MAP_SHARED|MAP_ANONYMOUS) region; a POSIX
 * semaphore in that same region signals "readings are ready" and a second one
 * acknowledges "analysis finished".  No byte is copied by the kernel: the
 * analyser reads the producer's data in place, which is why the semaphore is
 * the only thing providing ordering.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q4_shm q4_shm.c
 * Run   : ./q4_shm
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_READINGS 1024

typedef struct {
    sem_t ready;                    /* producer -> consumer: data is present */
    sem_t done;                     /* consumer -> producer: result is ready  */
    int   count;                    /* number of readings written            */
    int   data[MAX_READINGS];       /* the shared payload (no copy)          */
    double average;
    int   maximum;
} Shared;

int main(void)
{
    Shared *sh = mmap(NULL, sizeof *sh, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) { perror("mmap"); return 1; }

    /* pshared = 1: the semaphores must work across processes */
    if (sem_init(&sh->ready, 1, 0) != 0 || sem_init(&sh->done, 1, 0) != 0) {
        perror("sem_init");
        return 1;
    }
    sh->count = 0;
    sh->average = 0.0;
    sh->maximum = 0;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        /* ---------------- analyser: waits, then reads in place ---------------- */
        sem_wait(&sh->ready);                    /* ordering, not copying */
        long sum = 0;
        int max = 0;
        for (int i = 0; i < sh->count; i++) {
            sum += sh->data[i];
            if (i == 0 || sh->data[i] > max) max = sh->data[i];
        }
        sh->average = sh->count ? (double)sum / sh->count : 0.0;
        sh->maximum = max;
        printf("analyser (shared memory): %d readings, average %.1f, max %d\n",
               sh->count, sh->average, sh->maximum);
        fflush(stdout);
        sem_post(&sh->done);                     /* tell the producer we are done */
        _exit(0);
    }

    /* ---------------- sensor reader: writes directly into shared memory ------ */
    int readings[] = {31, 33, 36, 34, 41, 38};
    for (int i = 0; i < 6; i++) sh->data[i] = readings[i];
    sh->count = 6;
    sem_post(&sh->ready);
    sem_wait(&sh->done);                         /* wait before unmapping */

    waitpid(pid, NULL, 0);
    printf("reader (shared memory): done, analyser wrote average %.1f back\n",
           sh->average);

    sem_destroy(&sh->ready);
    sem_destroy(&sh->done);
    munmap(sh, sizeof *sh);
    return 0;
}
