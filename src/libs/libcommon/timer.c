/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: timer.c
 * 
 * Description: 
 *	Elapsed cpu and wall clock time functions
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/timer.h>
#include <s3/get_time.h>
#include <s3/s3.h>

#include <assert.h>
#include <stdio.h>

typedef struct {
    uint32 sec;
    float32 frac_sec;
} stime_t;

#define MAX_TIMERS	10

static stime_t timer[MAX_TIMERS];

int
timer_start(uint32 id)
{
    assert(id < MAX_TIMERS);

    get_time(&timer[id].sec, &timer[id].frac_sec);

    return S3_SUCCESS;
}

float32 timer_stop(uint32 id)
{
    uint32 stop_sec;
    float32 stop_frac_sec;
    float32 elapsed;

    get_time(&stop_sec, &stop_frac_sec);

    elapsed = stop_sec - timer[id].sec;
    elapsed += stop_frac_sec - timer[id].frac_sec;

    return elapsed;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.1  1995/10/05  13:00:41  eht
 * Initial revision
 *
 *
 */
