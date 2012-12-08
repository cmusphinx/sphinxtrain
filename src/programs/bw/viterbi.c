/* -*- c-basic-offset: 4 -*- */
/* ====================================================================
 * Copyright (c) 1996-2007 Carnegie Mellon University.  All rights 
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
 * File: viterbi.c
 * 
 * Description: 
 * 
 * Authors: 
 * 	David Huggins-Daines
 *      Eric Thayer
 *********************************************************************/

#include "forward.h"
#include "backward.h"
#include "viterbi.h"
#include "accum.h"
#include "baum_welch.h"

#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/byteorder.h>
#include <sphinxbase/profile.h>

#include <s3/remap.h>
#include <s3/corpus.h>
#include <s3/s3phseg_io.h>
#include <s3/model_def.h>

#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define INVLOGS3 100000.5 /* (1.0/log(1.0001)) */
int32
write_phseg(const char *filename,
	    model_inventory_t *modinv,
	    state_t *state_seq,
	    uint32 **active_astate,
	    uint32 *n_active_astate,
	    uint32 n_state,
	    uint32 n_obs,
	    float64 **active_alpha,
	    float64 *scale,
	    uint32 **bp)
{
    FILE *fh;
    int32 t;
    uint32 q;
    s3phseg_t *phseg, *next;
    model_def_t *mdef;
    model_def_entry_t *defn;
    uint32 n_defn;
    float64 ascr;

    /* Find the non-emitting ending state */
    for (q = 0; q < n_active_astate[n_obs-1]; ++q) {
	if (active_astate[n_obs-1][q] == n_state-1)
	    break;
    }
    if (q == n_active_astate[n_obs-1]) {
	E_ERROR("Failed to align audio to trancript: final state of the search is not reached\n");
	return S3_ERROR;
    }

    if ((fh = fopen(filename, "w")) == NULL) {
	return S3_ERROR;
    }

    /* Backtrace and build a phone segmentation */
    mdef = modinv->mdef;
    defn = mdef->defn;
    n_defn = mdef->n_defn;
    phseg = NULL;
    ascr = 0;
    for (t = n_obs-1; t >= 0; --t) {
	uint32 j;

	j = active_astate[t][q];

	/* Follow any non-emitting states at time t first. */
	if (state_seq[j].mixw == TYING_NON_EMITTING) {
	    s3phseg_t *prev;
	    uint32 phn;
	    int k, l;

	    while (state_seq[j].mixw == TYING_NON_EMITTING) {
		j = active_astate[t][bp[t][q]];
		q = bp[t][q];
	    }
	    

	    /* Do a rather nasty mdef scan to find the triphone in question. */
	    for (phn = 0; phn < n_defn; phn++) {

    	        for (k = defn[phn].n_state - 2, l = j; k >= 0 && l >= 0; k--, l--) {
	    	    if (state_seq[l].mixw != defn[phn].state[k])
	    		break;
	        }

	        if (k < 0)
	    	    break;
	    }
	    
	    if (phn == n_defn) {
		E_ERROR("Failed to find triphone for senone %u\n", state_seq[j].mixw);
	    }
	    
	    /* Record ascr and sf for the next phone */
	    if (phseg) {
		phseg->score = (int32)(ascr * INVLOGS3);
		phseg->sf = t + 1;
		ascr = 0;
	    }
	    prev = ckd_calloc(1, sizeof(*prev));
	    prev->next = phseg;
	    prev->phone = phn;
	    prev->ef = t;
	    phseg = prev;
	}

	/* Multiply alphas to get "acoustic score" for this phone */
	ascr += (log(active_alpha[t][q]) + log(scale[t]));

	/* Backtrace. */
	if (t > 0) {
	    q = bp[t][q];
	}
    }

    fprintf(fh, "\t%5s %5s %9s %s\n", "SFrm", "EFrm", "SegAScr", "Phone");
    phseg->sf = 0; /* vacuous */
    while (phseg) {
	next = phseg->next;
        fprintf(fh, "\t%5d %5d %9d %s\n",
                phseg->sf, phseg->ef, phseg->score,
		acmod_set_id2name(mdef->acmod_set, phseg->phone));
	ckd_free(phseg);
	phseg = next;
    }

    fclose(fh);
    return S3_SUCCESS;
}

