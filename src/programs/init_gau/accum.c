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
 * File: accum.c
 * 
 * Description: 
 * 	Accumulate mean and variance estimation sums
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "accum.h"


int
accum_state_mean(vector_t ***mean,
		 float32 ***dnom,
		 vector_t **feat,
		 uint32 *del_b,
		 uint32 *del_e,
		 uint32 n_del,
		 uint32 n_feat,
		 const uint32 *veclen,
		 uint32 *sseq,
		 uint32 *ci_sseq,
		 uint32 n_frame)
{
    uint32 t;		/* time (in frames) */
    uint32 s;		/* a tied state */
    uint32 ci_s;	/* a CI tied state */
    uint32 f;		/* a feature stream idx */
    uint32 c;		/* a vector component idx */

    for (t = 0; t < n_frame; t++) {
	if (sseq && ci_sseq) {
	    s = sseq[t];
	    ci_s = ci_sseq[t];
	}
	else {
	    s = 0;
	    ci_s = 0;
	}

	for (f = 0; f < n_feat; f++) {
	    dnom[s][f][0] += 1.0;
	    if (s != ci_s) {
		dnom[ci_s][f][0] += 1.0;
	    }
	    for (c = 0; c < veclen[f]; c++) {
		/* only one Gaussian per state */

		mean[s][f][0][c] += feat[t][f][c];

		if (s != ci_s) {
		    mean[ci_s][f][0][c] += feat[t][f][c];
		}
	    }
	}
    }
    return 0;
}

int
accum_state_var(vector_t ***var,
		vector_t ***mean,
		float32  ***dnom,
		vector_t **feat,
		uint32 *del_b,
		uint32 *del_e,
		uint32 n_del,
		uint32 n_feat,
		const uint32 *veclen,
		uint32 *sseq,
		uint32 *ci_sseq,
		uint32 n_frame)
{
    uint32 t;		/* time (in frames) */
    uint32 s;		/* a tied state */
    uint32 ci_s;	/* a CI tied state */
    uint32 f;		/* a feature stream idx */
    uint32 c;		/* a vector component idx */
    float32 diff;

    for (t = 0; t < n_frame; t++) {

	if (sseq && ci_sseq) {
	    /* get the tied state for time t */
	    s = sseq[t];
	
	    /* get the CI state as well */
	    ci_s = ci_sseq[t];
	}
	else {
	    s = 0;
	    ci_s = 0;
	}

	for (f = 0; f < n_feat; f++) {
	    dnom[s][f][0] += 1.0;
	    if (s != ci_s) {
		dnom[ci_s][f][0] += 1.0;
	    }
	    for (c = 0; c < veclen[f]; c++) {
		/* only one Gaussian per state */

		diff = feat[t][f][c] - mean[s][f][0][c];
		var[s][f][0][c] += diff * diff;

		if (s != ci_s) {
		    diff = feat[t][f][c] - mean[ci_s][f][0][c];
		    var[ci_s][f][0][c] += diff * diff;
		}
	    }
	}
    }
    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.4  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1996/08/06  14:13:04  eht
 * added "const" attribute to veclen arguments
 *
 * Revision 1.2  1996/02/02  17:32:59  eht
 * Added estimation of CI mean/var when only CD states are present
 *
 * Revision 1.1  1995/12/14  19:52:59  eht
 * Initial revision
 *
 *
 */
