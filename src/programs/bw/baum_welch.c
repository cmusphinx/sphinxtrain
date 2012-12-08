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
 * File: baum_welch.c
 * 
 * Description: 
 *	This file contains the code to update the HMM reestimation
 *	equations given a model and an observation sequence of
 *	in a collection of multiple observation sequences (i.e. an
 *	utterance in a corpus of utterances).
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "baum_welch.h"
#include "forward.h"
#include "viterbi.h"
#include "backward.h"
#include "accum.h"
#include <s3/state_seq.h>
#include <s3/model_inventory.h>
#include <s3/corpus.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/profile.h>

#include <s3/s3.h>

#include <math.h>
#include <assert.h>
#include <string.h>

/*********************************************************************
 *
 * Function:	baum_welch_update()
 * 
 * Description: 
 *	This routine updates the HMM reestimation equations given a
 *	model and an observation sequence of in a collection of
 *	multiple observation sequences (i.e. an	utterance in a
 *	corpus of utterances).
 *
 * Function Inputs: 
 *	float64 *log_forw_prob-
 *		log() of the probability of observation sequence
 *		given the model.
 *
 *	vector_t **feature -
 *		The observed feature vectors.	feature[t][f]
 *		is the feature vector for time t, feature f.
 *
 *	uint32 n_obs -
 *		The number of observations.  Defines the
 *		number of rows in the feature matrix.
 *
 *	state_t *state -
 *		The set of HMM states of the HMM to train.
 *
 *	uint32 n_state -
 *		The number of states present in the HMM.
 *
 *	model_inventory_t *inv -
 *		A structure containing pointers to the initial
 *		model parameters and reestimation sums.
 *
 *	int32 mixw_reest -
 *		A boolean value to indicate whether mixing weights
 *		should be reestimated.
 *
 *	int32 tmat_reest -
 *		A boolean value to indicate whether transition matrices
 *		should be reestimated.
 *
 *	int32 mean_reest -
 *		A boolean value to indicate whether means
 *		should be reestimated.
 *
 *	int32 var_reest -
 *		A boolean value to indicate whether variances
 *		should be reestimated.
 * 
 *      s3phseg_t *phseg -
 *              An optional phone segmentation to use to constrain the
 *              forward lattice.
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 *	None
 * 
 *********************************************************************/

int32
baum_welch_update(float64 *log_forw_prob,
		  vector_t **feature,
		  uint32 n_obs,
		  state_t *state,
		  uint32 n_state,
		  model_inventory_t *inv,
		  float64 a_beam,
		  float64 b_beam,
		  float32 spthresh,
		  s3phseg_t *phseg,
		  int32 mixw_reest,
		  int32 tmat_reest,
		  int32 mean_reest,
		  int32 var_reest,
		  int32 pass2var,
		  int32 var_is_full,
		  FILE *pdumpfh,
		  bw_timers_t *timers,
		  feat_t *fcb)
{
    float64 *scale = NULL;
    float64 **dscale = NULL;
    float64 **active_alpha;
    uint32 **active_astate;
    uint32 **bp;
    uint32 *n_active_astate;
    float64 log_fp;	/* accumulator for the log of the probability
			 * of observing the input given the model */
    uint32 t;		/* time */
    int ret;
    uint32 i,j;

    /* caller must ensure that there is some non-zero amount
       of work to be done here */
    assert(n_obs > 0);
    assert(n_state > 0);

    scale = (float64 *)ckd_calloc(n_obs, sizeof(float64));
    dscale = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    n_active_astate = (uint32 *)ckd_calloc(n_obs, sizeof(uint32));
    active_alpha  = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    active_astate = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));
    bp = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));

    /* Compute the scaled alpha variable and scale factors
     * for all states and time subject to the pruning constraints */
    if (timers)
	ptmr_start(&timers->fwd_timer);

/*
 * Debug?
 *   E_INFO("Before Forward search\n");
 */
    ret = forward(active_alpha, active_astate, n_active_astate, bp,
		  scale, dscale,
		  feature, n_obs, state, n_state,
		  inv, a_beam, phseg, timers, 0);

#if BW_DEBUG
    for (i=0 ; i < n_obs;i++){
      E_INFO("Number of active states %d at time %d\n",n_active_astate[i],i);
      E_INFO("Scale of time %d is %e \n",i,scale[i]);
      for(j=0 ; j < n_active_astate[i];j++){
	E_INFO("Active state: %d Active alpha: %e\n",active_astate[i][j], active_alpha[i][j]);
      }
    }
    i=0;
    j=0;
