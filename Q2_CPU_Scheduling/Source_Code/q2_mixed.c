/* ============================================================================
 * CSA0406 - Operating Systems - Assignment 2 - Q2 (design evidence)
 *
 * Part A - mixed workload of the video service: 4 long "film" jobs (1200 s)
 *          and 8 short "thumbnail" jobs (2 s). The same set is scheduled by
 *          FCFS, SJF, SRTF, RR(q=3) and a 4-level MLFQ; the program reports
 *          average waiting/turnaround time separately for the short and the
 *          long class, which is exactly what the customers complain about.
 *
 * Part B - starvation and ageing: one long low-priority job competes with a
 *          continuous stream of short high-priority jobs. Strict priority
 *          starves the long job; ageing (priority improved while waiting)
 *          bounds its waiting time.
 *
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o q2_mixed q2_mixed.c
 * Run   : ./q2_mixed
 * ==========================================================================*/
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define MAXJ 64
#define FILM 1200
#define THUMB 2
#define QCAP 256            /* ring-buffer capacity per priority level */

typedef struct {
    int id, arr, burst, rem, cls, comp, first, started;
} Job;

typedef struct { int n; Job j[MAXJ]; } Work;

static void build_mixed(Work *w)
{
    int arr_short[8] = {5, 10, 15, 25, 35, 45, 55, 65};
    int arr_long[4] = {0, 30, 60, 90};
    int k = 0;

    w->n = 12;
    for (int i = 0; i < 4; i++, k++) {
        w->j[k].id = k; w->j[k].arr = arr_long[i]; w->j[k].burst = FILM;
        w->j[k].rem = FILM; w->j[k].cls = 1; w->j[k].comp = -1;
        w->j[k].first = -1; w->j[k].started = 0;
    }
    for (int i = 0; i < 8; i++, k++) {
        w->j[k].id = k; w->j[k].arr = arr_short[i]; w->j[k].burst = THUMB;
        w->j[k].rem = THUMB; w->j[k].cls = 0; w->j[k].comp = -1;
        w->j[k].first = -1; w->j[k].started = 0;
    }
}

static void print_class_row(const char *label, const Work *w, int slices)
{
    double sw[2] = {0, 0}, st[2] = {0, 0};
    int cn[2] = {0, 0};

    for (int i = 0; i < w->n; i++) {
        int c = w->j[i].cls;
        if (w->j[i].comp < 0) continue;
        int tat = w->j[i].comp - w->j[i].arr;
        sw[c] += tat - w->j[i].burst;
        st[c] += tat;
        cn[c]++;
    }
    printf("%-26s %9.2f %9.2f %9.2f %9.2f %8d\n",
           label,
           cn[0] ? sw[0] / cn[0] : 0.0, cn[0] ? st[0] / cn[0] : 0.0,
           cn[1] ? sw[1] / cn[1] : 0.0, cn[1] ? st[1] / cn[1] : 0.0,
           slices);
}

/* ---- FCFS / SJF / SRTF: unit-step simulation, policy chosen by mode ---- */
static void run_basic(Work *w, int mode)
{
    int t = 0, slices = 0, done = 0, current = -1;

    while (done < w->n) {
        int pick = -1;

        if (mode == 0) {                      /* FCFS: queue order by arrival */
            if (current >= 0 && w->j[current].rem > 0) {
                pick = current;
            } else {
                for (int i = 0; i < w->n; i++)
                    if (w->j[i].rem > 0 && w->j[i].arr <= t &&
                        (pick < 0 || w->j[i].arr < w->j[pick].arr))
                        pick = i;
            }
        } else if (mode == 1) {               /* SJF: non-preemptive */
            if (current >= 0 && w->j[current].rem > 0) {
                pick = current;
            } else {
                for (int i = 0; i < w->n; i++) {
                    if (w->j[i].rem <= 0 || w->j[i].arr > t) continue;
                    if (pick < 0 || w->j[i].burst < w->j[pick].burst ||
                        (w->j[i].burst == w->j[pick].burst &&
                         w->j[i].arr < w->j[pick].arr))
                        pick = i;
                }
            }
        } else {                              /* SRTF: preemptive */
            for (int i = 0; i < w->n; i++) {
                if (w->j[i].rem <= 0 || w->j[i].arr > t) continue;
                if (pick < 0 || w->j[i].rem < w->j[pick].rem ||
                    (w->j[i].rem == w->j[pick].rem &&
                     w->j[i].arr < w->j[pick].arr))
                    pick = i;
            }
        }

        if (pick < 0) { t++; continue; }

        if (pick != current) slices++;
        current = pick;
        if (w->j[pick].first < 0) w->j[pick].first = t;
        w->j[pick].started = 1;
        w->j[pick].rem--;
        t++;
        if (w->j[pick].rem == 0) { w->j[pick].comp = t; done++; }
    }
    print_class_row(mode == 0 ? "FCFS" : (mode == 1 ? "SJF (non-preemptive)" : "SRTF"),
                    w, slices);
}

