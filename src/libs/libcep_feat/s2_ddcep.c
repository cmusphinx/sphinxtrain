/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
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
 * Revision 1.4  2004/07/21  18:05:38  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
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
