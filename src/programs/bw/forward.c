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
 * File: forward.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#define ACHK 10

#include <s3/model_inventory.h>
#include <s3/vector.h>
#include <s3/ckd_alloc.h>
#include <s3/gauden.h>
#include <s3/state.h>
#include <s3/feat.h>
#include <s3/s3.h>

#include <s3/profile.h>

#include <assert.h>
#include <math.h>

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
	float64 *scale,
	float64 **dscale,
	vector_t **feature,
	uint32 n_obs,
	state_t *state_seq,
	uint32 n_state,
	model_inventory_t *inv,
	float64 beam)
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
    float64 x;
    float64 pthresh = 1e-300;
    float64 balpha;
    float64 ***now_den;
    uint32 ***now_den_idx;
    uint32 retval = S3_SUCCESS;
    uint32 n_l_cb;
    uint32 *acbflag;
    timing_t *gau_timer = NULL;
    float64 *outprob;
    
    /* Get the CPU timer associated with mixture Gaussian evaluation */
    gau_timer = timing_get("gau");
    
    /* # of distinct codebooks referenced by this utterance */
    n_l_cb = inv->n_cb_inverse;

    /* active codebook flags */
    acbflag = ckd_calloc(n_l_cb, sizeof(uint32));

    g = inv->gauden;
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

    /* compute alpha for the initial state at t == 0 */

    if (gau_timer)
	timing_start(gau_timer);

    /* Compute the component Gaussians for state 0 mixture density */
    gauden_compute_log(now_den[state_seq[0].l_cb],
		       now_den_idx[state_seq[0].l_cb],
		       feature[0],
		       g,
		       state_seq[0].cb);

    active_l_cb[0] = state_seq[0].l_cb;

    dscale[0] = gauden_scale_densities_fwd(now_den, now_den_idx,
					   active_l_cb, 1, g);

    /* Compute the mixture density value for state 0 time 0 */
    outprob[0] = gauden_mixture(now_den[state_seq[0].l_cb],
				now_den_idx[state_seq[0].l_cb],
				mixw[state_seq[0].mixw],
				g);
    if (gau_timer)
	timing_stop(gau_timer);
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

	/* clear the active density flag array */
	for (i = 0; i < n_active_l_cb; i++) {
	    acbflag[active_l_cb[i]] = FALSE;
	}
	n_active_l_cb = 0;

	/* assume next active state set about the same size as current;
	   adjust to actual size as necessary later */
	active_alpha[t] = (float64 *)ckd_calloc(n_active, sizeof(float64));
	aalpha_alloc = n_active;

	/* for all active states */
	for (s = 0; s < n_active; s++) {
	    i = active[s];
#if FORWARD_DEBUG
	    E_INFO("At time %d, In Gaussian computation, active state %d\n",t, i);
#endif
	    /* get list of states adjacent to active state i */
	    next = state_seq[i].next_state;	

	    for (u = 0; u < state_seq[i].n_next; u++) {
		j = next[u];
#if FORWARD_DEBUG
		E_INFO("In Gaussian computation, active state %d, next state %d\n", i,j);
#endif
		if (state_seq[j].mixw != TYING_NON_EMITTING) {
		    if (amap[j] == INACTIVE) {
			l_cb = state_seq[j].l_cb;
			
			if (acbflag[l_cb] == FALSE) {
			    /* Component density values not yet computed */
			    if (gau_timer)
				timing_start(gau_timer);
			    gauden_compute_log(now_den[l_cb],
					       now_den_idx[l_cb],
					       feature[t],
					       g,
					       state_seq[j].cb);

			    active_l_cb[n_active_l_cb++] = l_cb;
			    acbflag[l_cb] = TRUE;

			    if (gau_timer)
				timing_stop(gau_timer);
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
			}
		    }
		}
	    }
	}

	/* Cope w/ numerical issues by dividing densities by max density */
	dscale[t] = gauden_scale_densities_fwd(now_den, now_den_idx,
					       active_l_cb, n_active_l_cb, g);
	
	/* for all active states */
	for (s = 0; s < n_active; s++) {
	    i = active[s];
	    
#if FORWARD_DEBUG
	    E_INFO("At time %d, In real state alpha update, active state %d\n",t, i);
#endif
	    /* get list of states adjacent to active state i */
	    next = state_seq[i].next_state;	
	    /* get the associated transition probs */
	    tprob = state_seq[i].next_tprob;

	    /* The the scaled alpha value for i at t-1 */
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

		    /* update the unscaled alpha[t][j] */
		    active_alpha[t][amap[j]] += prior_alpha * tprob[u] * outprob[j];

		}
		else {
		    /* already done below in the prior time frame */
		}
	    }
	}

	/* deal with the non-emitting states following the next active states */
	/* ARCHAN: Notice that this loop with automatically deal with null node after null node
	   Not sure whether this is the original programmar will though. 
	 */
	for (s = 0; s < n_next_active; s++) {
	    i = next_active[s];

#if FORWARD_DEBUG
	    E_INFO("At time %d, In non-emitting state update, active state %d\n",t, i);
#endif
	    /* find the prior states */
	    next = state_seq[i].next_state;
	    tprob = state_seq[i].next_tprob;

	    for (u = 0; u < state_seq[i].n_next; u++) {
		j = next[u];
#if FORWARD_DEBUG
		E_INFO("In non-emitting state update, active state %d, next state %d\n",i,j);
#endif
		if (state_seq[j].mixw == TYING_NON_EMITTING) {
		    x = active_alpha[t][s] * tprob[u];

#if FORWARD_DEBUG
		    E_INFO("In non-emitting state update, active_alpha[t][s]: %f,tprob[u]:  %f\n",active_alpha[t][s],tprob[u]);
#endif
		    if (amap[j] == INACTIVE) {
			amap[j] = n_next_active;
			active_alpha[t][n_next_active] = 0;
			next_active[n_next_active] = j;
			++n_next_active;

			if (n_next_active == aalpha_alloc) {
			    aalpha_alloc += ACHK;
			    active_alpha[t] = ckd_realloc(active_alpha[t],
							  sizeof(float64) * aalpha_alloc);
			}
		    }
		    active_alpha[t][amap[j]] += x;
		}
	    }
	}

	/* find best alpha value for pruning and scaling purposes */
	balpha = 0;
	for (s = 0; s < n_next_active; s++) {
	    if (balpha < active_alpha[t][s])
		balpha = active_alpha[t][s];
	}

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

	if (balpha < 1e-300) {
	    E_ERROR("Best alpha < 1e-300\n");

	    retval = S3_ERROR;

	    break;
	}

	/* compute the scale factor */
	scale[t] = 1.0 / balpha;

	if (log10(balpha) + log10(beam) > -300) {
	    pthresh = balpha * beam;
	}
	else {
	    pthresh = 1e-300;
	}
