/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: backward.c
 * 
 * Description: 
 * 	The routine in this file compute the beta variable in the
 *	forward backward algorithm.  The routine also updates the
 * 	reestimation sums for mixing weights, transition matrices,
 *	means and variances.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/model_inventory.h>
#include <s3/vector.h>
#include <s3/ckd_alloc.h>
#include <s3/state.h>
#include <s3/profile.h>
#include <s3/s2_param.h>
#include <s3/gauden.h>
#include <s3/feat.h>
#include <s3/s3.h>

#include "accum.h"

#include <assert.h>
#include <math.h>
#include <string.h>

static void
partial_op(float64 *p_op,
	   float64 op,

	   float64 **den,
	   uint32  **den_idx,
	   float32 **mixw,

	   uint32 n_feat,
	   uint32 n_top)
{
    uint32 j, k, kk;
    float64 f_op;

    for (j = 0; j < n_feat; j++) {	/* over all feature streams */

	/* Evaluate the mixture density for feature stream j
	 * gvn. component density values */
	k = den_idx[j][0];
	f_op = mixw[j][k] * den[j][0];
	for (kk = 1; kk < n_top; kk++) {
	    k = den_idx[j][kk];
	    f_op += mixw[j][k] * den[j][kk];
	}
	
	/* figure out partial output probability excluding
	 * the given feature stream j. */
	p_op[j] = op / f_op;
    }
}

static void
partial_ci_op(float64 *p_op,

	      float64 **den,
	      uint32  **den_idx,
	      float32 **mixw,

	      uint32 n_feat,
	      uint32 n_top)
{
    uint32 j, k, kk;
    float64 f_op;

    for (j = 0; j < n_feat; j++) {	/* over all feature streams */

	/* Evaluate the mixture density for feature stream j
	 * gvn. component density values */
	k = den_idx[j][0];
	f_op = mixw[j][k] * den[j][0];
	for (kk = 1; kk < n_top; kk++) {
	    k = den_idx[j][kk];
	    f_op += mixw[j][k] * den[j][kk];
	}

	p_op[j] = f_op;
    }
}

static void
den_terms_ci(float64 **d_term,

	     float64 post_j,
	     float64 *p_ci_op,
	     float64 **den,
	     uint32  **den_idx,
	     float32 **mixw,
	     uint32 n_feat,
	     uint32 n_top)
{
    uint32 j, k, kk;
    float64 inv_ci_op;

    for (j = 0; j < n_feat; j++) {
	if (p_ci_op[j] != 0) {
	    inv_ci_op = 1.0 / p_ci_op[j];

	    for (kk = 0; kk < n_top; kk++) {
		/* density index k for one of the n_top density values */
		k = den_idx[j][kk];
		
		d_term[j][kk] = mixw[j][k] * den[j][kk] * inv_ci_op * post_j;
	    }
	}
	else {
	    for (kk = 0; kk < n_top; kk++) {
		d_term[j][kk] = 0;
	    }
	}
    }
}

static void
den_terms(float64 **d_term,
	  float64 p_reest_term,
	  float64 *p_op,
	  float64 **den,
	  uint32  **den_idx,
	  float32 **mixw,
	  uint32 n_feat,
	  uint32 n_top)
{
    uint32 j, k, kk;

    for (j = 0; j < n_feat; j++) {
	for (kk = 0; kk < n_top; kk++) {
	    /* density index k for one of the n_top density values */
	    k = den_idx[j][kk];

	    d_term[j][kk] = mixw[j][k] * den[j][kk] * p_op[j] * p_reest_term;
	}
    }
}