/* --------------------------- ring queue helpers ------------------------- */
static int  qbuf[4][QCAP];
static int  qhead[4], qtail[4], qcnt[4];

static void q_reset(void)
{
    memset(qhead, 0, sizeof qhead);
    memset(qtail, 0, sizeof qtail);
    memset(qcnt, 0, sizeof qcnt);
}

static void q_push(int level, int job)
{
    if (qcnt[level] >= QCAP) return;            /* cannot happen: 12 jobs max */
    qbuf[level][qtail[level]] = job;
    qtail[level] = (qtail[level] + 1) % QCAP;
    qcnt[level]++;
}

static int q_pop(int level)
{
    int job = qbuf[level][qhead[level]];
    qhead[level] = (qhead[level] + 1) % QCAP;
    qcnt[level]--;
    return job;
}

static int q_top(void)
{
    for (int l = 0; l < 4; l++) if (qcnt[l] > 0) return l;
    return -1;
}

/* --------------------------------- RR ---------------------------------- */
static void run_rr(Work *w, int quantum)
{
    int t = 0, slices = 0, done = 0, added[MAXJ] = {0};
    char label[40];

    q_reset();
    for (int i = 0; i < w->n; i++)
        if (w->j[i].arr == 0) { added[i] = 1; q_push(0, i); }

    while (done < w->n) {
        if (qcnt[0] == 0) {                     /* CPU idle */
            t++;
            for (int i = 0; i < w->n; i++)
                if (!added[i] && w->j[i].arr <= t) { added[i] = 1; q_push(0, i); }
            continue;
        }
        int p = q_pop(0);
        if (w->j[p].first < 0) w->j[p].first = t;
        int run = (w->j[p].rem < quantum) ? w->j[p].rem : quantum;
        t += run;
        w->j[p].rem -= run;
        slices++;

        for (int i = 0; i < w->n; i++)
            if (!added[i] && w->j[i].arr <= t) { added[i] = 1; q_push(0, i); }

        if (w->j[p].rem > 0) q_push(0, p);
        else { w->j[p].comp = t; done++; }
    }
    snprintf(label, sizeof label, "Round Robin (q=%d)", quantum);
    print_class_row(label, w, slices);
}

/* -------------------------------- MLFQ ---------------------------------
 * Four levels with quanta 2 / 16 / 64 / unlimited. A new job enters level 0;
 * a job that uses up its quantum is demoted one level; a job that blocks or
 * is preempted keeps its level. Every 1000 time units all jobs are promoted
 * back to level 0 (ageing, so no job starves).
 * ---------------------------------------------------------------------- */
static void run_mlfq(Work *w)
{
    static const int quanta[4] = {2, 16, 64, INT_MAX};
    const int boost_period = 1000;
    int budget[MAXJ], level[MAXJ], added[MAXJ] = {0};
    int t = 0, done = 0, slices = 0, last_boost = 0;

    q_reset();
    for (int i = 0; i < w->n; i++) { level[i] = 0; budget[i] = quanta[0]; }
    for (int i = 0; i < w->n; i++)
        if (w->j[i].arr == 0) { added[i] = 1; q_push(0, i); }

    while (done < w->n) {
        /* ageing: promote everything to the top queue */
        if (t - last_boost >= boost_period) {
            last_boost = t;
            for (int l = 1; l < 4; l++) {
                int n = qcnt[l];
                for (int i = 0; i < n; i++) {
                    int j = q_pop(l);
                    if (w->j[j].rem > 0) {
                        level[j] = 0;
                        budget[j] = quanta[0];
                        q_push(0, j);
                    }
                }
            }
        }

        int lv = q_top();
        if (lv < 0) {                            /* CPU idle */
            t++;
            for (int i = 0; i < w->n; i++)
                if (!added[i] && w->j[i].arr <= t) {
                    added[i] = 1; level[i] = 0; budget[i] = quanta[0]; q_push(0, i);
                }
            continue;
        }

        int p = q_pop(lv);
        if (w->j[p].rem <= 0) continue;
        if (w->j[p].first < 0) w->j[p].first = t;

        /* one slice: min(remaining, quantum), cut short by the next arrival
         * because a new job enters level 0 and preempts lower levels */
        int run = (w->j[p].rem < budget[p]) ? w->j[p].rem : budget[p];
        int next_arr = INT_MAX;
        for (int i = 0; i < w->n; i++)
            if (!added[i] && w->j[i].arr > t && w->j[i].arr < next_arr)
                next_arr = w->j[i].arr;
        if (next_arr != INT_MAX && next_arr - t < run) run = next_arr - t;

        t += run;
        w->j[p].rem -= run;
        if (budget[p] != INT_MAX) budget[p] -= run;
        slices++;                                /* one dispatch = one switch */

        for (int i = 0; i < w->n; i++)
            if (!added[i] && w->j[i].arr <= t) {
                added[i] = 1; level[i] = 0; budget[i] = quanta[0]; q_push(0, i);
            }

        if (w->j[p].rem == 0) { w->j[p].comp = t; done++; continue; }

        if (budget[p] <= 0) {                    /* used its quantum: demote */
            if (level[p] < 3) level[p]++;
            budget[p] = quanta[level[p]];
            q_push(level[p], p);
        } else {
            q_push(lv, p);                       /* preempted, keeps its level */
        }
    }
    print_class_row("MLFQ (q=2/16/64, boost 1000)", w, slices);
}

