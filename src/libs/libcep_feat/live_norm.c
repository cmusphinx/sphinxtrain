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
 * 4. Redistributions of any form whatsoever must retain the following
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
/*********************************************************************
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
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
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