#endif

    /* Dump a phoneme segmentation if requested */
    if (cmd_ln_str("-outphsegdir")) {
	    const char *phsegdir;
	    char *segfn, *uttid;

	    phsegdir = cmd_ln_str("-outphsegdir");
	    uttid = (cmd_ln_int32("-outputfullpath")
		     ? corpus_utt_full_name() : corpus_utt());
	    segfn = ckd_calloc(strlen(phsegdir) + 1
			       + strlen(uttid)
			       + strlen(".phseg") + 1, 1);
	    strcpy(segfn, phsegdir);
	    strcat(segfn, "/");
	    strcat(segfn, uttid);
	    strcat(segfn, ".phseg");
	    write_phseg(segfn, inv, state, active_astate, n_active_astate,
			n_state, n_obs, active_alpha, scale, bp);
	    ckd_free(segfn);
    }

    if (timers)
	ptmr_stop(&timers->fwd_timer);

    if (ret != S3_SUCCESS) {

	/* Some problem with the utterance, release per utterance storage and
	 * forget about adding the utterance accumulators to the global accumulators */

	goto error;
    }

    /* Compute the scaled beta variable and update the reestimation
     * sums */
    if (timers)
	ptmr_start(&timers->bwd_timer);

#if BW_DEBUG
    E_INFO("Before Backward search\n");
#endif

    ret = backward_update(active_alpha, active_astate, n_active_astate, scale, dscale,
			  feature, n_obs,
			  state, n_state,
			  inv, b_beam, spthresh,
			  mixw_reest, tmat_reest, mean_reest, var_reest, pass2var,
			  var_is_full, pdumpfh, timers, fcb);
    if (timers)
	ptmr_stop(&timers->bwd_timer);

    if (ret != S3_SUCCESS) {

	/* Some problem with the utterance, release per utterance storage and
	 * forget about adding the utterance accumulators to the global accumulators */

	goto error;
    }

#if BW_DEBUG
    E_INFO("Before Global Accumulation\n");
#endif

    /* If no error was found in the forward or backward procedures,
     * add the resulting utterance reestimation accumulators to the
     * global reestimation accumulators */
    if (timers)
	ptmr_start(&timers->rstu_timer);
    accum_global(inv, state, n_state,
		 mixw_reest, tmat_reest, mean_reest, var_reest,
		 var_is_full);
    if (timers)
	ptmr_stop(&timers->rstu_timer);

    for (i = 0; i < n_active_astate[n_obs-1] && active_astate[n_obs-1][i] != (n_state-1); i++);

    assert(i < n_active_astate[n_obs-1]);

    /* Calculate log[ p( O | \lambda ) ] */
    assert(active_alpha[n_obs-1][i] > 0);
    log_fp = log(active_alpha[n_obs-1][i]);
    for (t = 0; t < n_obs; t++) {
	assert(scale[t] > 0);
	log_fp -= log(scale[t]);
        for (j = 0; j < inv->gauden->n_feat; j++) {
	    log_fp += dscale[t][j];
        }
    }

    *log_forw_prob = log_fp;

    ckd_free((void *)scale);
    ckd_free(n_active_astate);
    for (i = 0; i < n_obs; i++) {
	ckd_free((void *)active_alpha[i]);
	ckd_free((void *)active_astate[i]);
	ckd_free((void *)dscale[i]);
	ckd_free((void *)bp[i]);
    }
    ckd_free((void *)active_alpha);
    ckd_free((void *)active_astate);
    ckd_free((void **)dscale);
    ckd_free(bp);

    return S3_SUCCESS;

error:
    ckd_free((void *)scale);
    for (i = 0; i < n_obs; i++) {
	if (dscale[i])
	    ckd_free((void *)dscale[i]);
    }
    ckd_free((void **)dscale);
    
    ckd_free(n_active_astate);
    for (i = 0; i < n_obs; i++) {
	ckd_free((void *)active_alpha[i]);
	ckd_free((void *)active_astate[i]);
	ckd_free((void *)bp[i]);
    }
    ckd_free((void *)active_alpha);
    ckd_free((void *)active_astate);
    ckd_free(bp);

    E_ERROR("%s ignored\n", corpus_utt_brief_name());

    return S3_ERROR;
}
