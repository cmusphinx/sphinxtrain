/* -*- c-basic-offset: 4 -*- */
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
 * File: forward.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#define ACHK 10

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/profile.h>

#include <s3/model_inventory.h>
#include <s3/s3phseg_io.h>
#include <s3/vector.h>
#include <s3/gauden.h>
#include <s3/state.h>
#include <s3/s3.h>

#include <assert.h>
#include <math.h>
#include <string.h>

#include "baum_welch.h"

#define FORWARD_DEBUG 0
#define INACTIVE	0xffff

/*********************************************************************
 *
 * Function: 
 * 	forward
 *
 * Description: 
 * 	This function computes the scaled forward variable, alpha.
 *
 *	In order to conserve memory for long utterances, only the
 *	active (i.e. non-zero) alpha values are stored.
 *
 * Function Inputs: 
 * 	float64 **active_alpha -
 *		On the successful return of this function,
 *		this array contains the scaled alpha variable for
 *		active states for all input observations.  For any
 *		input observation at time t (t >= 0  && t < n_obs), the number
 *		of active states can be found by accessing
 *		n_active_astate[t].  So for some time t, the active
 *		scaled alpha values are active_alpha[t][i] where
 *		i >= 0 and i < n_active_astate[t].  For some valid t and
 *		i, the sentence HMM state id can be found by accessing
 *		active_astate[t][i].
 *
 * 	uint32 **active_astate -
 *		On the successful return of this function,
 *		this array contains the mapping of active state indexes
 *		into sentence HMM indexes (i.e. indexes into the state_seq[]
 *		array).  The active states of the sentence HMM may be
 *		enumerated by accessing active_astate[t][i] over all
 *		t (t >= 0 && t < n_obs) and i
 *		(i >= 0 && i < n_active_astate[t]).
 *
 * 	uint32 *n_active_astate -
 *		On the successful return of this function,
 *		this array contains the number of active states for all
 *		t (t >= 0 && t < n_obs).
 *
 *	float64 *scale -
 *		On the successful return of this function,
 * 		this variable contains the scale factor applied to the
 *		alpha variable for all t (t >= 0 && t < n_obs).
 *
 *	vector_t **feature -
 *		This variable contains the input observation vectors.
 *		The value feature[t][f] must be valid for
 *		t >= 0 && t < n_obs and f >= 0 && f < n_feat) where
 *		n_feat is the # of assumed statistically independent
 *		feature streams to be modelled.
 *
 * 	uint32 **bp -
 *		On the successful return of this function,
 *		this array contains backtrace pointers for active states
 *              for all input observations except the first timepoint.
 *              As in active_alpha[], the sentence HMM state id can be
 *              found by accessing active_astate[t][i].  NOTE!
 *              This is a "raw" backpointer array and as such, it contains
 *              pointers to non-emitting states.  These pointers refer to
 *              the current frame rather than the previous one.  Thus,
 *              the state id of the backpointer is either
 *              active_astate[t-1][bp[t][i]] (for normal states) or
 *              active_astate[t][bp[t][i]] (for non-emitting states).
 *
 *	uint32 n_obs -
 *		This variable contains the number of input observation
 *		vectors seen given the model.
 *
 *	state_t *state_seq -
 *		This is a list of model state structures which define
 *		the sentence HMM for this observation sequence.
 *
 *	uint32 n_state -
 *		The total # of states in the sentence HMM for this
 *		utterance.
 *
 *	model_inventory_t *inv -
 *		This structure contains the inventory of initial acoustic
 *		model parameters.
 *
 *	float64 beam -
 *		A pruning beam to apply to the evaluation of the alpha
 *		variable.
 *
 *      s3phseg_t *phseg -
 *              An optional phone segmentation to use to constrain the
 *              forward lattice.
 *
 * Global Inputs: 
 * 	None
 *
 * Return Values: 
 *
 * 	S3_SUCCESS
 *		The alpha variable was completed successfully for this
 *		observation sequence and model.
 *
 *	S3_ERROR
 *		Some error was detected that prevented the computation of the
 *		variable.
 *
 * Global Outputs: 
 * 	None
 *
 * Errors: 
 * 	- Initial alpha value < epsilon
 *	- If semi-continuous models, unable to normalize input frame
 *	- Output liklihood underflow
 *	- Alpha variable < epsilon for all active states
 *
 *********************************************************************/