int32
write_s2stseg(const char *filename,
	      state_t *state_seq,
	      uint32 **active_astate,
	      uint32 *n_active_astate,
	      uint32 n_state,
	      uint32 n_obs,
	      uint32 **bp)
{
    FILE *fh;
    uint32 q;
    int32 t;
    uint16 word, *stseg;

    /* Backtrace and build a phone segmentation. */
    /* Find the non-emitting ending state */
    for (q = 0; q < n_active_astate[n_obs-1]; ++q) {
	if (active_astate[n_obs-1][q] == n_state-1)
	    break;
    }
    if (q == n_active_astate[n_obs-1]) {
	E_ERROR("Failed to align audio to trancript: final state of the search is not reached\n");
	return S3_ERROR;
    }

    if ((fh = fopen(filename, "wb")) == NULL) {
	return S3_ERROR;
    }

    word = n_obs;
    SWAP_INT16(&word);
    fwrite(&word, 2, 1, fh);

    stseg = ckd_calloc(n_obs, sizeof(uint16));

    for (t = n_obs-1; t >= 0; --t) {
	uint32 j;

	j = active_astate[t][q];

	/* Follow any non-emitting states at time t first. */
	while (state_seq[j].mixw == TYING_NON_EMITTING) {
	    j = active_astate[t][bp[t][q]];
	    q = bp[t][q];
	}

	/* mixw = senone (we hope!) */
	stseg[t] = state_seq[j].mixw;
	SWAP_INT16(&stseg[t]);

	/* Backtrace. */
	if (t > 0) {
	    q = bp[t][q];
	}
    }

    fwrite(stseg, 2, n_obs, fh);
    ckd_free(stseg);
    fclose(fh);
    return S3_SUCCESS;
}

