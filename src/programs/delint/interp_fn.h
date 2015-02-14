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
 * File: interp_fn.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef INTERP_FN_H
#define INTERP_FN_H

#include <sphinxbase/prim_type.h>

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
