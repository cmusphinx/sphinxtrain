/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: live_norm.c
 * 
 * Description: 
 *   CMN for a live system.  This file defines the following public
 *   functions:
 *
 *   mean_norm_init(int32 vlen)
 *     Initialize subsystem to normalize feature vectors of
 *     length VLEN.
 *
 *   mean_norm_acc_sub(float *vec)
 *     Add coefficents to running total and subtract the current
 *     estimated mean from VEC.  Increments the number of input
 *     frames.
 *
 *   mean_norm_update()
 *     Computes a new mean from the accumulated coefficient totals and
 *     the number of input frames seen since the last update.
 *     The new mean is averaged with the prior value for the mean to
 *     obtain the estimated mean for the next utterance.
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$";*/

#include <s3/live_norm.h>
#include <s3/prim_type.h>

#include <stdio.h>
#include <stdlib.h>

static int   veclen;		/* the feature vector length */
static float *cur_mean = NULL;	/* the mean subtracted from input frames */
static float *sum = NULL;	/* the sum over input frames */
static int   nframe;		/* the total number of input frames */

void mean_norm_init(uint32 vlen)
{
    veclen   = vlen;
    cur_mean = (float *) calloc(veclen, sizeof(float));
    sum      = (float *) calloc(veclen, sizeof(float));
    nframe   = 0;
    printf("mean_norm_init: zero mean\n");
}

void mean_norm_acc_sub(float32 *vec)
{
    int32 i;

    for (i = 1; i < veclen; i++) {
	sum[i] += vec[i];
	vec[i] -= cur_mean[i];
    }
    ++nframe;
}

void mean_norm_update()
{
    int32 i;

    printf("mean_norm_update: from < ");
    for (i = 1; i < veclen; i++)
	printf("%5.2f ", cur_mean[i]);
    printf(">\n");

    for (i = 1; i < veclen; i++) {
	sum[i] /= nframe;
	cur_mean[i] = (cur_mean[i] + sum[i]) / 2;
	sum[i] = 0;
    }
    nframe = 0;

    printf("mean_norm_update: to   < ");
    for (i = 1; i < veclen; i++)
	printf("%5.2f ", cur_mean[i]);
    printf(">\n");
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1995/10/17  13:05:04  eht
 * Cleaned up code a bit so that it is more ANSI compliant
 *
 * Revision 1.2  1995/10/10  12:36:12  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/06/02  20:57:22  eht
 * Initial revision
 *
 *
 */
