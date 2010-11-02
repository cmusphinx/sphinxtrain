/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * profile.c -- For timing and event counting.
 *
 * HISTORY
 * 
 * 01-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed timer_ names to cyctimer_ (for cycle counter).
 * 		Added timing_ structures and functions using system calls for timing.
 * 
 * 13-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ifdefs around cyctimer_print_all and cyctimer_print_all_norm.
 * 
 * 27-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Sphinx-II version.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (! WIN32)
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <windows.h>
#include <time.h>
#endif

#include <s3/profile.h>
#include <s3/err.h>
#include <sphinxbase/ckd_alloc.h>


typedef struct {
    char *name;
    uint32 count;
} ctr_t;
static ctr_t *ctr = NULL;
static int32 n_ctr = 0;

#define MAX_CTR			30


int32 counter_new (char *name)
{
    if (! ctr)
	ctr = (ctr_t *) ckd_calloc (MAX_CTR, sizeof(ctr_t));
    if (n_ctr >= MAX_CTR) {
	E_WARN("#counters (%d) exceeded\n", MAX_CTR);
	return -1;
    }
    ctr[n_ctr].name = (char *) ckd_salloc (name);
    ctr[n_ctr].count = 0;
    
    return (n_ctr++);
}


void counter_increment (int32 id, int32 inc)
{
    if ((id < 0) || (id >= MAX_CTR))
	return;
    
    ctr[id].count += inc;
}


void counter_reset (int32 id)
{
    if ((id < 0) || (id >= MAX_CTR))
	return;
    
    ctr[id].count = 0;
}


void counter_reset_all ( void )
{
    int32 i;
    
    for (i = 0; i < n_ctr; i++)
	counter_reset (i);
}

void counter_print_all (FILE *fp)
{
    int32 i;
    
    if (n_ctr > 0) {
	fprintf (fp, "CTR:");
	for (i = 0; i < n_ctr; i++)
	    fprintf (fp, "[%s %10d]", ctr[i].name, ctr[i].count);
	fprintf (fp, "\n");
    }
}

#if (WIN32)

#define TM_LOWSCALE	1e-7
#define TM_HIGHSCALE	(4294967296.0 * TM_LOWSCALE);

static float64 make_sec (FILETIME *tm)
{
    float64 dt;
    
    dt = tm->dwLowDateTime * TM_LOWSCALE;
    dt += tm->dwHighDateTime * TM_HIGHSCALE;

    return (dt);
}

#else /* !WIN32 */

static float64 make_sec (struct timeval *s)
{
    return (s->tv_sec + s->tv_usec * 0.000001);
}

#endif

#define MAX_NAMED_TMR	10

static timing_t *tmr_list[MAX_NAMED_TMR];
static char *tmr_name[MAX_NAMED_TMR];
static int n_tmr = 0;

void timing_bind_name(const char *name, timing_t *tmr)
{
    int i;

    if (n_tmr == MAX_NAMED_TMR) {
	E_ERROR("No more named timers; increase MAX_NAMED_TMR\n");
	return;
    }

    if (n_tmr > 0) {
	for (i = 0; i < n_tmr; i++) {
	    if (strcmp(name, tmr_name[i]) == 0) {
		E_ERROR("Timer %s already defined\n", name);

		return;
	    }
	}
    }

    tmr_list[n_tmr] = tmr;
    tmr_name[n_tmr] = strdup(name);

    ++n_tmr;
}

timing_t *
timing_get(const char *name)
{
    int i;
    timing_t *ret = NULL;
    
    if (n_tmr > 0) {
	for (i = 0; i < n_tmr; i++) {
	    if (strcmp(name, tmr_name[i]) == 0) {
		ret = tmr_list[i];
		break;
	    }
	}
    }

    return ret;
}


/*
 * Obtain and initialize a timing module
 */
timing_t *timing_new ( void )
{
    timing_t *tm;
    
    tm = (timing_t *) ckd_calloc (1, sizeof(timing_t));
    tm->t_elapsed = 0.0;
    tm->t_cpu = 0.0;

    return tm;
}


void timing_start (timing_t *tm)
{
#if (! WIN32)
    struct timeval e_start;	/* Elapsed time */
    
#if (! _HPUX_SOURCE)
    struct rusage start;	/* CPU time */
    
    /* Unix but not HPUX */
    getrusage (RUSAGE_SELF, &start);
    tm->start_cpu = make_sec (&start.ru_utime) + make_sec (&start.ru_stime);
#endif
    /* Unix + HP */
    gettimeofday (&e_start, 0);
    tm->start_elapsed = make_sec (&e_start);
#else
    HANDLE pid;
    FILETIME t_create, t_exit, kst, ust;
    
    /* PC */
    pid = GetCurrentProcess();
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
    tm->start_cpu = make_sec (&ust) + make_sec (&kst);

    tm->start_elapsed = (float64)clock() / CLOCKS_PER_SEC;
#endif
}


void timing_stop (timing_t *tm)
{
    float64 dt_cpu, dt_elapsed;
    
#if (! WIN32)
    struct timeval e_stop;	/* Elapsed time */
    
#if (! _HPUX_SOURCE)
    struct rusage stop;		/* CPU time */
    
    /* Unix but not HPUX */
    getrusage (RUSAGE_SELF, &stop);
    dt_cpu = make_sec (&stop.ru_utime) + make_sec (&stop.ru_stime) - tm->start_cpu;
#else
    dt_cpu = 0.0;
#endif
    /* Unix + HP */
    gettimeofday (&e_stop, 0);
    dt_elapsed = (make_sec (&e_stop) - tm->start_elapsed);
#else
    HANDLE pid;
    FILETIME t_create, t_exit, kst, ust;
    
    /* PC */
    pid = GetCurrentProcess();
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
    dt_cpu = make_sec (&ust) + make_sec (&kst) - tm->start_cpu;
    dt_elapsed = ((float64)clock() / CLOCKS_PER_SEC) - tm->start_elapsed;
#endif

    tm->t_cpu += dt_cpu;
    tm->t_elapsed += dt_elapsed;

    tm->t_tot_cpu += dt_cpu;
    tm->t_tot_elapsed += dt_elapsed;
}


void timing_reset (timing_t *tm)
{
    tm->t_cpu = 0.0;
    tm->t_elapsed = 0.0;
}
