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
 * File: interp_fn.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef INTERP_FN_H
#define INTERP_FN_H

#include <s3/prim_type.h>


void
interp_counts_3d_uniform(float32 ***cnt_buf,	/* count of events in
						   observation space (i, j) where
						   START <= i < RUN_LENGTH, 0 <= j < D2 */
			 uint32 start,
			 uint32 run_length,
			 uint32 d2,	  /* some other # of classes */
			 uint32 n_events, /* number of events in all observation
						     spaces (i, j) */
			 float32 cnt_lambda);	/* weight of event distributions (i, j)
						   relative to uniform probability */

void
interp_mixw(float32 ****out_mixw,
	    
	    float32 ***mixw_acc_a,
	    float32 ***mixw_acc_b,
	    float64 *dnom,

	    float32 **lambda,
	    float32 cilambda,
	    
	    uint32 **ci_mixw,
	    uint32 **n_tied,

	    uint32 n_cd_state,
	    uint32 n_ci_state,

	    uint32 n_mixw,
	    uint32 n_feat,
	    uint32 n_gau);

#define N_DIST_TYPE 3

typedef enum {
    DIST_UNIFORM=0,
    DIST_CI=1,
    DIST_CD=2
} dist_type_t;


#endif /* INTERP_FN_H */ 

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
 * Revision 1.3  1995/10/10  12:53:26  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/08/15  13:39:10  eht
 * Change interface
 *
 * Revision 1.1  1995/08/09  00:38:37  eht
 * Initial revision
 *
 *
 */
