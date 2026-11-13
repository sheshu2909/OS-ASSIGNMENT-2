/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q1 (evaluation evidence)
 * Measures the three costs that decide "process per student" vs "thread per
 * student":  (1) fork()+exit+wait   (2) pthread_create()+join
 *             (3) pipe round trip (two context switches + two copies)
 *             (4) resident memory consumed by 256 live thread stacks
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q1_overhead q1_overhead.c
 * Run   : ./q1_overhead [iterations]           (default 200)
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define NTHREAD_RSS 256
#define STACK_TOUCH  (64 * 1024)

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void *noop(void *arg) { return arg; }

static long rss_kb(void)
{
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];
    long kb = -1;
    if (f == NULL) return -1;
    while (fgets(line, sizeof line, f) != NULL)
        if (strncmp(line, "VmRSS:", 6) == 0) sscanf(line + 6, "%ld", &kb);
    fclose(f);
    return kb;
}

/* ---- one live thread: touch part of its stack, then stay alive briefly ---- */
static void nap_ms(long ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void *stack_toucher(void *arg)
{
    volatile unsigned char block[STACK_TOUCH];
    memset((void *)block, (int)(size_t)arg, sizeof block);
    nap_ms(800);             /* keep the stack resident while RSS is sampled */
    return NULL;
}

int main(int argc, char **argv)
{
    int iters = (argc > 1) ? atoi(argv[1]) : 200;
    if (iters < 1) iters = 1;

    printf("== Q1: cost of a process vs a thread ==\n");

    /* ---------------- (1) process creation ---------------- */
    double t0 = now_ms();
    for (int i = 0; i < iters; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) _exit(0);
        int status;
        if (waitpid(pid, &status, 0) < 0) perror("waitpid");
    }
    double fork_us = (now_ms() - t0) * 1000.0 / iters;

    /* ---------------- (2) thread creation ---------------- */
    t0 = now_ms();
    for (int i = 0; i < iters; i++) {
        pthread_t th;
        if (pthread_create(&th, NULL, noop, NULL) != 0) { perror("pthread_create"); return 1; }
        pthread_join(th, NULL);
    }
    double thread_us = (now_ms() - t0) * 1000.0 / iters;

    /* ---------------- (3) pipe round trip ---------------- */
    int up[2], down[2];
    if (pipe(up) != 0 || pipe(down) != 0) { perror("pipe"); return 1; }
    pid_t echo = fork();
    if (echo < 0) { perror("fork"); return 1; }
    if (echo == 0) {
        char b;
        close(up[1]);
        close(down[0]);
        while (read(up[0], &b, 1) == 1)
            if (write(down[1], &b, 1) != 1) _exit(1);
        _exit(0);
    }
    close(up[0]);
    close(down[1]);
    const int rtt_iters = 2000;
    t0 = now_ms();
    for (int i = 0; i < rtt_iters; i++) {
        char b = 'x';
        if (write(up[1], &b, 1) != 1) { perror("write"); break; }
        if (read(down[0], &b, 1) != 1) { perror("read"); break; }
    }
    double rtt_us = (now_ms() - t0) * 1000.0 / rtt_iters;
    close(up[1]);
    waitpid(echo, NULL, 0);

    /* ---------------- (4) resident memory of live thread stacks ---------------- */
    long rss_before = rss_kb();
    pthread_t th[NTHREAD_RSS];
    int created = 0;
    for (int i = 0; i < NTHREAD_RSS; i++) {
        if (pthread_create(&th[i], NULL, stack_toucher, (void *)(size_t)i) == 0)
            created++;
        else
            break;
    }
    nap_ms(300);                    /* let every thread touch its stack */
    long rss_after = rss_kb();
    for (int i = 0; i < created; i++) pthread_join(th[i], NULL);

    printf("\n%-38s %12s\n", "measurement", "value");
    printf("%-38s %12.1f us\n", "fork() + _exit() + waitpid()", fork_us);
    printf("%-38s %12.1f us\n", "pthread_create() + pthread_join()", thread_us);
    printf("%-38s %12.1f us\n", "pipe round trip (2 switches, 2 copies)", rtt_us);
    if (rss_before > 0 && rss_after > 0) {
        long delta = rss_after - rss_before;
        printf("%-38s %12ld kB\n", "RSS growth for 256 live thread stacks", delta);
        printf("%-38s %12.1f kB\n", "  -> resident bytes per thread stack",
               created > 0 ? (double)delta / created : 0.0);
        printf("(each thread touched %d kB of its stack; created %d)\n",
               STACK_TOUCH / 1024, created);
    } else {
        printf("%-38s %12s\n", "RSS growth for 256 live thread stacks", "n/a");
    }
    printf("\nprocess/thread creation cost ratio: %.1f x\n", fork_us / thread_us);
    return 0;
}
