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
 * File: s2_dcep.c
 * 
 * Description: 
 * 	Computes the dcep feature:
 *		 dcep: < (cep[t + 2] - cep[t - 2]), (cep[t + 4] - cep[t - 4]) >
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include "s2_dcep.h"

#include <s3/s2_param.h>

#include <string.h>

static uint32 veclen = 13;

void
s2_dcep_set_veclen(uint32 l)
{
    veclen = l;
}

static void
s2_short_dcep_frame(vector_t dcep,
		    vector_t power,
		    vector_t *mfcc)
{
    int32 w;
    int32 k;

    w = S2_SHORT_DIFFW;

    power[1] = mfcc[w][0] - mfcc[-w][0];

    for (k = 1; k < veclen; k++) {
	dcep[k-1] = mfcc[w][k] - mfcc[-w][k];
    }
}

void
s2_short_dcep_feat(vector_t **out,
		   vector_t *mfcc,
		   uint32 n_frame)
{
    uint32 i, k;

    /* compute the short duration diff cepstrum */
    for (i = S2_SHORT_DIFFW; i < n_frame - S2_SHORT_DIFFW; i++) {
	s2_short_dcep_frame(out[i][S2_DCEP_FEATURE],
			    out[i][S2_POW_FEATURE],
			    &mfcc[i]);
    }

    for (i = 0; i < S2_SHORT_DIFFW; i++) {
	memcpy(out[i][S2_DCEP_FEATURE],
	       out[S2_SHORT_DIFFW][S2_DCEP_FEATURE],
	       sizeof(float32) * (veclen-1));

	out[i][S2_POW_FEATURE][S2_DCEP_FEATURE] =
	    out[S2_SHORT_DIFFW][S2_POW_FEATURE][S2_DCEP_FEATURE];
    }

    for (i = n_frame - 1, k = n_frame - S2_SHORT_DIFFW - 1;
	 i >= n_frame - S2_SHORT_DIFFW; i--) {
	memcpy(out[i][S2_DCEP_FEATURE],
	       out[k][S2_DCEP_FEATURE],
	       sizeof(float32) * (veclen - 1));

	out[i][S2_POW_FEATURE][S2_DCEP_FEATURE] =
	    out[k][S2_POW_FEATURE][S2_DCEP_FEATURE];
    }
}

static void
s2_long_dcep_frame(vector_t dcep,
		   vector_t *mfcc)
{
    int32 w, k;

    w = S2_LONG_DIFFW;

    for (k = 1; k < S2_CEP_VECLEN; k++) {
	dcep[k-1] = mfcc[w][k] - mfcc[-w][k];
    }
}

void
s2_long_dcep_feat(vector_t **out,
		  vector_t *mfcc,
		  uint32 n_frame)
{
    uint32 i, k;

    /* compute the long duration diff cepstrum */
    for (i = S2_LONG_DIFFW; i < n_frame - S2_LONG_DIFFW; i++) {
	s2_long_dcep_frame(out[i][S2_DCEP_FEATURE] + (veclen-1),
			   &mfcc[i]);
    }
    
    for (i = 0; i < S2_LONG_DIFFW; i++) {
	memcpy(out[i][S2_DCEP_FEATURE] + (veclen-1),
	       out[S2_LONG_DIFFW][S2_DCEP_FEATURE] + (veclen-1),
	       sizeof(float32) * (veclen-1));
    }

    for (i = n_frame-1, k = n_frame - S2_LONG_DIFFW - 1;
	 i >= n_frame - S2_LONG_DIFFW; i--) {
	memcpy(out[i][S2_DCEP_FEATURE] + (veclen-1),
	       out[k][S2_DCEP_FEATURE] + (veclen-1),
	       sizeof(float32) * (veclen-1));
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1996/08/05  12:51:29  eht
 * Include <string.h> for memcpy() prototype
 *
 * Revision 1.2  1996/03/25  15:36:31  eht
 * Changes to allow for settable input feature length
 *
 * Revision 1.1  1995/12/14  20:12:58  eht
 * Initial revision
 *
 *
 */