int32
viterbi_update(float64 *log_forw_prob,
	       vector_t **feature,
	       uint32 n_obs,
	       state_t *state_seq,
	       uint32 n_state,
	       model_inventory_t *inv,
	       float64 a_beam,
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
    gauden_t *g;		/* Gaussian density parameters and
				   reestimation sums */
    float32 ***mixw;		/* all mixing weights */
    float64 ***now_den = NULL;	/* Short for den[t] */
    uint32 ***now_den_idx = NULL;/* Short for den_idx[t] */
    uint32 *active_cb;
    uint32 n_active_cb;
    float32 **tacc;		/* Transition matrix reestimation sum accumulators
				   for the utterance. */
    float32 ***wacc;		/* mixing weight reestimation sum accumulators
				   for the utterance. */
    float32 ***denacc = NULL;	/* mean/var reestimation accumulators for time t */
    size_t denacc_size;		/* Total size of data references in denacc.  Allows
				   for quick clears between time frames */
    uint32 n_lcl_cb;
    uint32 *cb_inv;
    uint32 i, j, q;
    int32 t;
    uint32 n_feat;
    uint32 n_density;
    uint32 n_top;
    int ret;
    float64 log_fp;	/* accumulator for the log of the probability
			 * of observing the input given the model */
    uint32 max_n_next = 0;
    uint32 n_cb;

    static float64 *p_op = NULL;
    static float64 *p_ci_op = NULL;
    static float64 **d_term = NULL;
    static float64 **d_term_ci = NULL;

    /* caller must ensure that there is some non-zero amount
       of work to be done here */
    assert(n_obs > 0);
    assert(n_state > 0);

    g = inv->gauden;
    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);
    n_top = gauden_n_top(g);
    n_cb = gauden_n_mgau(g);

    if (p_op == NULL) {
	p_op    = ckd_calloc(n_feat, sizeof(float64));
	p_ci_op = ckd_calloc(n_feat, sizeof(float64));
    }

    if (d_term == NULL) {
	d_term    = (float64 **)ckd_calloc_2d(n_feat, n_top, sizeof(float64));
	d_term_ci = (float64 **)ckd_calloc_2d(n_feat, n_top, sizeof(float64));
    }

    scale = (float64 *)ckd_calloc(n_obs, sizeof(float64));
    dscale = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    n_active_astate = (uint32 *)ckd_calloc(n_obs, sizeof(uint32));
    active_alpha  = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    active_astate = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));
    active_cb = ckd_calloc(2*n_state, sizeof(uint32));
    bp = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));

    /* Run forward algorithm, which has embedded Viterbi. */
    if (timers)
	ptmr_start(&timers->fwd_timer);
    ret = forward(active_alpha, active_astate, n_active_astate, bp,
		  scale, dscale,
		  feature, n_obs, state_seq, n_state,
		  inv, a_beam, phseg, timers, 0);
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
	    write_phseg(segfn, inv, state_seq, active_astate, n_active_astate,
			n_state, n_obs, active_alpha, scale, bp);
	    ckd_free(segfn);
    }
    if (timers)
	ptmr_stop(&timers->fwd_timer);


    if (ret != S3_SUCCESS) {

	/* Some problem with the utterance, release per utterance storage and
	 * forget about adding the utterance accumulators to the global accumulators */

	goto all_done;
    }

    mixw = inv->mixw;

    if (mixw_reest) {
	/* Need to reallocate mixing accumulators for utt */
	if (inv->l_mixw_acc) {
	    ckd_free_3d((void ***)inv->l_mixw_acc);
	    inv->l_mixw_acc = NULL;
	}
	inv->l_mixw_acc = (float32 ***)ckd_calloc_3d(inv->n_mixw_inverse,
						     n_feat,
						     n_density,
						     sizeof(float32));
    }
    wacc = inv->l_mixw_acc;
    n_lcl_cb = inv->n_cb_inverse;
    cb_inv = inv->cb_inverse;

    /* Allocate local accumulators for mean, variance reestimation
       sums if necessary */
    gauden_alloc_l_acc(g, n_lcl_cb,
		       mean_reest, var_reest,
		       var_is_full);

    if (tmat_reest) {
	if (inv->l_tmat_acc) {
	    ckd_free_2d((void **)inv->l_tmat_acc);
	    inv->l_tmat_acc = NULL;
	}
	for (i = 0; i < n_state; i++) {
	    if (state_seq[i].n_next > max_n_next)
		max_n_next = state_seq[i].n_next;
	}
	inv->l_tmat_acc = (float32 **)ckd_calloc_2d(n_state,
						    max_n_next,
						    sizeof(float32));
    }
    /* transition matrix reestimation sum accumulators
       for the utterance */
    tacc = inv->l_tmat_acc;

    n_active_cb = 0;
    now_den = (float64 ***)ckd_calloc_3d(n_lcl_cb,
					 n_feat,
					 n_top,
					 sizeof(float64));
    now_den_idx =  (uint32 ***)ckd_calloc_3d(n_lcl_cb,
					     n_feat,
					     n_top,
					     sizeof(uint32));

    if (mean_reest || var_reest) {
	/* allocate space for the per frame density counts */
	denacc = (float32 ***)ckd_calloc_3d(n_lcl_cb,
					    n_feat,
					    n_density,
					    sizeof(float32));

	/* # of bytes required to store all weighted vectors */
	denacc_size = n_lcl_cb * n_feat * n_density * sizeof(float32);
    }
    else {
	denacc = NULL;
	denacc_size = 0;
    }

    /* Okay now run through the backtrace and accumulate counts. */
    /* Find the non-emitting ending state */
    for (q = 0; q < n_active_astate[n_obs-1]; ++q) {
	if (active_astate[n_obs-1][q] == n_state-1)
	    break;
    }
    if (q == n_active_astate[n_obs-1]) {
	E_ERROR("Failed to align audio to trancript: final state of the search is not reached\n");
	ret = S3_ERROR;
	goto all_done;
    }

    for (t = n_obs-1; t >= 0; --t) {
	uint32 l_cb;
	uint32 l_ci_cb;
	float64 op, p_reest_term;
	uint32 prev;

	j = active_astate[t][q];

	/* Follow any non-emitting states at time t first. */
	while (state_seq[j].mixw == TYING_NON_EMITTING) {
	    prev = active_astate[t][bp[t][q]];

#if VITERBI_DEBUG
	    printf("Following non-emitting state at time %d, %u => %u\n",
		   t, j, prev);
#endif
	    /* Backtrace and accumulate transition counts. */
	    if (tmat_reest) {
		assert(tacc != NULL);
		tacc[prev][j - prev] += 1.0;
	    }
	    q = bp[t][q];
	    j = prev;
	}

	/* Now accumulate statistics for the real state. */
	l_cb = state_seq[j].l_cb;
	l_ci_cb = state_seq[j].l_ci_cb;
	n_active_cb = 0;

	if (timers)
	    ptmr_start(&timers->gau_timer);

	gauden_compute_log(now_den[l_cb],
			   now_den_idx[l_cb],
			   feature[t],
			   g,
			   state_seq[j].cb,
			   NULL);
	active_cb[n_active_cb++] = l_cb;

	if (l_cb != l_ci_cb) {
	    gauden_compute_log(now_den[l_ci_cb],
			       now_den_idx[l_ci_cb],
			       feature[t],
			       g,
			       state_seq[j].ci_cb,
			       NULL);
	    active_cb[n_active_cb++] = l_ci_cb;
	}
	gauden_scale_densities_bwd(now_den, now_den_idx,
				   &dscale[t],
				   active_cb, n_active_cb, g);

	assert(state_seq[j].mixw != TYING_NON_EMITTING);
	/* Now calculate mixture densities. */
	/* This is the normalizer sum_m c_{jm} p(o_t|\lambda_{jm}) */
	op = gauden_mixture(now_den[l_cb], now_den_idx[l_cb],
			    mixw[state_seq[j].mixw], g);
        if (timers)
	    ptmr_stop(&timers->gau_timer);

	if (timers)
	    ptmr_start(&timers->rsts_timer);
	/* Make up this bogus value to be consistent with backward.c */
	p_reest_term = 1.0 / op;

	/* Compute the output probability excluding the contribution
	 * of each feature stream.  i.e. p_op[0] is the output
	 * probability excluding feature stream 0 */
	partial_op(p_op,
		   op,
		   now_den[l_cb],
		   now_den_idx[l_cb],
		   mixw[state_seq[j].mixw],
		   n_feat,
		   n_top);

	/* compute the probability of each (of possibly topn) density */
	den_terms(d_term,
		  p_reest_term,
		  p_op,
		  now_den[l_cb],
		  now_den_idx[l_cb],
		  mixw[state_seq[j].mixw],
		  n_feat,
		  n_top);

	if (l_cb != l_ci_cb) {
	    /* For each feature stream f, compute:
	     *     sum_k(mixw[f][k] den[f][k])
	     * and store the results in p_ci_op */
	    partial_ci_op(p_ci_op,
			  now_den[l_ci_cb],
			  now_den_idx[l_ci_cb],
			  mixw[state_seq[j].ci_mixw],
			  n_feat,
			  n_top);

	    /* For each feature stream and density compute the terms:
	     *   w[f][k] den[f][k] / sum_k(w[f][k] den[f][k]) * post_j
	     * and store results in d_term_ci */
	    den_terms_ci(d_term_ci,
			 1.0, /* post_j = 1.0 */
			 p_ci_op,
			 now_den[l_ci_cb],
			 now_den_idx[l_ci_cb],
			 mixw[state_seq[j].ci_mixw],
			 n_feat,
			 n_top);
	}
		    

	/* accumulate the probability for each density in the mixing
	 * weight reestimation accumulators */
	if (mixw_reest) {
	    accum_den_terms(wacc[state_seq[j].l_mixw], d_term,
			    now_den_idx[l_cb], n_feat, n_top);

	    /* check if mixw and ci_mixw are different to avoid
	     * doubling the EM counts in a CI run. */
	    if (state_seq[j].mixw != state_seq[j].ci_mixw) {
                if (n_cb < inv->n_mixw) {
                    /* semi-continuous, tied mixture, and discrete case */
		    accum_den_terms(wacc[state_seq[j].l_ci_mixw], d_term,
				    now_den_idx[l_cb], n_feat, n_top);
		}
		else {
		    /* continuous case */
		    accum_den_terms(wacc[state_seq[j].l_ci_mixw], d_term_ci,
				    now_den_idx[l_ci_cb], n_feat, n_top);
		}
	    }
	}
		    
	/* accumulate the probability for each density in the 
	 * density reestimation accumulators */
	if (mean_reest || var_reest) {
	    accum_den_terms(denacc[l_cb], d_term,
			    now_den_idx[l_cb], n_feat, n_top);
	    if (l_cb != l_ci_cb) {
		accum_den_terms(denacc[l_ci_cb], d_term_ci,
				now_den_idx[l_ci_cb], n_feat, n_top);
	    }
	}
		
	if (timers)
	    ptmr_stop(&timers->rsts_timer);
	/* Note that there is only one state/frame so this is kind of
	   redundant */
        if (timers)
    	    ptmr_start(&timers->rstf_timer);
	if (mean_reest || var_reest) {
	    /* Update the mean and variance reestimation accumulators */
	    if (pdumpfh)
		fprintf(pdumpfh, "time %d:\n", t);
	    accum_gauden(denacc,
			 cb_inv,
			 n_lcl_cb,
			 feature[t],
			 now_den_idx,
			 g,
			 mean_reest,
			 var_reest,
			 pass2var,
			 inv->l_mixw_acc,
			 var_is_full,
			 pdumpfh,
			 fcb);
	    memset(&denacc[0][0][0], 0, denacc_size);
	}
        if (timers)
	    ptmr_stop(&timers->rstf_timer);

	if (t > 0) { 
	    prev = active_astate[t-1][bp[t][q]];
#if VITERBI_DEBUG
	    printf("Backtrace at time %d, %u => %u\n",
		   t, j, prev);
#endif
	    /* Backtrace and accumulate transition counts. */
	    if (tmat_reest) {
		assert(tacc != NULL);
		tacc[prev][j-prev] += 1.0;
	    }
	    q = bp[t][q];
	    j = prev;
	}
    }

    /* If no error was found, add the resulting utterance reestimation
     * accumulators to the global reestimation accumulators */
    if (timers)
	ptmr_start(&timers->rstu_timer);
    accum_global(inv, state_seq, n_state,
		 mixw_reest, tmat_reest, mean_reest, var_reest,
		 var_is_full);
    if (timers)
	ptmr_stop(&timers->rstu_timer);

    /* Find the final state */
    for (i = 0; i < n_active_astate[n_obs-1]; ++i) {
	if (active_astate[n_obs-1][i] == n_state-1)
	    break;
    }
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

 all_done:
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
    ckd_free((void *)active_cb);

    if (denacc)
	ckd_free_3d((void ***)denacc);

    if (now_den)
	ckd_free_3d((void ***)now_den);
    if (now_den_idx)
	ckd_free_3d((void ***)now_den_idx);

    if (ret != S3_SUCCESS)
	E_ERROR("%s ignored\n", corpus_utt_brief_name());

    return ret;
}

