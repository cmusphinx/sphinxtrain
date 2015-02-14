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
 * File: interp_fn.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "interp_fn.h"

#include <sphinxbase/matrix.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <s3/s3.h>	/* define TYING_NO_ID */

#include <assert.h>
/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Traceability: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

void
interp_counts_3d_uniform(float32 ***cnt_buf,	/* count of events in
						   observation space (i, j) where
						   START <= i < START+RUN_LENGTH,
						   0 <= j < D2 */
			 uint32 start,
			 uint32 run_length,
			 uint32 d2,	  /* some other # of classes */
			 uint32 n_events, /* number of events in all observation
						     spaces (i, j) */
			 float32 cnt_lambda)	/* weight of event distributions (i, j)
						   relative to uniform probability */
{
    float32 u_lambda = (1.0 - cnt_lambda);
    float32 u_prob;
    float64 wt_u_cnt;
    uint32 i, j, k, end;

    assert (n_events > 0);

    u_prob = 1.0 / (float32) n_events;

    end = start + run_length;

    /*
     * Interpolate counts between distribution arising from the
     * counts and uniform distribution.
     */
    for (i = start; i < end; i++) {
	for (j = 0; j < d2; j++) {

	    /* compute the weighted uniform count for (d1, d2) */
	    for (k = 0, wt_u_cnt = 0; k < n_events; k++) {
		wt_u_cnt += cnt_buf[i][j][k];
	    }

	    wt_u_cnt *= (u_prob * u_lambda);

	    /* interpolate all events in (d1, d2) */
	    for (k = 0; k < n_events; k++) {
		cnt_buf[i][j][k] = cnt_buf[i][j][k] * cnt_lambda + wt_u_cnt;
	    }
	}
    }
}

/*
 * SPHINX-II doesn't automatically compute context independent
 * smoothing weights.  We probably should, but wanted to get comparable
 * system going first.
 */
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
	    uint32 n_gau)
{
    uint32 i, cd_i, ci_i, j, k, l;
    float32 uniform;
    float64 tt_uni, tt_ci, tt_cd;
    uint32 total_n_tied;

    E_INFO("Interpolating CD states\n");

    uniform = 1.0 / (float32)n_gau;

    /* add b buf to a */
    accum_3d(mixw_acc_a, mixw_acc_b,
	     n_mixw, n_feat, n_gau);
    
    for (i = 0; i < n_cd_state; i++) {

	cd_i = i + n_ci_state;

	if (n_tied[i][0] != TYING_NO_ID) {
	    /* n_tied[][] counts the number of times the CD distribution occurs with
	       the corresponding CI distribution (in ci_mixw[][]). */
	    for (j = 0, total_n_tied = 0; n_tied[i][j] != TYING_NO_ID; j++) {
		assert(n_tied[i][j] > 0);
		total_n_tied += n_tied[i][j];

		ci_i = ci_mixw[i][j];
		assert(ci_i != TYING_NO_ID);

		for (k = 0; k < n_feat; k++) {
		    for (l = 0; l < n_gau; l++) {
			if (mixw_acc_a[cd_i][k][l] > MIN_IEEE_NORM_POS_FLOAT32)
			    tt_cd = lambda[i][DIST_CD] * mixw_acc_a[cd_i][k][l] * dnom[cd_i];
			else
			    tt_cd = 0;
		    
			if (mixw_acc_a[ci_i][k][l] > MIN_IEEE_NORM_POS_FLOAT32)
			    tt_ci = lambda[i][DIST_CI] * mixw_acc_a[ci_i][k][l] * dnom[ci_i];
			else
			    tt_ci = 0;
			
			tt_uni = lambda[i][DIST_UNIFORM] * uniform;
			
			if ( j == 0 )
			    mixw_acc_b[cd_i][k][l]  = n_tied[i][j] * (tt_cd + tt_ci + tt_uni);
			else
			    mixw_acc_b[cd_i][k][l] += n_tied[i][j] * (tt_cd + tt_ci + tt_uni);
		    }
		}
	    }
	}
	else {
	    /* for unobserved tied states, make flat */
	    float32 uni = 1.0 / (float)n_gau;

	    for (k = 0; k < n_feat; k++)
		for (l = 0; l < n_gau; l++)
		    mixw_acc_b[cd_i][k][l] = uni;

	    total_n_tied = 1;
	}


	/* avg the probs */
	for (k = 0; k < n_feat; k++)
	    for (l = 0; l < n_gau; l++)
		mixw_acc_b[cd_i][k][l] /= (float32)total_n_tied;

    }

    /* interpolate CI distributions with uniform distribution */
    interp_counts_3d_uniform(mixw_acc_a,
			     0,			/* start state */
			     n_ci_state,	/* run length */
			     n_feat, n_gau,
			     cilambda);

    /* move CI ones to the B buffer, since A will be freed */
    for (i = 0; i < n_ci_state; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_gau; k++) {
		mixw_acc_b[i][j][k] = mixw_acc_a[i][j][k];
	    }
	}
    }

    *out_mixw = mixw_acc_b;
    
    ckd_free_3d((void ***)mixw_acc_a);
}