/* ---------------------- Part B: starvation / ageing -------------------- */
/* One long job (priority 5, 60 units) competes with a NEW one-unit job that
 * arrives at every time unit and has priority 1. Without ageing the long job
 * can never win the CPU; with ageing its effective priority improves every 50
 * units of waiting, so it eventually runs and its completion time is bounded. */
static void starvation_demo(void)
{
    enum { NT = 5000, HORIZON = 2000 };
    static int arrivals[NT + 1];

    for (int ageing = 0; ageing <= 1; ageing++) {
        int rem_film = 60, comp_film = -1;
        int thumb_done = 0, qh = 0, qt = 0;
        double thumb_wait = 0;
        int t = 0;

        while (comp_film < 0 && t < HORIZON) {
            if (t >= 1 && t <= NT) arrivals[qt++] = t;      /* a new short job */

            int film_prio = 5 - (ageing ? t / 50 : 0);
            if (film_prio < 0) film_prio = 0;

            int have_thumb = (qh < qt);
            int run_film;
            if (rem_film <= 0)          run_film = 0;
            else if (!have_thumb)       run_film = 1;
            else                        run_film = (film_prio < 1);  /* strict */

            if (run_film) {
                rem_film--;
                t++;
                if (rem_film == 0) comp_film = t;
            } else if (have_thumb) {
                int arrived = arrivals[qh++];
                thumb_wait += (double)(t - arrived);
                thumb_done++;
                t++;
            } else {
                break;
            }
        }
        if (comp_film >= 0)
            printf("%-20s film completes at t=%-5d thumbnails served=%d avg thumb wait=%.1f\n",
                   ageing ? "priority + ageing" : "strict priority", comp_film,
                   thumb_done, thumb_done ? thumb_wait / thumb_done : 0.0);
        else
            printf("%-20s film NEVER completes within %d units (starved); thumbnails served=%d\n",
                   ageing ? "priority + ageing" : "strict priority", HORIZON, thumb_done);
    }
}

int main(void)
{
    printf("=== Q2-A: mixed workload of the video service ===\n");
    printf("4 film jobs of %d s (arrivals 0,30,60,90) and 8 thumbnail jobs of %d s\n",
           FILM, THUMB);
    printf("(arrivals 5,10,15,25,35,45,55,65). Times are in seconds.\n\n");
    printf("%-26s %9s %9s %9s %9s %8s\n",
           "policy", "WT short", "TAT short", "WT long", "TAT long", "switches");
    printf("--------------------------------------------------------------------------\n");

    Work w;
    build_mixed(&w); run_basic(&w, 0);
    build_mixed(&w); run_basic(&w, 1);
    build_mixed(&w); run_basic(&w, 2);
    build_mixed(&w); run_rr(&w, 3);
    build_mixed(&w); run_mlfq(&w);

    printf("\n=== Q2-B: starvation and ageing ===\n");
    printf("One 60 s job at priority 5 competes with a stream of one-second jobs\n");
    printf("at priority 1 arriving at every t = 1,2,3,... (endless supply).\n\n");
    starvation_demo();

    printf("\nInterpretation: without ageing the long job is pushed back by every\n");
    printf("new short job; ageing raises its effective priority while it waits,\n");
    printf("so its completion time becomes bounded.\n");
    return 0;
}
