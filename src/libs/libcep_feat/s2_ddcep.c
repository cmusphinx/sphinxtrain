
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
 * File: s2_ddcep.c
 * 
 * Description: 
 * 	Computes the 2nd order difference cepstrum:
 *		ddcep: < dcep[t+1] - dcep[t-1] >
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include "s2_ddcep.h"

#include <s3/s2_param.h>

#include <string.h>


static uint32 veclen = 13;

void
s2_ddcep_set_veclen(uint32 l)
{
    veclen = l;
}

void
s2_sec_ord_dcep_frame(vector_t ddcep,
		      vector_t power,
		      vector_t *mfcc)
{
    int32 w;
    int32 k;
    float32 d1;
    float32 d2;

    w = S2_SHORT_DIFFW;

    /* diff cep + 1 */
    d1 = mfcc[w + 1][0] - mfcc[-w + 1][0];

    /* diff cep - 1 */
    d2 = mfcc[-1 + w][0] - mfcc[-1 - w][0];

    power[2] = d1 - d2;

    for (k = 1; k < veclen; k++) {
	d1 = mfcc[w + 1][k] - mfcc[-w + 1][k];
	d2 = mfcc[w - 1][k] - mfcc[-w - 1][k];

	ddcep[k-1] = d1 - d2;
    }
}

void
s2_sec_ord_dcep_feat(vector_t **out,
		     vector_t *mfcc,
		     uint32 n_frame)
{
    uint32 i, k;

    /* compute the long duration diff cepstrum */
    for (i = S2_2ND_ORD_DIFFW; i < n_frame - S2_2ND_ORD_DIFFW; i++) {
	s2_sec_ord_dcep_frame(out[i][S2_2DCEP_FEATURE],
			      out[i][S2_POW_FEATURE],
			      &mfcc[i]);
    }
    
    for (i = 0; i < S2_2ND_ORD_DIFFW; i++) {
	out[i][S2_POW_FEATURE][2] =
	    out[S2_2ND_ORD_DIFFW][S2_POW_FEATURE][2];

	memcpy(out[i][S2_2DCEP_FEATURE],
	       out[S2_2ND_ORD_DIFFW][S2_2DCEP_FEATURE],
	       sizeof(float32) * (veclen-1));
    }

    for (i = n_frame-1, k = n_frame - S2_2ND_ORD_DIFFW - 1;
	 i >= n_frame - S2_2ND_ORD_DIFFW; i--) {
	out[i][S2_POW_FEATURE][2] =
	    out[k][S2_POW_FEATURE][2];
	
	memcpy(out[i][S2_2DCEP_FEATURE],
	       out[k][S2_2DCEP_FEATURE],
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
 * Revision 1.3  1996/08/05  12:53:10  eht
 * Include <string.h> for prototype
 *
 * Revision 1.2  1996/03/25  15:36:31  eht
 * Changes to allow for settable input feature length
 *
 * Revision 1.1  1995/12/14  20:12:58  eht
 * Initial revision
 *
 *
 */