int32
mmi_viterbi_run(float64 *log_forw_prob,
		vector_t **feature,
		uint32 n_obs,
		state_t *state_seq,
		uint32 n_state,
		model_inventory_t *inv,
		float64 a_beam)
{
    float64 *scale = NULL;
    float64 **dscale = NULL;
    float64 **active_alpha;
    uint32 **active_astate;
    uint32 **bp;
    uint32 *n_active_astate;
    uint32 *active_cb;
    uint32 i, j;
    int32 t;
    int ret;
    int final_state_error = 0;
    float64 log_fp;/* accumulator for the log of the probability
		    * of observing the input given the model */
    
    /* caller must ensure that there is some non-zero amount
       of work to be done here */
    assert(n_obs > 0);
    assert(n_state > 0);
    
    scale = (float64 *)ckd_calloc(n_obs, sizeof(float64));
    dscale = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    n_active_astate = (uint32 *)ckd_calloc(n_obs, sizeof(uint32));
    active_alpha  = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    active_astate = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));
    active_cb = ckd_calloc(2*n_state, sizeof(uint32));
    bp = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));

    /* Run forward algorithm, which has embedded Viterbi. */
    ret = forward(active_alpha, active_astate, n_active_astate, bp,
		  scale, dscale,
		  feature, n_obs, state_seq, n_state,
		  inv, a_beam, NULL, NULL, 1);

    if (ret != S3_SUCCESS) {

	/* Some problem with the utterance, release per utterance storage and
	 * forget about adding the utterance accumulators to the global accumulators */
	
	goto all_done;
    }

    /* Find the non-emitting ending state */
    for (i = 0; i < n_active_astate[n_obs-1]; ++i) {
	if (active_astate[n_obs-1][i] == n_state-1)
	    break;
    }
    if (i == n_active_astate[n_obs-1]) {
	/* since there are so many such errors during the mmie training,
	      it's very annoying to output this error message
	      E_ERROR("Failed to align audio to trancript: final state of the search is not reached\n"); */
	ret = S3_ERROR;
	final_state_error = 1;
	goto all_done;
    }

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

 all_done:
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
    ckd_free((void *)active_cb);
    ckd_free((void **)bp);

    if (ret != S3_SUCCESS && !final_state_error)
	E_ERROR("viterbi run error in sentence %s\n", corpus_utt_brief_name());
    
    return ret;
}

