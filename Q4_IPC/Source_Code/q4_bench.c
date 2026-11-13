/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q4 growth investigation
 * Transfers 1 MB, 10 MB and 100 MB through (a) a pipe and (b) shared memory,
 * three repetitions each, and reports the best wall-clock time and the
 * effective throughput in MB/s.  The pipe path copies every byte into the
 * kernel buffer and out again; the shared-memory path lets the consumer read
 * the producer's pages directly, which is why it is faster and why it needs
 * an explicit semaphore for ordering.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q4_bench q4_bench.c
 * Run   : ./q4_bench
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define CHUNK (64 * 1024)
#define REPS  3

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

/* ------------------------------- pipe ---------------------------------- */
static double pipe_transfer(size_t bytes)
{
    int fd[2];
    if (pipe(fd) != 0) { perror("pipe"); exit(1); }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0) {
        close(fd[1]);
        char *buf = malloc(CHUNK);
        if (buf == NULL) _exit(2);
        size_t total = 0;
        ssize_t n;
        unsigned long sum = 0;
        while ((n = read(fd[0], buf, CHUNK)) > 0) {
            total += (size_t)n;
            for (ssize_t i = 0; i < n; i++)
                sum += (unsigned char)buf[i];   /* same work as the shm reader */
        }
        close(fd[0]);
        free(buf);
        _exit((total == bytes && sum != 0) ? 0 : 1);
    }

    close(fd[0]);
    char *buf = malloc(CHUNK);
    if (buf == NULL) { perror("malloc"); exit(1); }
    memset(buf, 0xAB, CHUNK);

    double t0 = now_ms();
    size_t sent = 0;
    while (sent < bytes) {
        size_t n = (bytes - sent < CHUNK) ? bytes - sent : CHUNK;
        ssize_t w = write(fd[1], buf, n);
        if (w <= 0) break;
        sent += (size_t)w;
    }
    close(fd[1]);                       /* EOF for the reader */

    int status = 0;
    waitpid(pid, &status, 0);
    double ms = now_ms() - t0;
    free(buf);
    return ms;
}

/* --------------------------- shared memory ------------------------------ */
typedef struct {
    sem_t filled;
    sem_t drained;
} Ctrl;

static double shm_transfer(size_t bytes)
{
    char *region = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) { perror("mmap"); exit(1); }
    memset(region, 0x11, bytes);        /* pre-fault: a real pipeline reuses
                                         * the same buffer, so first-touch page
                                         * faults must not be timed here */

    Ctrl *ctrl = mmap(NULL, sizeof *ctrl, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ctrl == MAP_FAILED) { perror("mmap"); exit(1); }
    sem_init(&ctrl->filled, 1, 0);
    sem_init(&ctrl->drained, 1, 0);

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0) {
        sem_wait(&ctrl->filled);
        unsigned long sum = 0;
        const unsigned char *p = (const unsigned char *)region;
        for (size_t i = 0; i < bytes; i++) sum += p[i];   /* reads every byte */
        sem_post(&ctrl->drained);
        _exit(sum != 0 ? 0 : 3);        /* touch every page read */
    }

    double t0 = now_ms();
    memset(region, 0xAB, bytes);            /* producer writes in place */
    sem_post(&ctrl->filled);
    sem_wait(&ctrl->drained);               /* consumer read in place  */
    double ms = now_ms() - t0;

    waitpid(pid, NULL, 0);
    sem_destroy(&ctrl->filled);
    sem_destroy(&ctrl->drained);
    munmap(ctrl, sizeof *ctrl);
    munmap(region, bytes);
    return ms;
}

int main(void)
{
    const size_t sizes[] = {1u << 20, 10u << 20, 100u << 20};

    printf("== Q4: transfer time, pipe vs shared memory ==\n");
    printf("chunk size %d KiB, best of %d runs\n\n", CHUNK / 1024, REPS);
    printf("%-9s %8s %12s %10s %12s %10s %9s\n",
           "size", "MB", "pipe (ms)", "pipe MB/s", "shm (ms)", "shm MB/s", "shm/pipe");
    printf("----------------------------------------------------------------------------\n");

    for (unsigned k = 0; k < sizeof sizes / sizeof sizes[0]; k++) {
        size_t bytes = sizes[k];
        double best_pipe = 1e18, best_shm = 1e18;

        for (int r = 0; r < REPS; r++) {
            double p = pipe_transfer(bytes);
            if (p < best_pipe) best_pipe = p;
            double s = shm_transfer(bytes);
            if (s < best_shm) best_shm = s;
        }

        double mb = (double)bytes / (1024.0 * 1024.0);
        printf("%-9u %8.1f %12.2f %10.1f %12.2f %10.1f %9.2f\n",
               (unsigned)(bytes >> 20), mb, best_pipe, mb / (best_pipe / 1000.0),
               best_shm, mb / (best_shm / 1000.0), best_pipe / best_shm);
    }

    printf("\npipe   = write() into the kernel buffer + read() out of it (2 copies)\n");
    printf("shm    = one mmap region, both processes touch the same pages (0 copies)\n");
    return 0;
}
