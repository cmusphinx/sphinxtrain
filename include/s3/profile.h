/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * profile.h -- For timing and event counting.
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
