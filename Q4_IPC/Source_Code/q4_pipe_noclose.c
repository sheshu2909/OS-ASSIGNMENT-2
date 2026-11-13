/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q4 (failure experiment)
 * Identical to q4_pipe.c EXCEPT that the parent keeps its copy of fd[1] open.
 * The child therefore never sees end-of-file and blocks in read() for ever.
 * Intended to be run under a timeout so the block itself becomes evidence:
 *
 *   timeout 5 ./q4_pipe_noclose ; echo "exit status = $?"    # 124 = timed out
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q4_pipe_noclose q4_pipe_noclose.c
 * ==========================================================================*/
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid == 0) {
        close(fd[1]);
        int count = 0, max = 0, value;
        printf("analyser: waiting for readings (it will never see EOF)\n");
        fflush(stdout);
        while (read(fd[0], &value, sizeof value) == (int)sizeof value) {
            count++;
            if (count == 1 || value > max) max = value;
        }
        (void)max;
        printf("analyser: THIS LINE IS NEVER PRINTED\n");
        _exit(0);
    }

    close(fd[0]);
    int readings[] = {31, 33, 36, 34, 41, 38};
    for (int i = 0; i < 6; i++)
        if (write(fd[1], &readings[i], sizeof readings[i]) < 0) perror("write");

    /* BUG: the parent deliberately does NOT close fd[1] here.
     * The pipe still has a live writer, so the child's read() stays blocked. */
    printf("reader: wrote 6 readings but kept the write end open\n");
    fflush(stdout);

    waitpid(pid, NULL, 0);                  /* never returns */
    printf("reader: done\n");               /* never reached */
    return 0;
}