int32
forward(float64 **active_alpha,
	uint32 **active_astate,
	uint32 *n_active_astate,
	uint32 **bp,
	float64 *scale,
	float64 **dscale,
	vector_t **feature,
	uint32 n_obs,
	state_t *state_seq,
	uint32 n_state,
	model_inventory_t *inv,
	float64 beam,
	s3phseg_t *phseg,
	bw_timers_t *timers,
	uint32 mmi_train)
{
    uint32 i, j, s, t, u;
    uint32 l_cb;
    uint32 *active_a;
    uint32 *active_b;
    uint32 *active;
    uint32 *active_l_cb;
    uint32 n_active;
    uint32 n_active_l_cb;
    uint32 n_sum_active;
    uint32 *next_active;
    uint32 n_next_active;
    uint32 aalpha_alloc;
    uint16 *amap;
    uint32 *next;
    float32 *tprob;
    float64 prior_alpha;
    float32 ***mixw;
    gauden_t *g;
    acmod_set_t *as;
    float64 x;
    float64 pthresh = 1e-300;
    float64 balpha;
    float64 ***now_den;
    uint32 ***now_den_idx;
    uint32 retval = S3_SUCCESS;
    uint32 n_l_cb;
    int32 *acbframe; /* Frame in which a codebook was last active */
    float64 *outprob;
    /* Can we prune this frame using phseg? */
    int can_prune_phseg;
    float64 *best_pred = NULL;
    
    /* # of distinct codebooks referenced by this utterance */
    n_l_cb = inv->n_cb_inverse;

    /* active codebook frame index */
    acbframe = ckd_calloc(n_l_cb, sizeof(*acbframe));

    g = inv->gauden;
    as = inv->mdef->acmod_set;
    /* density values and indices (for top-N eval) for some time t */
    now_den = (float64 ***)ckd_calloc_3d(n_l_cb, gauden_n_feat(g), gauden_n_top(g),
					 sizeof(float64));
    now_den_idx = (uint32 ***)ckd_calloc_3d(n_l_cb, gauden_n_feat(g), gauden_n_top(g),
					    sizeof(uint32));
    /* Mixing weight array */
    mixw = inv->mixw;

    /* Scratch area for output probabilities at some time t */
    outprob = (float64 *)ckd_calloc(n_state, sizeof(float64));

    /* Active state lists for time t and t+1 */
    active_a = ckd_calloc(n_state, sizeof(uint32));
    active_b = ckd_calloc(n_state, sizeof(uint32));

    /* Active (local) codebooks for some time t */
    active_l_cb = ckd_calloc(n_state, sizeof(uint32));

    /* Mapping from sentence HMM state index to active state list index
    * for currently active time. */
    amap = ckd_calloc(n_state, sizeof(uint16));
    
    /* set up the active and next_active lists and associated counts */
    active = active_a;
    next_active = active_b;
    n_active = 0;
    n_active_l_cb = 0;
    n_sum_active = 0;
    n_next_active = 0;

    /* Initialize the active state map such that all states are inactive */
    for (i = 0; i < n_state; i++)
	amap[i] = INACTIVE;

    /*
     * The following section computes the output liklihood of
     * the initial state for t == 0 and puts the initial state
     * in the active state list.
     */

    if (timers)
	ptmr_start(&timers->gau_timer);

    /* compute alpha for the initial state at t == 0 */
    /* Compute the component Gaussians for state 0 mixture density */
    gauden_compute_log(now_den[state_seq[0].l_cb],
		       now_den_idx[state_seq[0].l_cb],
		       feature[0],
		       g,
		       state_seq[0].cb, NULL);

    active_l_cb[0] = state_seq[0].l_cb;

    dscale[0] = gauden_scale_densities_fwd(now_den, now_den_idx,
					   active_l_cb, 1, g);

    /* Compute the mixture density value for state 0 time 0 */
    outprob[0] = gauden_mixture(now_den[state_seq[0].l_cb],
				now_den_idx[state_seq[0].l_cb],
				mixw[state_seq[0].mixw],
				g);
    if (timers)
	ptmr_stop(&timers->gau_timer);
    if (outprob[0] <= MIN_IEEE_NORM_POS_FLOAT32) {
	E_ERROR("Small output prob (== %.2e) seen at frame 0 state 0\n", outprob[0]);

	retval = S3_ERROR;
	
	goto cleanup;
    }

    /*
     * Allocate space for the initial state in the alpha
     * and active state arrays
     */
    active_alpha[0] = ckd_calloc(1, sizeof(float64));
    active_astate[0] = ckd_calloc(1, sizeof(uint32));
    if (bp)
	bp[0] = ckd_calloc(1, sizeof(uint32)); /* Unused, actually */
    aalpha_alloc = 1;

    /*
     * Allocate the bestscore array for embedded Viterbi
     */
    if (bp)
	best_pred = ckd_calloc(1, sizeof(float64));

    /* Compute scale for t == 0 */
    scale[0] = 1.0 / outprob[0];

    /* set the scaled alpha variable for the initial state */
    active_alpha[0][0] = 1.0;
    /* put the initial state in the active state array for t == 0 */
    active_astate[0][0] = 0;
    /* Only one initial state (for now) */
    n_active_astate[0] = 1;

    /* insert the initial state in the active list */
    active[n_active] = 0;
    n_active++;

    /* Compute scaled alpha over all remaining time in the utterance */
    for (t = 1; t < n_obs; t++) {
	/* Find active phone for this timepoint. */
	if (phseg) {
	    /* Move the pointer forward if necessary. */
	    if (t > phseg->ef)
		phseg = phseg->next;
	}
	n_active_l_cb = 0;

	/* assume next active state set about the same size as current;
	   adjust to actual size as necessary later */
	active_alpha[t] = (float64 *)ckd_calloc(n_active, sizeof(float64));
	if (bp) {
	    bp[t] = (uint32 *)ckd_calloc(n_active, sizeof(uint32));
	    /* reallocate the best score array and zero it out */
	    if (n_active > aalpha_alloc)
		best_pred = (float64 *)ckd_realloc(best_pred, n_active * sizeof(float64));
	    memset(best_pred, 0, n_active * sizeof(float64));
	}
	aalpha_alloc = n_active;

	/* For all active states at the previous frame, activate their
	   successors in this frame and compute codebooks. */
	/* (these are pre-computed so they can be scaled to avoid underflows) */
	for (s = 0; s < n_active; s++) {
	    i = active[s];
#if FORWARD_DEBUG
	    E_INFO("At time %d, In Gaussian computation, active state %d\n",t, i);
#endif
	    /* get list of states adjacent to active state i */
	    next = state_seq[i].next_state;	

	    /* activate them all, computing their codebook densities if necessary */
	    for (u = 0; u < state_seq[i].n_next; u++) {
		j = next[u];
#if FORWARD_DEBUG
		E_INFO("In Gaussian computation, active state %d, next state %d\n", i,j);
#endif
		if (state_seq[j].mixw != TYING_NON_EMITTING) {
		    if (amap[j] == INACTIVE) {
			l_cb = state_seq[j].l_cb;
			
			if (acbframe[l_cb] != t) {
			    /* Component density values not yet computed */
			    if (timers)
				ptmr_start(&timers->gau_timer);
			    gauden_compute_log(now_den[l_cb],
					       now_den_idx[l_cb],
					       feature[t],
					       g,
					       state_seq[j].cb,
					       /* Preinitializing topn
						  only really makes a
						  difference for
						  semi-continuous
						  (n_l_cb == 1)
						  models. */
					       n_l_cb == 1
					       ? now_den_idx[l_cb] : NULL);

			    active_l_cb[n_active_l_cb++] = l_cb;
			    acbframe[l_cb] = t;

			    if (timers)
				ptmr_stop(&timers->gau_timer);
			}

			/* Put next state j into the active list */
			amap[j] = n_next_active;

			/* Initialize the alpha variable to zero */
			active_alpha[t][n_next_active] = 0;

			/* Map active state list index to sentence HMM index */
			next_active[n_next_active] = j;

			++n_next_active;

			if (n_next_active == aalpha_alloc) {
			    /* Need to reallocate the active_alpha array */
			    aalpha_alloc += ACHK;
			    active_alpha[t] = ckd_realloc(active_alpha[t],
							  sizeof(float64) * aalpha_alloc);
			    /* And the backpointer array */
			    if (bp) {
				bp[t] = ckd_realloc(bp[t],
						    sizeof(uint32) * aalpha_alloc);
				/* And the best score array */
				best_pred = (float64 *)ckd_realloc(best_pred,
								   sizeof(float64) * aalpha_alloc);
				/* Make sure the new stuff is zero */
				memset(bp[t] + aalpha_alloc - ACHK,
				       0, sizeof(uint32) * ACHK);
				memset(best_pred + aalpha_alloc - ACHK,
				       0, sizeof(float64) * ACHK);
			    }
			}
		    }
		}
	    }
	}

	/* Cope w/ numerical issues by dividing densities by max density */
	dscale[t] = gauden_scale_densities_fwd(now_den, now_den_idx,
					       active_l_cb, n_active_l_cb, g);
	
	/* Now, for all active states in the previous frame, compute
	   alpha for all successors in this frame. */
	for (s = 0; s < n_active; s++) {
	    i = active[s];
	    
#if FORWARD_DEBUG
	    E_INFO("At time %d, In real state alpha update, active state %d\n",t, i);
#endif
	    /* get list of states adjacent to active state i */
	    next = state_seq[i].next_state;	
	    /* get the associated transition probs */
	    tprob = state_seq[i].next_tprob;

	    /* the scaled alpha value for i at t-1 */
	    prior_alpha = active_alpha[t-1][s];

	    /* For all emitting states j adjacent to i, update their
	     * alpha values.  */
	    for (u = 0; u < state_seq[i].n_next; u++) {
		j = next[u];
#if FORWARD_DEBUG
		E_INFO("In real state update, active state %d, next state %d\n", i,j);
#endif
		l_cb = state_seq[j].l_cb;

		if (state_seq[j].mixw != TYING_NON_EMITTING) {
		    /* Next state j is an emitting state */
		    outprob[j] = gauden_mixture(now_den[l_cb],
						now_den_idx[l_cb],
						mixw[state_seq[j].mixw],
						g);


		    /* update backpointers bp[t][j] */
		    x = prior_alpha * tprob[u];
		    if (bp) {
			if (x > best_pred[amap[j]]) {
#if FORWARD_DEBUG
			    E_INFO("In real state update, backpointer %d => %d updated from %e to (%e * %e = %e)\n",
				   i, j, best_pred[amap[j]], prior_alpha, tprob[u], x);
#endif
			    best_pred[amap[j]] = x;
			    bp[t][amap[j]] = s;
			}
		    }
		    
		    /* update the unscaled alpha[t][j] */
		    active_alpha[t][amap[j]] += x * outprob[j];
		}
		else {
		    /* already done below in the prior time frame */
		}
	    }
	}

#if FORWARD_DEBUG
	if (bp) {
	    for (s = 0; s < n_next_active; ++s) {
		j = next_active[s];
		E_INFO("After real state update, best path to %d(%d) = %d(%d)\n",
		       j, amap[j], active[bp[t][s]], bp[t][s]);
	    }
	}
#endif
	/* Now, for all active states in this frame, consume any
	   following non-emitting states (multiplying in their
	   transition probabilities)  */
	for (s = 0; s < n_next_active; s++) {
	    i = next_active[s];

	    /* find the successor states */
	    next = state_seq[i].next_state;
	    tprob = state_seq[i].next_tprob;

	    for (u = 0; u < state_seq[i].n_next; u++) {
		j = next[u];
		/* for any non-emitting ones */
		if (state_seq[j].mixw == TYING_NON_EMITTING) {
#if FORWARD_DEBUG
		    E_INFO("In non-emitting state update, active state %d, next state %d\n",i,j);
#endif
		    x = active_alpha[t][s] * tprob[u];

#if FORWARD_DEBUG
		    E_INFO("In non-emitting state update, active_alpha[t][s]: %f,tprob[u]:  %f\n",active_alpha[t][s],tprob[u]);
#endif
		    /* activate this state if necessary */
		    if (amap[j] == INACTIVE) {
			amap[j] = n_next_active;
			active_alpha[t][n_next_active] = 0;
			next_active[n_next_active] = j;
			++n_next_active;

			if (n_next_active == aalpha_alloc) {
			    aalpha_alloc += ACHK;
			    active_alpha[t] = ckd_realloc(active_alpha[t],
							  sizeof(float64) * aalpha_alloc);
			    if (bp) {
				bp[t] = ckd_realloc(bp[t],
						    sizeof(uint32) * aalpha_alloc);
				best_pred = (float64 *)ckd_realloc(best_pred,
								   sizeof(float64) * aalpha_alloc);
				memset(bp[t] + aalpha_alloc - ACHK,
				       0, sizeof(uint32) * ACHK);
				memset(best_pred + aalpha_alloc - ACHK,
				       0, sizeof(float64) * ACHK);
			    }
			}
			if (bp) {
			    /* Give its backpointer a default value */
			    bp[t][amap[j]] = s;
			    best_pred[amap[j]] = x;
			}
		    }

		    /* update backpointers bp[t][j] */
		    if (bp && x > best_pred[amap[j]]) {
			bp[t][amap[j]] = s;
			best_pred[amap[j]] = x;
		    }
		    /* update its alpha value */
		    active_alpha[t][amap[j]] += x;
		}
	    }
	}

#if FORWARD_DEBUG
	for (s = 0; s < n_next_active; ++s) {
	    j = next_active[s];
	    if (bp && state_seq[j].mixw == TYING_NON_EMITTING) {
		E_INFO("After non-emitting state update, best path to %d(%d) = %d(%d)\n",
		       j, amap[j], next_active[bp[t][s]], bp[t][s]);
		/* Assumptions about topology that might not be valid
		 * but are useful for debugging. */
		assert(next_active[bp[t][s]] <= j);
		assert(j - next_active[bp[t][s]] <= 2);
	    }
	}
#endif
	/* find best alpha value in current frame for pruning and scaling purposes */
	balpha = 0;
	/* also take the argmax to find the best backtrace */
	for (s = 0; s < n_next_active; s++) {
	    if (balpha < active_alpha[t][s]) {
		balpha = active_alpha[t][s];
	    }
	}

	/* cope with some pathological case */
	if (balpha == 0.0 && n_next_active > 0) {
	    E_ERROR("All %u active states,", n_next_active);
	    for (s = 0; s < n_next_active; s++) {
		if (state_seq[next_active[s]].mixw != TYING_NON_EMITTING)
		    fprintf(stderr, " %u", state_seq[next_active[s]].mixw);
		else
		    fprintf(stderr, " N(%u,%u)",
			    state_seq[next_active[s]].tmat, state_seq[next_active[s]].m_state);

	    }
	    fprintf(stderr, ", zero at time %u\n", t);
	    fflush(stderr);
	    retval =  S3_ERROR;
	    break;
	}

	/* and some related pathological cases */
	if (balpha < 1e-300) {
	    E_ERROR("Best alpha < 1e-300\n");

	    retval = S3_ERROR;

	    break;
	}
	if (n_next_active == 0) {
	    E_ERROR("No active states at time %u\n", t);
	    retval = S3_ERROR;
	    break;
	}

	/* compute the scale factor */
	scale[t] = 1.0 / balpha;
	/* compute the pruning threshold based on the beam */
	if (log10(balpha) + log10(beam) > -300) {
	    pthresh = balpha * beam;
	}
	else {
	    /* avoiding underflow... */
	    pthresh = 1e-300;
	}
/* DEBUG XXXXX */
/* pthresh = 0.0; */
/* END DEBUG */

	/* Determine if phone segmentation-based pruning would leave
	 * us with an empty active list (that would be bad!) */
	can_prune_phseg = 0;
	if (phseg) {
	    for (s = 0; s < n_next_active; ++s) 
		if (acmod_set_base_phone(as, state_seq[next_active[s]].phn)
		    == acmod_set_base_phone(as, phseg->phone))
		    break;
	    can_prune_phseg = !(s == n_next_active);
#if FORWARD_DEBUG
	    if (!can_prune_phseg) {
		E_INFO("Will not apply phone-based pruning at timepoint %d "
		       "(%d != %d) (%s != %s)\n", t,
		       state_seq[next_active[s]].phn,
		       phseg->phone,
		       acmod_set_id2name(inv->mdef->acmod_set, state_seq[next_active[s]].phn),
		       acmod_set_id2name(inv->mdef->acmod_set, phseg->phone)
		       );
	    }
#endif
	}
	/* Prune active states for the next frame and rescale their alphas. */
	active_astate[t] = ckd_calloc(n_next_active, sizeof(uint32));
	for (s = 0, n_active = 0; s < n_next_active; s++) {
	    /* "Snap" the backpointers for non-emitting states, so
	       that they don't point to bogus indices (we will use
	       amap to recover them). */
	    if (bp && state_seq[next_active[s]].mixw == TYING_NON_EMITTING) {
#if FORWARD_DEBUG
		E_INFO("Snapping backpointer for %d, %d => %d\n",
		       next_active[s], bp[t][s], next_active[bp[t][s]]);
#endif
		bp[t][s] = next_active[bp[t][s]];
	    }
	    /* If we have a phone segmentation, use it instead of the beam. */
	    if (phseg && can_prune_phseg) {
		if (acmod_set_base_phone(as, state_seq[next_active[s]].phn)
		    == acmod_set_base_phone(as, phseg->phone)) {
		    active_alpha[t][n_active] = active_alpha[t][s] * scale[t];
		    active[n_active] = active_astate[t][n_active] = next_active[s];
		    if (bp)
			bp[t][n_active] = bp[t][s];
		    amap[next_active[s]] = n_active;
		    n_active++;
		}
		else {
		    amap[next_active[s]] = INACTIVE;
		}
	    }
	    else {
		if (active_alpha[t][s] > pthresh) {
		    active_alpha[t][n_active] = active_alpha[t][s] * scale[t];
		    active[n_active] = active_astate[t][n_active] = next_active[s];
		    if (bp)
			bp[t][n_active] = bp[t][s];
		    amap[next_active[s]] = n_active;
		    n_active++;
		}
		else {
		    amap[next_active[s]] = INACTIVE;
		}
	    }
	}
	/* Now recover the backpointers for non-emitting states. */
	for (s = 0; s < n_active; ++s) {
	    if (bp && state_seq[active[s]].mixw == TYING_NON_EMITTING) {
#if FORWARD_DEBUG
		E_INFO("Snapping backpointer for %d, %d => %d(%d)\n",
		       active[s], bp[t][s], amap[bp[t][s]], active[amap[bp[t][s]]]);
#endif
		bp[t][s] = amap[bp[t][s]];
	    }
	}
	/* And finally deactive all states. */
	for (s = 0; s < n_active; ++s) {
	    amap[active[s]] = INACTIVE;
	}
	n_active_astate[t] = n_active;
	n_next_active = 0;

	n_sum_active += n_active;
    }
    if (!mmi_train)
	printf(" %u ", n_sum_active / n_obs);
    
cleanup:
    ckd_free(active_a);
    ckd_free(active_b);
    ckd_free(amap);

    ckd_free(active_l_cb);
    ckd_free(acbframe);

    ckd_free(outprob);
    ckd_free(best_pred);

    ckd_free_3d((void ***)now_den);
    ckd_free_3d((void ***)now_den_idx);

    return retval;
}