int32
mmi_viterbi_update(vector_t **feature,
		   uint32 n_obs,
		   state_t *state_seq,
		   uint32 n_state,
		   model_inventory_t *inv,
		   float64 a_beam,
		   int32 mean_reest,
		   int32 var_reest,
		   float64 arc_gamma,
		   feat_t *fcb)
{
    float64 *scale = NULL;
    float64 **dscale = NULL;
    float64 **active_alpha;
    uint32 **active_astate;
    uint32 **bp;
    uint32 *n_active_astate;
    gauden_t *g;/* Gaussian density parameters and reestimation sums */
    float32 ***mixw;/* all mixing weights */
    float64 ***now_den = NULL;/* Short for den[t] */
    uint32 ***now_den_idx = NULL;/* Short for den_idx[t] */
    uint32 *active_cb;
    uint32 n_active_cb;
    float32 ***denacc = NULL;/* mean/var reestimation accumulators for time t */
    size_t denacc_size;/* Total size of data references in denacc.  Allows
			  for quick clears between time frames */
    uint32 n_lcl_cb;
    uint32 *cb_inv;
    uint32 i, j, q;
    int32 t;
    uint32 n_feat;
    uint32 n_density;
    uint32 n_top;
    int ret;
    uint32 n_cb;

    static float64 *p_op = NULL;
    static float64 *p_ci_op = NULL;
    static float64 **d_term = NULL;
    static float64 **d_term_ci = NULL;

    /* caller must ensure that there is some non-zero amount
       of work to be done here */
    assert(n_obs > 0);
    assert(n_state > 0);

    g = inv->gauden;
    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);
    n_top = gauden_n_top(g);
    n_cb = gauden_n_mgau(g);

    if (p_op == NULL) {
	p_op    = ckd_calloc(n_feat, sizeof(float64));
	p_ci_op = ckd_calloc(n_feat, sizeof(float64));
    }

    if (d_term == NULL) {
	d_term    = (float64 **)ckd_calloc_2d(n_feat, n_top, sizeof(float64));
	d_term_ci = (float64 **)ckd_calloc_2d(n_feat, n_top, sizeof(float64));
    }

    scale = (float64 *)ckd_calloc(n_obs, sizeof(float64));
    dscale = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    n_active_astate = (uint32 *)ckd_calloc(n_obs, sizeof(uint32));
    active_alpha  = (float64 **)ckd_calloc(n_obs, sizeof(float64 *));
    active_astate = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));
    active_cb = ckd_calloc(2*n_state, sizeof(uint32));
    bp = (uint32 **)ckd_calloc(n_obs, sizeof(uint32 *));

    /* Run forward algorithm, which has embedded Viterbi. */
    ret = forward(active_alpha, active_astate, n_active_astate, bp,
		  scale, dscale,
		  feature, n_obs, state_seq, n_state,
		  inv, a_beam, NULL, NULL, 1);
    
    if (cmd_ln_str("-outphsegdir")) {
	E_FATAL("current MMI implementation don't support -outphsegdir\n");
    }


    if (ret != S3_SUCCESS) {

	/* Some problem with the utterance, release per utterance storage and
	 * forget about adding the utterance accumulators to the global accumulators */

	goto all_done;
    }

    mixw = inv->mixw;

    n_lcl_cb = inv->n_cb_inverse;
    cb_inv = inv->cb_inverse;

    /* Allocate local accumulators for mean, variance reestimation
       sums if necessary */
    gauden_alloc_l_acc(g, n_lcl_cb,
		       mean_reest, var_reest,
		       FALSE);

    n_active_cb = 0;
    now_den = (float64 ***)ckd_calloc_3d(n_lcl_cb,
					 n_feat,
					 n_top,
					 sizeof(float64));
    now_den_idx =  (uint32 ***)ckd_calloc_3d(n_lcl_cb,
					     n_feat,
					     n_top,
					     sizeof(uint32));

    if (mean_reest || var_reest) {
	/* allocate space for the per frame density counts */
	denacc = (float32 ***)ckd_calloc_3d(n_lcl_cb,
					    n_feat,
					    n_density,
					    sizeof(float32));

	/* # of bytes required to store all weighted vectors */
	denacc_size = n_lcl_cb * n_feat * n_density * sizeof(float32);
    }
    else {
	denacc = NULL;
	denacc_size = 0;
    }

    /* Okay now run through the backtrace and accumulate counts. */
    /* Find the non-emitting ending state */
    for (q = 0; q < n_active_astate[n_obs-1]; ++q) {
	if (active_astate[n_obs-1][q] == n_state-1)
	    break;
    }
    if (q == n_active_astate[n_obs-1]) {
	E_ERROR("Failed to align audio to trancript: final state of the search is not reached\n");
	ret = S3_ERROR;
	goto all_done;
    }

    for (t = n_obs-1; t >= 0; --t) {
	uint32 l_cb;
	uint32 l_ci_cb;
	float64 op, p_reest_term;
	uint32 prev;

	j = active_astate[t][q];

	/* Follow any non-emitting states at time t first. */
	while (state_seq[j].mixw == TYING_NON_EMITTING) {
	    prev = active_astate[t][bp[t][q]];
	    q = bp[t][q];
	    j = prev;
	}

	/* Now accumulate statistics for the real state. */
	l_cb = state_seq[j].l_cb;
	l_ci_cb = state_seq[j].l_ci_cb;
	n_active_cb = 0;

	gauden_compute_log(now_den[l_cb],
			   now_den_idx[l_cb],
			   feature[t],
			   g,
			   state_seq[j].cb,
			   NULL);
	active_cb[n_active_cb++] = l_cb;

	if (l_cb != l_ci_cb) {
	    gauden_compute_log(now_den[l_ci_cb],
			       now_den_idx[l_ci_cb],
			       feature[t],
			       g,
			       state_seq[j].ci_cb,
			       NULL);
	    active_cb[n_active_cb++] = l_ci_cb;
	}
	ret = gauden_scale_densities_bwd(now_den, now_den_idx,
					 &dscale[t],
					 active_cb, n_active_cb, g);
	if (ret != S3_SUCCESS)
	    goto all_done;
	
	assert(state_seq[j].mixw != TYING_NON_EMITTING);
	/* Now calculate mixture densities. */
	/* This is the normalizer sum_m c_{jm} p(o_t|\lambda_{jm}) */
	op = gauden_mixture(now_den[l_cb], now_den_idx[l_cb],
			    mixw[state_seq[j].mixw], g);

	/* Make up this bogus value to be consistent with backward.c */
	p_reest_term = 1.0 / op;

	/* Compute the output probability excluding the contribution
	 * of each feature stream.  i.e. p_op[0] is the output
	 * probability excluding feature stream 0 */
	partial_op(p_op,
		   op,
		   now_den[l_cb],
		   now_den_idx[l_cb],
		   mixw[state_seq[j].mixw],
		   n_feat,
		   n_top);

	/* compute the probability of each (of possibly topn) density */
	den_terms(d_term,
		  p_reest_term,
		  p_op,
		  now_den[l_cb],
		  now_den_idx[l_cb],
		  mixw[state_seq[j].mixw],
		  n_feat,
		  n_top);

	if (l_cb != l_ci_cb) {
	    /* For each feature stream f, compute:
	     *     sum_k(mixw[f][k] den[f][k])
	     * and store the results in p_ci_op */
	    partial_ci_op(p_ci_op,
			  now_den[l_ci_cb],
			  now_den_idx[l_ci_cb],
			  mixw[state_seq[j].ci_mixw],
			  n_feat,
			  n_top);

	    /* For each feature stream and density compute the terms:
	     *   w[f][k] den[f][k] / sum_k(w[f][k] den[f][k]) * post_j
	     * and store results in d_term_ci */
	    den_terms_ci(d_term_ci,
			 1.0, /* post_j = 1.0 */
			 p_ci_op,
			 now_den[l_ci_cb],
			 now_den_idx[l_ci_cb],
			 mixw[state_seq[j].ci_mixw],
			 n_feat,
			 n_top);
	}
	    
	/* accumulate the probability for each density in the 
	 * density reestimation accumulators */
	if (mean_reest || var_reest) {
	    accum_den_terms(denacc[l_cb], d_term,
			    now_den_idx[l_cb], n_feat, n_top);
	    if (l_cb != l_ci_cb) {
		accum_den_terms(denacc[l_ci_cb], d_term_ci,
				now_den_idx[l_ci_cb], n_feat, n_top);
	    }
	}
	
	/* Note that there is only one state/frame so this is kind of
	   redundant */
	if (mean_reest || var_reest) {
	    /* Update the mean and variance reestimation accumulators */
	    mmi_accum_gauden(denacc,
			     cb_inv,
			     n_lcl_cb,
			     feature[t],
			     now_den_idx,
			     g,
			     mean_reest,
			     var_reest,
			     arc_gamma,
			     fcb);
	    memset(&denacc[0][0][0], 0, denacc_size);
	}
	
	if (t > 0) { 
	    prev = active_astate[t-1][bp[t][q]];
	    q = bp[t][q];
	    j = prev;
	}
    }

    /* If no error was found, add the resulting utterance reestimation
     * accumulators to the global reestimation accumulators */
    accum_global(inv, state_seq, n_state,
		 FALSE, FALSE, mean_reest, var_reest,
		 FALSE);

 all_done:
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
    ckd_free((void *)active_cb);
    ckd_free((void **)bp);

    if (denacc)
	ckd_free_3d((void ***)denacc);

    if (now_den)
	ckd_free_3d((void ***)now_den);
    if (now_den_idx)
	ckd_free_3d((void ***)now_den_idx);

    if (ret != S3_SUCCESS)
	E_ERROR("viterbi update error in sentence %s\n", corpus_utt_brief_name());

    return ret;
}