/* DEBUG XXXXX */
/* pthresh = 0.0; */
/* END DEBUG */

	/* Produce the scaled alpha values and active state list for
	 * unpruned alphas */
	active_astate[t] = ckd_calloc(n_next_active, sizeof(uint32));
	for (s = 0, n_active = 0; s < n_next_active; s++) {
	    if (active_alpha[t][s] > pthresh) {
		active_alpha[t][n_active] = active_alpha[t][s] * scale[t];
		active[n_active] = active_astate[t][n_active] = next_active[s];
		n_active++;
	    }
	    assert(amap[next_active[s]] != INACTIVE);
	    amap[next_active[s]] = INACTIVE; /* reset the mapping for next frame */
	}
	n_active_astate[t] = n_active;
	n_next_active = 0;

	n_sum_active += n_active;
    }
    printf(" %u ", n_sum_active / n_obs);
    
cleanup:
    ckd_free(active_a);
    ckd_free(active_b);
    ckd_free(amap);

    ckd_free(active_l_cb);
    ckd_free(acbflag);

    ckd_free(outprob);

    ckd_free_3d((void ***)now_den);
    ckd_free_3d((void ***)now_den_idx);

    return retval;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/06/17  19:17:14  arthchan2003
 * Code Update for silence deletion and standardize the name for command -line arguments
 * 
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.13  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.12  1996/07/29  16:15:18  eht
 * - gauden_compute call arguments made (hopefully) more rational
 * - change alpha and scale rep to float64 from float32
 * - smaller den and den_idx structures
 *
 * Revision 1.11  1996/03/26  13:51:08  eht
 * - Deal w/ beam float32 -> float64 bug
 * - Deal w/ case when # densities referenced in an utterance is much fewer than the
 *   total # of densities to train.
 * - Deal w/ adflag[] not set bug
 *
 * Revision 1.10  1996/03/04  15:58:56  eht
 * added more cpu time counters
 *
 * Revision 1.9  1996/02/02  17:39:53  eht
 * - Uncomment pruning.
 * - Allow a list of active Gaussians to be evaluated rather than all.
 *
 * Revision 1.8  1995/12/14  19:49:26  eht
 * - Add code to only normalize gaussian densities when 1 tied mixture is used.  O/W don't do.
 * - Add code to warn about zero output probability for emitting states.
 *
 * Revision 1.7  1995/10/12  18:30:22  eht
 * Made state.h a "local" header file
 *
 * Revision 1.6  1995/10/10  12:43:50  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.5  1995/10/09  14:55:33  eht
 * Change interface to new ckd_alloc routines
 *
 * Revision 1.4  1995/09/14  14:19:36  eht
 * reformatted slightly
 *
 * Revision 1.3  1995/08/09  20:17:41  eht
 * hard code the beam for now
 *
 * Revision 1.2  1995/07/07  11:57:54  eht
 * Changed formatting and content of verbose output and
 * make scale sum of alphas for a given time rather than
 * max alpha to make SPHINX-II comparison easier.
 *
 * Revision 1.1  1995/06/02  20:41:22  eht
 * Initial revision
 *
 *
 */