/*********************************************************************
 *
 * Function: 
 * 	backward_update
 *
 * Description: 
 * 	The routine in this file compute the beta variable in the
 * 	forward backward algorithm.  The routine also updates the
 * 	reestimation sums for mixing weights, transition matrices,
 * 	means and variances.
 * 
 * Function Inputs: 
 *	float64 **alpha -
 *		A 2-d array containing the scaled alpha variable.
 *		alpha[t][s] is scaled alpha at time t for state s.
 *
 *	float64 *scale -
 *		The scale factor for each time frame.
 * 
 *	float64 ****den -
 *		The top N component mixture density values for
 *		all time.
 *
 *		den[t][d][f][i] addresses the the Ith density of
 *		the top N densities for acoustic feature stream f,
 *		density d at time t.
 * 
 *	uint32 ****den_idx -
 *		The top N component mixture density indices for
 *		all time.
 *
 *		den[t][d][f][i] addresses the the Ith density index
 *		of the top N densities for acoustic feature stream f,
 *		density d at time t.
 * 
 *	vector_t **feature -
 *		The feature streams for all time within the
 *		utterance.
 *
 *		feature[t][f][c] addresses component c of the feature
 *		vector for feature f at time t.
 * 
 *	uint32 n_obs -
 *		Number of observations (i.e. frames) in this observation
 *		sequence (i.e. utterance)
 * 
 *	state_t *state_seq -
 *		The sequence of sentence HMM states for the utterance.
 * 
 *	uint32 n_state -
 *		The number of states in the sentence HMM.
 * 
 *	model_inventory_t *inv -
 *		A pointer to a structure which contains references to
 *		all model parameters and reestimation sum accumulators.
 * 
 *	float64 beam -
 *		Pruning beam width.
 * 
 *	float32 spthresh -
 *		State posterior probability threshold for reestimation.
 *		State posterior prob must be greater than this value
 *		for the state to be included in the reestimation counts.
 * 
 *	int32 mixw_reest -
 *		A boolean indicating whether or not to do mixing weight
 *		reestimation.
 * 
 *	int32 tmat_reest -
 *		A boolean indicating whether or not to do transition probability matrix
 *		reestimation.
 * 
 *	int32 mean_reest -
 *		A boolean indicating whether or not to do mean
 *		reestimation.
 * 
 *	int32 var_reest
 *		A boolean indicating whether or not to do variance
 *		reestimation.
 *
 *     int32 mllr_mult
 *           A boolean indicating whether or not to compute the
 *           multiplicative term for MLLR adaptation of means
 *
 *     int32 mllr_add
 *           A boolean indicating whether or not to compute the
 *           additive term for MLLR adaptation of means
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	S3_SUCCESS -
 *		No errors found; Local accumulators updated.
 *	S3_ERROR -
 *		Error found; Ignore local accumulator values.
 * 
 * Global Outputs: 
 *	None
 * 
 * Errors: 
 * 
 *********************************************************************/

