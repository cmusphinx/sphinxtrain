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
 * File: ddcep.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include "ddcep_frame.h"
#include "dcep_frame.h"

#include <s3/feat.h>

uint32 dd_wsize = 1;
uint32 d_wsize = 2;

void
ddcep_frame_set_window_size(uint32 ws)
{
    d_wsize = dcep_frame_short_window_size();
    dd_wsize = ws;
}

uint32
ddcep_frame_window_size()
{
    return dd_wsize;
}

void
ddcep_frame(vector_t ddcep,
	    vector_t power,
	    vector_t mfcc)
{
    int32 d_w;
    int32 dd_w;
    int32 mfcc_frame_len;
    int32 k;
    float32 d1;
    float32 d2;

    mfcc_frame_len = feat_mfcc_len();

    /* compute dcep window offset in terms of coefficients */
    d_w = d_wsize * mfcc_frame_len;

    /* compute ddcep window offset in terms of coefficients */
    dd_w = dd_wsize * mfcc_frame_len;

    /* compute 2nd diff of c[0] */
    d1 = mfcc[d_w + dd_w] - mfcc[-d_w + dd_w];	/* dcep[0] of dd_wsize frames in future */
    d2 = mfcc[d_w - dd_w] - mfcc[-d_w - dd_w];	/* dcep[0] of dd_wsize frames in past */

    power[2] = d1 - d2;				/* ddcep[0] is third component of power feature */

    for (k = 1; k < mfcc_frame_len; k++) {
	/* compute 2nd diff of mfcc[k] */

	/* dcep[k] of dd_wsize frames in future */
	d1 = mfcc[k + d_w + dd_w] - mfcc[k - d_w + dd_w];

	/* dcep[k] of dd_wsize frames in past */
	d2 = mfcc[k + d_w - dd_w] - mfcc[k - d_w - dd_w];

	ddcep[k-1] = d1 - d2;
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1996/01/26  18:04:51  eht
 * Initial revision
 *
 *
 */
