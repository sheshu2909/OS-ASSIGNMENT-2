/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q1
 * Processes and Process States: create children with fork(), report PCB-level
 * identity (pid, ppid, state, address-space size) and let the parent be
 * observed with ps while the children are alive.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q1_processes q1_processes.c
 * Run   : ./q1_processes [hold_seconds] [pid_file]
 *         ./q1_processes 6 q1_pids.txt      (then run ps on the pids)
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NCHILD 3

/* Print pid, ppid, scheduling state and address-space usage of this process.
 * /proc/self/status is the Linux interface; on non-Linux POSIX layers it may
 * be absent, in which case we degrade gracefully instead of failing. */
static void print_self_status(const char *who)
{
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];
    char state[16] = "n/a", vm_size[32] = "n/a", vm_rss[32] = "n/a";

    if (f == NULL) {
        printf("  %-11s pid=%-6ld ppid=%-6ld (no /proc/self/status here)\n",
               who, (long)getpid(), (long)getppid());
        return;
    }
    while (fgets(line, sizeof line, f) != NULL) {
        if (strncmp(line, "State:", 6) == 0)
            sscanf(line + 6, "%15s", state);
        else if (strncmp(line, "VmSize:", 7) == 0)
            sscanf(line + 7, "%31s", vm_size);
        else if (strncmp(line, "VmRSS:", 6) == 0)
            sscanf(line + 6, "%31s", vm_rss);
    }
    fclose(f);
    printf("  %-11s pid=%-6ld ppid=%-6ld state=%-4s VmSize=%s kB VmRSS=%s kB\n",
           who, (long)getpid(), (long)getppid(), state, vm_size, vm_rss);
}

/* A short CPU-bound phase so the child is genuinely in the RUNNING state. */
static void busy_work(void)
{
    volatile unsigned long x = 1;
    for (unsigned long i = 0; i < 2000000UL; i++)
        x = x * 2654435761UL + i;
    (void)x;
}

int main(int argc, char **argv)
{
    int hold = (argc > 1) ? atoi(argv[1]) : 5;
    const char *pidfile = (argc > 2) ? argv[2] : NULL;
    pid_t pids[NCHILD] = {0};

    printf("== Q1: fork() and the five process states ==\n");
    print_self_status("parent");
    fflush(stdout);

    /* ---- NEW -> READY -> RUNNING: fork() creates the PCB and admits it ---- */
    for (int i = 0; i < NCHILD; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            /* ---- child context ---- */
            char who[16];
            snprintf(who, sizeof who, "child %d", i + 1);
            print_self_status(who);
            printf("  child %d: RUNNING  (CPU-bound loop)\n", i + 1);
            fflush(stdout);

            busy_work();

            printf("  child %d: WAITING  (sleep %d s - state S in ps)\n", i + 1, hold);
            fflush(stdout);
            sleep((unsigned)hold);

            printf("  child %d: READY/RUNNING again, exiting with code %d (TERMINATED)\n",
                   i + 1, i + 1);
            fflush(stdout);
            _exit(i + 1);
        }
        pids[i] = pid;
    }

    /* ---- parent records the pids so the shell can run ps on them ---- */
    if (pidfile != NULL) {
        FILE *pf = fopen(pidfile, "w");
        if (pf != NULL) {
            for (int i = 0; i < NCHILD; i++)
                fprintf(pf, "%ld\n", (long)pids[i]);
            fclose(pf);
        }
    }

    printf("\nparent: forked %d children:", NCHILD);
    for (int i = 0; i < NCHILD; i++)
        printf(" %ld", (long)pids[i]);
    printf("\nparent: children are now sleeping - observe them with:\n");
    printf("  ps -fp <pids>        (Linux: ps -o pid,ppid,stat,comm -p <pids>)\n\n");
    fflush(stdout);

    /* ---- WAITING -> TERMINATED: parent reaps each child, no zombie remains ---- */
    for (int i = 0; i < NCHILD; i++) {
        int status = 0;
        pid_t w = waitpid(pids[i], &status, 0);
        if (w < 0) {
            perror("waitpid");
            continue;
        }
        if (WIFEXITED(status))
            printf("parent: reaped child %ld, exit status %d\n",
                   (long)w, WEXITSTATUS(status));
    }
    printf("parent: all children terminated and reaped (no zombies)\n");
    return 0;
}
