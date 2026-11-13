/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q3
 * Traffic-camera number-plate reader: 64 cameras processed (a) sequentially,
 * (b) with one POSIX thread per camera and (c) with a fixed pool of 4, 8 and
 * 16 worker threads. Wall time, speed-up and parallel efficiency are printed,
 * and every mode must produce the same checksum (correctness evidence).
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q3_camera q3_camera.c
 * Run   : ./q3_camera [frames_per_camera] [work_units]
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CAMERAS 64
#define MAXFRAMES 512
#define MAXTHREADS 64

static int g_frames = 40;
static int g_units = 150000;

static unsigned long results[CAMERAS];
static atomic_int next_task;
static _Atomic unsigned long pool_sum;

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

/* Deterministic stand-in for the per-frame image work (plate detection). */
static unsigned long frame_work(unsigned long seed, int units)
{
    unsigned long x = seed * 0x9E3779B97F4A7C15UL + 1UL;
    for (int i = 0; i < units; i++) {
        x = x * 1099511628211UL + 0x2545F4914F6CDD1DUL;
        x ^= x >> 29;
    }
    return x;
}

static unsigned long camera_sum(int cam)
{
    unsigned long s = 0;
    for (int f = 0; f < g_frames; f++)
        s ^= frame_work((unsigned long)cam * 4096UL + (unsigned long)f, g_units);
    return s;
}

/* ------------------------- one thread per camera ----------------------- */
typedef struct { int cam; } CamArg;

static void *camera_thread(void *arg)
{
    CamArg *a = (CamArg *)arg;
    results[a->cam] = camera_sum(a->cam);
    return NULL;
}

/* ------------------------------ thread pool ---------------------------- */
static void *pool_worker(void *arg)
{
    (void)arg;
    unsigned long local = 0;
    for (;;) {
        int task = atomic_fetch_add(&next_task, 1);
        if (task >= CAMERAS * g_frames) break;
        int cam = task / g_frames;
        int f = task % g_frames;
        local ^= frame_work((unsigned long)cam * 4096UL + (unsigned long)f, g_units);
    }
    atomic_fetch_xor(&pool_sum, local);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc > 1) g_frames = atoi(argv[1]);
    if (argc > 2) g_units = atoi(argv[2]);
    if (g_frames < 1 || g_frames > MAXFRAMES) g_frames = 40;
    if (g_units < 1) g_units = 150000;

    printf("== Q3: number-plate reader, %d cameras x %d frames (work=%d units/frame) ==\n\n",
           CAMERAS, g_frames, g_units);
    printf("%-22s %8s %10s %9s %11s\n",
           "mode", "threads", "time (ms)", "speed-up", "efficiency");
    printf("--------------------------------------------------------------\n");

    /* ---------------- sequential baseline ---------------- */
    double t0 = now_ms();
    for (int c = 0; c < CAMERAS; c++) results[c] = camera_sum(c);
    double seq_ms = now_ms() - t0;
    unsigned long ref = 0;
    for (int c = 0; c < CAMERAS; c++) ref ^= results[c];
    printf("%-22s %8d %10.1f %9s %11s\n", "sequential", 1, seq_ms, "1.00", "1.00");

    /* ---------------- one thread per camera ---------------- */
    pthread_t th[MAXTHREADS];
    CamArg args[MAXTHREADS];

    t0 = now_ms();
    for (int c = 0; c < CAMERAS; c++) {
        args[c].cam = c;
        if (pthread_create(&th[c], NULL, camera_thread, &args[c]) != 0) {
            perror("pthread_create"); return 1;
        }
    }
    for (int c = 0; c < CAMERAS; c++) pthread_join(th[c], NULL);
    double percam_ms = now_ms() - t0;

    unsigned long s2 = 0;
    for (int c = 0; c < CAMERAS; c++) s2 ^= results[c];
    printf("%-22s %8d %10.1f %9.2f %11.2f  %s\n", "thread per camera", CAMERAS,
           percam_ms, seq_ms / percam_ms, (seq_ms / percam_ms) / CAMERAS,
           s2 == ref ? "(checksum OK)" : "(CHECKSUM MISMATCH)");

    /* ---------------- fixed pools of 4, 8 and 16 ---------------- */
    const int sizes[] = {4, 8, 16};
    for (unsigned k = 0; k < sizeof sizes / sizeof sizes[0]; k++) {
        int n = sizes[k];
        atomic_store(&next_task, 0);
        atomic_store(&pool_sum, 0);

        t0 = now_ms();
        for (int i = 0; i < n; i++)
            if (pthread_create(&th[i], NULL, pool_worker, NULL) != 0) {
                perror("pthread_create"); return 1;
            }
        for (int i = 0; i < n; i++) pthread_join(th[i], NULL);
        double pool_ms = now_ms() - t0;

        unsigned long s = atomic_load(&pool_sum);
        printf("%-22s %8d %10.1f %9.2f %11.2f  %s\n", "thread pool", n, pool_ms,
               seq_ms / pool_ms, (seq_ms / pool_ms) / n,
               s == ref ? "(checksum OK)" : "(CHECKSUM MISMATCH)");
    }

    printf("\nchecksum of the sequential run: 0x%016lx\n", ref);
    printf("frames processed: %d, total frame work units: %d\n",
           CAMERAS * g_frames, CAMERAS * g_frames * g_units);
    return 0;
}