int32
backward_update(float64 **active_alpha,
		uint32 **active_astate,
		uint32 *n_active_astate,
		float64 *scale,
		float64 **dscale,
		vector_t **feature,
		uint32 n_obs,
		float32 ****spkr_xfrm_ainv,
		float32  ***spkr_xfrm_b,
		state_t *state_seq,
		uint32 n_state,
		model_inventory_t *inv,
		float64 beam,
		float32 spthresh,
		int32 mixw_reest,
		int32 tmat_reest,
		int32 mean_reest,
		int32 var_reest,
		int32 pass2var,
		int32 mllr_mult,
		int32 mllr_add)
{
    void *tt;			/* temp variable used to do
				   pointer swapping */
    uint32 i, j, s, u, q, q_f;	/* various variables of iteration */
    int32 t;			/* time */
    uint32 *active_a;	/* an active state list */
    uint32 *active_b;	/* another active state list */
    uint32 *non_emit;	/* list of active non-emitting states */
    uint32 n_non_emit;	/* the # of non-emitting states on the
				   active list */
    uint32 *tmp_non_emit;	/* list of active non-emitting states after pruning */
    uint32 n_tmp_non_emit;
    uint32 *active;	/* the active list for time t */
    uint32 n_active;	/* the # of active states */
    uint32 *active_cb;
    uint32 *next_active; /* the active list for time t-1 */
    uint32 n_next_active; /* the # of next active states */
    unsigned char *asf_a;	/* active state flag array a */
    unsigned char *asf_b;	/* active state flag array b */
    unsigned char *asf;		/* active state flag current frame */
    unsigned char *asf_next;	/* active state flag next frame */
    uint32 *prior;	/* prior state list */
    float32 *tprob;		/* transition probabilities for an (i, j) */
    float32 ***mixw;		/* all mixing weights */
    float64 pthresh;		/* pruning threshold */
    float64 pprob;		/* pruned posterior probability */
    float64 t_pprob;		/* total pruned posterior probability */
    float64 sum_alpha_beta;	/* sum of the product of all alpha and beta
				   values for some time t */
    float64 *beta_a;		/* storage for source or destination beta */
    float64 *beta_b;		/* storage for source or destination beta */
    float64 *beta;		/* source beta (set to either beta_a
				   or beta_b) */
    float64 *prior_beta;	/* destination beta (set to either beta_a
				   or beta_b) */
    gauden_t *g;		/* Gaussian density parameters and
				   reestimation sums */

    float64 ***now_den;		/* Short for den[t] */
    uint32 ***now_den_idx;	/* Short for den_idx[t] */

    float32 **tacc;		/* Transition matrix reestimation sum accumulators
				   for the utterance. */
    float32 *a_tacc;		/* The reestimation accumulator for a particular transition */

    float32 ***wacc;		/* mixing weight reestimation sum accumulators
				   for the utterance. */
    float32 ***denacc;		/* mean/var reestimation accumulators for time t */
    size_t denacc_size;		/* Total size of data references in denacc.  Allows
				   for quick clears between time frames */

    float64 recip_final_alpha;	/* Reciprocal of the final alpha value (i.e. last frame
				   final HMM state */
    int retval = S3_SUCCESS;	/* use to exit gracefully */
    uint32 n_lcl_cb;
    uint32 *cb_inv;

    float64 p_reest_term;
    float64 post_j;
    float64 sum_reest_post_j = 0.0;
    static float64 *p_op = NULL;
    static float64 *p_ci_op = NULL;
    float64 op;
    static float64 **d_term = NULL;
    static float64 **d_term_ci = NULL;

    uint32 n_feat;
    uint32 n_density;
    uint32 n_top;

    uint32 n_reest_tot = 0;
    uint32 n_active_tot = 0;

    uint32 *adflag;
    uint32 n_active_cb;

    timing_t *gau_timer = NULL;
    timing_t *rsts_timer = NULL;
    timing_t *rstf_timer = NULL;

    float64  ttt;

    uint32 max_n_next = 0;

    uint32 l_cb;
    uint32 l_ci_cb;

    uint32 n_cb;

    /* Get the Gaussian density evaluation CPU timer */
    gau_timer = timing_get("gau");

    /* Get the per state reestimation CPU timer */
    rsts_timer = timing_get("rsts");

    /* Get the per frame reestimation CPU timer */
    rstf_timer = timing_get("rstf");

    /* Look for the final state in the active states at the last frame */
    for (q_f = 0;
	 q_f < n_active_astate[n_obs-1] &&
	     active_astate[n_obs-1][q_f] != (n_state-1); q_f++);
    if (q_f == n_active_astate[n_obs-1]) {
	E_ERROR("final state not reached\n");
	return S3_ERROR;
    }

    /* Set pruning threshold for such that all unpruned paths for some
     * time t and state i satisfy alpha[t][i]beta[t][i] > beam * alpha[T][F] */
    pthresh = beam * active_alpha[n_obs-1][q_f];

    g = inv->gauden;
    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);
    n_top = gauden_n_top(g);

    if (p_op == NULL) {
	p_op    = ckd_calloc(n_feat, sizeof(float64));
	p_ci_op = ckd_calloc(n_feat, sizeof(float64));
    }

    if (d_term == NULL) {
	d_term    = (float64 **)ckd_calloc_2d(n_feat, n_top, sizeof(float64));
	d_term_ci = (float64 **)ckd_calloc_2d(n_feat, n_top, sizeof(float64));
    }

    /* Allocate space for source/destination beta */
    beta_a = ckd_calloc(n_state, sizeof(float64));
    beta_b = ckd_calloc(n_state, sizeof(float64));

    /* initialize locations for source/destination beta */
    beta = beta_a;
    prior_beta = beta_b;

    /* Allocate space for the cur/next active state lists */
    active_a = ckd_calloc(n_state, sizeof(uint32));
    active_b = ckd_calloc(n_state, sizeof(uint32));
    active_cb = ckd_calloc(2*n_state, sizeof(uint32));

    /* count up the max possible number of active non-emitting states */
    n_non_emit = 0;
    for (s = 0; s < n_state; s++)
	if (state_seq[s].mixw == TYING_NON_EMITTING)
	    n_non_emit++;
    
    /* Allocate space for the active non-emitting state lists */
    non_emit = ckd_calloc(n_non_emit, sizeof(uint32));
    tmp_non_emit = ckd_calloc(n_non_emit, sizeof(uint32));

    /* initialize locations for cur/next active state lists */
    active = active_a;
    next_active = active_b;

    n_active = 0;
    n_next_active = 0;
    
    /* Allocate space for the cur/next active state flags */
    asf_a = ckd_calloc(n_state, sizeof(unsigned char));
    asf_b = ckd_calloc(n_state, sizeof(unsigned char));

    /* Active state flags prevent states from being added to
       the active list more than once */

    /* Initialize cur/next active state lists */
    asf = asf_a;
    asf_next = asf_b;
    
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

    /* Allocate local accumulators for mean, variance or mllr
       matrix reestimation sums if necessary */
    gauden_alloc_l_acc(g, n_lcl_cb,
		       mean_reest, var_reest, mllr_mult, mllr_add);

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


    adflag = ckd_calloc(n_lcl_cb, sizeof(uint32));
    n_active_cb = 0;

    now_den = (float64 ***)ckd_calloc_3d(n_lcl_cb,
					 n_feat,
					 n_top,
					 sizeof(float64));
    now_den_idx =  (uint32 ***)ckd_calloc_3d(n_lcl_cb,
					     n_feat,
					     n_top,
					     sizeof(uint32));

    if (mean_reest || var_reest || mllr_mult || mllr_add) {
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

    recip_final_alpha = 1.0/active_alpha[n_obs-1][q_f];
    
    /* Set the initial beta value */
    prior_beta[n_state-1] = 1.0;

    /* Last state is non-emitting */
    non_emit[0] = n_state-1;
    n_non_emit = 1;

    t = n_obs-1;

    if (scale[t] == 0) {
	E_ERROR("Scale factor at time == %u is zero\n", t);

	retval = S3_ERROR;

	goto free;
    }

    /* Process non-emitting initial states first */
    for (s = 0; s < n_non_emit; s++) {
	j = non_emit[s];
	    
	prior = state_seq[j].prior_state;
	tprob = state_seq[j].prior_tprob;

	for (u = 0; u < state_seq[j].n_prior; u++) {
	    i = prior[u];
	    for (q = 0; q < n_active_astate[t] && active_astate[t][q] != i; q++);
	    if (q == n_active_astate[t]) {
		/* state i not active in forward pass; skip it */
		continue;
	    }
	    
	    /* accumulate before scaling so scale[t] doesn't appear
	     * in the reestimation sums */

	    if (tmat_reest) {
		assert(tacc != NULL);

		a_tacc = &tacc[i][j-i];
	    }
	    else {
		a_tacc = NULL;
	    }

	    if (a_tacc)
		*a_tacc +=
		    active_alpha[t][q] * tprob[u] * prior_beta[j] * recip_final_alpha;

	    assert(tprob[u] > 0);

	    prior_beta[i] += tprob[u] * prior_beta[j];

	    if (asf[i] != TRUE) {
		asf[i] = TRUE;
		active[n_active++] = i;
	    }
	}
    }

    for ( s = 0; s < n_active; s++) {
	i = active[s];
	prior_beta[i] *= scale[t];
    }

    n_non_emit = 0;
    n_tmp_non_emit = 0;
    t_pprob = 0;

    n_cb = gauden_n_mgau(g);

    for (t = n_obs-2; t >= 0; t--) {
	if (scale[t] == 0) {
	    E_ERROR("Scale factor at time == %u is zero\n", t);

	    retval = S3_ERROR;

	    goto free;
	}

	for (i = 0; i < n_active_cb; i++) {
	    adflag[active_cb[i]] = FALSE;
	}
	n_active_cb = 0;

	sum_reest_post_j = 0.0;

	/* zero beta at time t */
	memset(beta, 0, n_state * sizeof(float64));

	sum_alpha_beta = 0;

	n_active_tot += n_active;

	/* for all active emitting j states at time t+1,
	* compute the log density values */

	for (s = 0; s < n_active; s++) {
	    j = active[s];
	    l_cb = state_seq[j].l_cb;
	    l_ci_cb = state_seq[j].l_ci_cb;
	    
	    if (adflag[l_cb] == FALSE) {
		/* The top N densities for the observation
		   at time t+1 and their indices */
		if (gau_timer)
		    timing_start(gau_timer);

		gauden_compute_log(now_den[l_cb],
				   now_den_idx[l_cb],
				   feature[t+1],
				   g,
				   state_seq[j].cb);

		active_cb[n_active_cb++] = l_cb;
		adflag[l_cb] = TRUE;

		if (l_cb != l_ci_cb) {
		    if (adflag[l_ci_cb] == FALSE) {
			gauden_compute_log(now_den[l_ci_cb],
					   now_den_idx[l_ci_cb],
					   feature[t+1],
					   g,
					   state_seq[j].ci_cb);
			
			active_cb[n_active_cb++] = l_ci_cb;

			adflag[l_ci_cb] = TRUE;
		    }
		}

		if (gau_timer)
		    timing_stop(gau_timer);
	    }
	}

	/* Scale densities by dividing all by max */
	gauden_scale_densities_bwd(now_den, now_den_idx,
				   &dscale[t+1],
				   active_cb, n_active_cb, g);
	
	for (s = 0; s < n_active; s++) {
	    j = active[s];
	    l_cb = state_seq[j].l_cb;
	    l_ci_cb = state_seq[j].l_ci_cb;
	    
	    op = gauden_mixture(now_den[l_cb],
				now_den_idx[l_cb],
				mixw[state_seq[j].mixw],
				g);

	    if (gau_timer)
		timing_stop(gau_timer);

	    assert(asf[j] == TRUE);
	    assert(state_seq[j].mixw != TYING_NON_EMITTING);
	    
	    asf[j] = FALSE;

	    prior = state_seq[j].prior_state;
	    tprob = state_seq[j].prior_tprob;

	    /* for all states, i, prior to state j */
	    for (u = 0; u < state_seq[j].n_prior; u++) {
		i = prior[u];

		for (q = 0; q < n_active_astate[t] &&
			 active_astate[t][q] != i; q++);
		if (q == n_active_astate[t]) {
		    /* state i not active in forward pass; skip it */
		    continue;
		}

		/* since survived pruning, this will be true
		   for reasonable pruning thresholds */

		assert(prior_beta[j] > 0);
		
		if (rsts_timer)
		    timing_start(rsts_timer);

		p_reest_term =
		    active_alpha[t][q] *
		    (float64) tprob[u] *
		    prior_beta[j] *
		    recip_final_alpha;

		post_j = p_reest_term * op;

		if (post_j < 0) {
		    E_WARN("posterior of state %u @ time %u (== %.8e) < 0\n", j, post_j, t+1);
		    retval = S3_ERROR;

		    if (rsts_timer)
			timing_stop(rsts_timer);
		    goto free;
		}
		if (post_j > 1.0 + 1e-2) {
		    E_ERROR("posterior of state %u (== %.8e) @ time %u > 1 + 1e-2\n", j, post_j, t+1);
		    E_ERROR("alpha == %e * tprob == %e * op == %e * beta == %e * 1 / falpha == %e\n", active_alpha[t][q], tprob[u], op, prior_beta[j], recip_final_alpha);
		    
		    retval = S3_ERROR;

		    if (rsts_timer)
			timing_stop(rsts_timer);
		    goto free;
		}

		if ((post_j > MIN_POS_FLOAT32) && (post_j > spthresh)) {
		    sum_reest_post_j += post_j;

		    /* beta, alpha and outprob are non-zero,
		       so we have all we need to accumulate another
		       term for all reestimation sums */

		    ++n_reest_tot;

		    /* Update the transition matrix accumulators if necessary */
		    if (tmat_reest) {
			tacc[i][j-i] += post_j;
		    }

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
				     post_j,
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
			    if (n_cb == 1) {
				/* semi-continuous and discrete case */
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
		    if (mean_reest || var_reest || mllr_mult || mllr_add) {
			accum_den_terms(denacc[l_cb], d_term,
					now_den_idx[l_cb], n_feat, n_top);
			if (l_cb != l_ci_cb) {
			    accum_den_terms(denacc[l_ci_cb], d_term_ci,
					    now_den_idx[l_ci_cb], n_feat, n_top);
			}
		    }
		}
		
		if (rsts_timer)
		    timing_stop(rsts_timer);
		
		/* Add another term for \beta_t(i) */
		beta[i] += tprob[u] * op * prior_beta[j];
		
		if (asf_next[i] != TRUE) {
		    /* not already on the active list for time t-1 */
		    
		    asf_next[i] = TRUE;
		    
		    if (state_seq[i].mixw == TYING_NON_EMITTING) {
			non_emit[n_non_emit] = i;
			n_non_emit++;
		    }
		    else {
			next_active[n_next_active] = i;
			n_next_active++;
		    }
		}
	    }
	}

	/* Do an alpha / beta consistency check */
	for (s = 0, n_active = 0, pprob = 0; s < n_next_active; s++) {
	    i = next_active[s];

	    for (q = 0; q < n_active_astate[t] &&
		     active_astate[t][q] != i; q++);
	    if (q == n_active_astate[t]) {
		/* state i not active in forward pass; skip it */
		continue;
	    }
		
	    ttt = active_alpha[t][q] * beta[i];

	    if (ttt > pthresh) {
		sum_alpha_beta += ttt;
		active[n_active++] = i;
	    }
	    else {
		asf_next[i] = FALSE;
		pprob += ttt;
	    }
	}

	for (s = 0, n_tmp_non_emit = 0; s < n_non_emit; s++) {
	    i = non_emit[s];

	    for (q = 0; q < n_active_astate[t] &&
		     active_astate[t][q] != i; q++);
	    if (q == n_active_astate[t]) {
		/* state i not active in forward pass; skip it */
		continue;
	    }

	    ttt = active_alpha[t][q] * beta[i];

	    if (ttt > pthresh) {
		sum_alpha_beta += ttt;
		tmp_non_emit[n_tmp_non_emit++] = i;
	    }
	    else {
		asf_next[i] = FALSE;
		pprob += ttt;
	    }
	}
	n_non_emit = 0;

	pprob *= recip_final_alpha;
	t_pprob += pprob;
	
	/* check an invariant.  Theoretically,
	 * sum_alpha_beta - alpha[n_obs-1][n_state-1] must be zero, but
	 * we're dealing with finite machine word length, pruning, etc. */

	if (fabs(sum_alpha_beta - active_alpha[n_obs-1][q_f])
	    > (S2_ALPHA_BETA_EPSILON * active_alpha[n_obs-1][q_f])) {

	    E_ERROR("alpha(%e) <> sum of alphas * betas (%e) in frame %d\n",
		    active_alpha[n_obs-1][q_f], sum_alpha_beta, t);
		
	    retval = S3_ERROR;

	    goto free;
	}

	/* Update beta for all predecessors of the non-emitting
	 * states encountered above */
	for (s = 0; s < n_tmp_non_emit; s++) {
	    j = tmp_non_emit[s];
	    
	    assert(asf_next[j] == TRUE);
	    asf_next[j] = FALSE;

	    prior = state_seq[j].prior_state;
	    tprob = state_seq[j].prior_tprob;

	    for (u = 0; u < state_seq[j].n_prior; u++) {
		i = prior[u];

		for (q = 0; q < n_active_astate[t] &&
			 active_astate[t][q] != i; q++);
		if (q == n_active_astate[t]) {
		    /* state i not active in forward pass; skip it */
		    continue;
		}
		
		/* accumulate before scaling so scale[t] doesn't appear
		 * in the reestimation sums */

		if (rsts_timer)
		    timing_start(rsts_timer);
		if (tmat_reest) {
		    tacc[i][j-i] += 
			active_alpha[t][q] * tprob[u] * beta[j] * recip_final_alpha;
		}
		if (rsts_timer)
		    timing_stop(rsts_timer);

		assert(tprob[u] > 0);

		beta[i] += tprob[u] * beta[j];

		assert(state_seq[i].mixw != TYING_NON_EMITTING);

		if (asf_next[i] != TRUE) {
		    asf_next[i] = TRUE;
		    active[n_active++] = i;
		}
	    }
	}

	n_next_active = 0;
	n_tmp_non_emit = 0;
	
	/* scale the resulting betas at time t now */
	for (s = 0; s < n_active; s++) {
	    i = active[s];
	    
	    beta[i] *= scale[t];
	}

 	if (rstf_timer)
	    timing_start(rstf_timer);
	if (mean_reest || var_reest) {
	    /* Update the mean and variance reestimation accumulators */
	    accum_gauden(denacc,
			 cb_inv,
			 n_lcl_cb,
			 feature[t+1],
			 spkr_xfrm_ainv,
			 spkr_xfrm_b,
			 now_den_idx,
			 g,
			 mean_reest,
			 var_reest,
			 pass2var,
			 inv->l_mixw_acc);
	}

        if (mllr_mult || mllr_add) {
	    /* Update the MLLR matrices accumulators */
	    accum_regmat(denacc,
			 inv,
			 feature[t+1],
			 now_den_idx);
        }

	if (mean_reest || var_reest || mllr_mult || mllr_add)
	    memset(&denacc[0][0][0], 0, denacc_size);

	if (rstf_timer)
	    timing_stop(rstf_timer);
 	
	/* swap beta and prior beta */
	tt = beta;
	beta = prior_beta;
	prior_beta = tt;

	/* swap active state flags and next active state flags */
	tt = asf;
	asf = asf_next;
	asf_next = tt;
    }

    if (gau_timer)
	timing_start(gau_timer);
    gauden_compute_log(now_den[state_seq[0].l_cb],
		       now_den_idx[state_seq[0].l_cb],
		       feature[0],
		       g,
		       state_seq[0].cb);

    active_cb[0] = state_seq[0].l_cb;

    gauden_scale_densities_bwd(now_den, now_den_idx,
			       &dscale[0],
			       active_cb, 1, g);

    op = gauden_mixture(now_den[state_seq[0].l_cb],
			now_den_idx[state_seq[0].l_cb],
			mixw[state_seq[0].mixw],
			g);
    
    if (gau_timer)
	timing_stop(gau_timer);
    
    if (retval == S3_SUCCESS) {

	/* do a final alpha != beta consistency check */

	beta[0] = prior_beta[0] * op;

	if (fabs(beta[0] - active_alpha[n_obs-1][q_f])
	    > (S2_ALPHA_BETA_EPSILON * active_alpha[n_obs-1][q_f])) {
	    E_ERROR("alpha(%e) <> beta(%e)\n",
		    active_alpha[n_obs-1][q_f], beta[0]);
	    
	    retval = S3_ERROR;
	}

	if (beta[0] == 0.0) {
	    E_ERROR("beta underflow\n");

	    retval = S3_ERROR;
	}
    }

    if ((retval == S3_SUCCESS) && (asf[0] == TRUE)) {
	l_cb = state_seq[0].l_cb;
	l_ci_cb = state_seq[0].l_ci_cb;

	partial_op(p_op,
		   op,
		   now_den[l_cb],
		   now_den_idx[l_cb],
		   mixw[state_seq[0].mixw],
		   n_feat,
		   n_top);

	den_terms(d_term,
		  prior_beta[0] * recip_final_alpha,
		  p_op,
		  now_den[l_cb],
		  now_den_idx[l_cb],
		  mixw[state_seq[0].mixw],
		  n_feat,
		  n_top);

	if (state_seq[0].l_cb != state_seq[0].l_ci_cb) {
	    partial_ci_op(p_ci_op,
			  now_den[l_ci_cb],
			  now_den_idx[l_ci_cb],
			  mixw[state_seq[0].ci_mixw],
			  n_feat,
			  n_top);

	    den_terms_ci(d_term_ci,
			 1.0,	/* ASSUMPTION: 1 initial state */
			 p_ci_op,
			 now_den[l_ci_cb],
			 now_den_idx[l_ci_cb],
			 mixw[state_seq[0].ci_mixw],
			 n_feat,
			 n_top);
	}

	
	if (mixw_reest) {
	    accum_den_terms(wacc[state_seq[0].l_mixw], d_term,
			    now_den_idx[l_cb], n_feat, n_top);

	    /* check if mixw and ci_mixw are different to avoid
	     * doubling of counts in a CI run.  Will not affect
	     * final probabilities, but might affect algorithms
	     * which rely on accurate EM counts */
	    if (state_seq[0].ci_mixw != state_seq[0].mixw) {
		if (n_cb == 1) {
		    /* do the update of the CI accumulators as well */
		    accum_den_terms(wacc[state_seq[j].l_ci_mixw], d_term,
				    now_den_idx[l_cb], n_feat, n_top);
		}
		else {
		    accum_den_terms(wacc[state_seq[j].l_ci_mixw], d_term_ci,
				    now_den_idx[l_ci_cb], n_feat, n_top);
		}
	    }
	}
	
	
	if (mean_reest || var_reest || mllr_mult || mllr_add) {
	    accum_den_terms(denacc[l_cb], d_term,
			    now_den_idx[l_cb], n_feat, n_top);
	    if (l_cb != l_ci_cb) {
		accum_den_terms(denacc[l_ci_cb], d_term_ci,
				now_den_idx[l_ci_cb], n_feat, n_top);
	    }
	}
	
 	if (rstf_timer)
	    timing_start(rstf_timer);
	if (mean_reest || var_reest) {
	    /* Update the mean and variance reestimation accumulators */
	    accum_gauden(denacc,
			 cb_inv,
			 n_lcl_cb,
			 feature[0],
			 spkr_xfrm_ainv,
			 spkr_xfrm_b,
			 now_den_idx,
			 g,
			 mean_reest,
			 var_reest,
			 pass2var,
			 wacc);
	}

        if (mllr_mult || mllr_add) {
	    /* Update the MLLR matrices accumulators */
	    accum_regmat(denacc,
			 inv,
			 feature[0],
			 now_den_idx);
        }
	if (rstf_timer)
	    timing_stop(rstf_timer);
    }

    printf(" %d", n_active_tot / n_obs);
    printf(" %d", n_reest_tot / n_obs);
    printf(" %e", t_pprob / n_obs);

free:

    ckd_free(active_a);
    ckd_free(active_b);
    ckd_free(active_cb);
    ckd_free(non_emit);
    ckd_free(tmp_non_emit);
    ckd_free(asf_a);
    ckd_free(asf_b);
    ckd_free(adflag);

    if (denacc != NULL)
	ckd_free_3d((void ***)denacc);

    ckd_free(beta_a);
    ckd_free(beta_b);

    ckd_free_3d((void ***)now_den);
    ckd_free_3d((void ***)now_den_idx);

    return (retval);
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.20  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.19  1996/08/22  10:27:05  eht
 * - Pruning added based on posterior state probabilities
 * - Fix some bugs for some combinations of reest(*) flags
 * - Removed unused vars
 *
 * Revision 1.18  1996/08/06  14:03:20  eht
 * Compute and print out average active emitting beta states
 *
 * Revision 1.17  1996/07/29  16:09:47  eht
 * - Make reestimation accumulation operation (hopefully) more clear.
 * - Go to (float64) throughout alpha and beta computations
 * - Scale[t] is now pre-divided for efficiency.
 * - Test for alpha[t][j] != 0 so that beta is constrained to paths
 *   which existed in the computation of alpha[][].  Got rid of
 *   unreasonable posterior probability errors.
 * - MLLR reestimation
 *
 * Revision 1.16  1996/03/26  13:48:16  eht
 * - Fixed bbeam bug when it was defined as float32 rather than float64
 * - Deal w/ case where many fewer densities referenced by an utt than total #
 *   of densities to train.
 *
 * Revision 1.15  1996/03/04  15:59:17  eht
 * Added more cpu time counters
 *
 * Revision 1.14  1996/02/02  17:37:36  eht
 * Enable pruning again.  Need to consider better pruning in the future
 *
 * Revision 1.13  1996/01/26  18:23:49  eht
 * Deal w/ accumulating CI mixture Gaussian counts when only CD mixture Gaussians are present.
 *
 * Revision 1.12  1995/12/15  18:37:07  eht
 * Added some type cases for memory alloc/free
 *
 * Revision 1.11  1995/12/14  19:28:35  eht
 * Add another sanity check assert()
 * Add code to deal w/ not reestimating each class of parameter (tmat, mixw, mean, var).
 *
 * Revision 1.10  1995/12/01  20:54:46  eht
 * Fixed problem where gamma accumulators are freed when set
 * to NULL.
 *
 * Revision 1.9  1995/11/30  20:48:39  eht
 * Deal with non-allocation of accumulators when tmat_reest,
 * mixw_reest, mean_reest and var_reest are off.
 *
 * Revision 1.8  1995/10/18  11:18:04  eht
 * Replaced bcopy() with memset() for ANSI compatibility
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
 * Do some error checking on scale[].  Essentially, the
 * only scale factor that is not involved in some normalization
 * in the alpha pass is scale[0], but it doesn't seem to cost
 * too much to be careful.
 *
 * Revision 1.3  1995/08/30  18:33:38  eht
 * Updated comments
 *
 * Revision 1.2  1995/08/09  20:16:19  eht
 * fix some off-by-one errors
 *
 * Revision 1.1  1995/06/02  20:41:22  eht
 * Initial revision
 *
 *
 */
