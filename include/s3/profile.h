/*
 * profile.h -- For timing and event counting.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 27-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Sphinx-II version.
 */


#ifndef _LIBUTIL_PROFILE_H_
#define _LIBUTIL_PROFILE_H_


#include "prim_type.h"
#include <stdio.h>


/* Event counter functions */
int32 counter_new (char *name);

void counter_increment (int32 id, int32 count);	/* Increment counter by count */

void counter_print_all (FILE *fp);
void counter_reset_all ( void );


/*
 * Cycle counter functions for fine grained performance evaluation.
 * (Only available on these machines: DEC-Alpha)
 */
int32 cyctimer_new (char *name);	/* Allocate timer with given name and return its id;
					   Timer is paused at this time */

void cyctimer_resume (int32 id);
void cyctimer_pause (int32 id);

void cyctimer_print_all (FILE *fp);
void cyctimer_reset_all ( void );
void cyctimer_print_all_norm (FILE *fp, float64 norm_sec, int32 norm_id);
float64 cyctimer_get_sec (int32 id);


/*
 * Timing structures and functions for coarse-grained performance measurements.
 * Using standard system calls.
 */
typedef struct {
    float64 t_cpu;		/* CPU time for most recent start-stop operation */
    float64 t_elapsed;		/* Elapsed time for most recent start-stop operation */
    float64 t_tot_cpu;		/* Accumulated CPU time */
    float64 t_tot_elapsed;	/* Accumulated elapsed time */
    float64 start_cpu;		/* FOR INTERNAL USE ONLY */
    float64 start_elapsed;	/* FOR INTERNAL USE ONLY */
} timing_t;


timing_t *timing_new ( void );	/* Obtain and initialize a new timing module */

/* Start timing */
void timing_start (timing_t *tm);

/* Stop timing and accumulate t_cpu, t_elapsed, t_tot_cpu, t_tot_elapsed */
void timing_stop (timing_t *tm);

/* Reset t_cpu, t_elapsed to 0.0 */
void timing_reset (timing_t *tm);

/* Register a timing_t structure under a name */
void timing_bind_name(const char *name, timing_t *tmr);

/* Given a name, return the timing_t structure
   associated with it.  Returns NULL if no such structure */
timing_t *timing_get(const char *name);


#endif
