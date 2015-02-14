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
 * File: state_seq.c
 * 
 * Description: 
 *    The routines in this file take a triphone sequence
 *    (see cvt2triphone.c) and converts it into and HMM by
 *    concatenating the triphone models.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <s3/state_seq.h>

#include <s3/remap.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <string.h>
#include <assert.h>

/*********************************************************************
 *
 * Function: state_seq_print
 * 
 * Description: 
 *    This routine prints out a given state sequence in a human
 *    readable form.
 *
 * Function Inputs: 
 *   state_t *state -
 *	This is the state sequence of interest
 *
 *   uint32 n_state -
 *	This is the number of states in the sequence
 *
 *   model_def_t *mdef -
 *	This contains the model definitions for all the component
 *	models in the system.
 *
 *   acmod_set_t *acmod_set -
 *	This contains the mapping information to map models to
 *	model ID's and vice versa.
 * 
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    None
 *
 * Global Outputs: 
 *    None
 *********************************************************************/

void
state_seq_print(state_t *state,
		uint32 n_state,
		model_def_t *mdef)
{
    unsigned int m_mixw = 0;
    unsigned int m_cb = 0;
    unsigned int l;
    unsigned int i;
    unsigned int j;
    char big_str[1024];
    char oprob_format[32];
    char non_e_format[32];
    model_def_entry_t *defn;
    uint32 n_defn;
    uint32 phn;
    acmod_set_t *acmod_set = mdef->acmod_set;
    uint32 phn_state = 0;

    defn = mdef->defn;
    n_defn = mdef->n_defn;
    
    E_INFO("**state_seq**\n");
    /* figure out the column widths by finding the max
     * length of a datum in each column. */
    for (i = 0; i < n_state; i++) {
	if (state[i].mixw != TYING_NON_EMITTING) {
	    sprintf(big_str, "%u", state[i].mixw);
	    l = strlen(big_str);

	    if (l > m_mixw) m_mixw = l;

	    sprintf(big_str, "%u", state[i].cb);
	    l = strlen(big_str);

	    if (l > m_cb) m_cb = l;
	}
    }

    /* create a printf format string for emitting states using
     * the maxes found above */
    sprintf(oprob_format, "%%%uu:%%%uu(%%3s:%%1u) %%%uu:%%%uu ",
	    m_mixw, m_mixw, m_cb, m_cb);

    /* create a printf format for non-emitting states as well */
    sprintf(non_e_format, "%%%us", m_mixw+m_cb+10);
    
    for (i = 0; i < n_state; i++) {
	printf("%3u ", i);

	if (state[i].mixw != TYING_NON_EMITTING) {
	    for (phn = 0; phn < n_defn; phn++) {
		for (phn_state = 0; phn_state < defn[phn].n_state; phn_state++) {
		    if (state[i].mixw == defn[phn].state[phn_state])
			break;
		}
		if (phn_state < defn[phn].n_state)
		    break;
	    }

	    if (phn == n_defn) {
		E_ERROR("mixw %u not found\n", state[i].mixw);
	    }

	    printf(oprob_format,
		   state[i].mixw, state[i].l_mixw,
		   acmod_set_id2name(acmod_set, phn),
		   phn_state,
		   state[i].cb, state[i].l_cb);
	}
	else {
	    printf(non_e_format, "<ne>");
	}

	printf("\n\t");


	if (state[i].n_prior == 0) {
	    printf("<no prior>");
	}
	else {
	    for (j = 0; j < state[i].n_prior; j++) {
		printf("%3u %6.3f ",
		       state[i].prior_state[j], 
		       state[i].prior_tprob[j]);
	    }
	}

	printf("\n\t");


	if (state[i].n_next == 0) {
	    printf("<no next>");
	}
	else {
	    for (j = 0; j < state[i].n_next; j++) {
		printf("%3u %6.3f ",
		       state[i].next_state[j], 
		       state[i].next_tprob[j]);
	    }
	}

	printf("\n");
    }
}

static void
set_next_state(state_t *state,
	       uint32 s,
	       uint32 *n_next,
	       uint32 *next_state,
	       float32 *next_tprob,
	       uint32 *n)
{
    uint32 i = *n;

    state[s].n_next = n_next[s];

    if (n_next[s] > 0) {
	state[s].next_tprob = &next_tprob[i];
	state[s].next_state = &next_state[i];
    }
    else
	state[s].next_tprob = NULL;

    *n = i + n_next[s];
}

static uint32
set_prior_state(state_t *state,
		uint32 s,
		uint32 *n_prior,
		uint32 *prior_state,
		float32 *prior_tprob,
		uint32 *p)
{
    uint32 i = *p;

    state[s].n_prior = n_prior[s];

    if (n_prior[s] > 0) {
	state[s].prior_state = &prior_state[i];
	state[s].prior_tprob = &prior_tprob[i];
    }
    else
	state[s].prior_tprob = NULL;

    *p = i + n_prior[s];

    return S3_SUCCESS;
}

static void
count_next_prior_states(uint32 *n_next,
			uint32 *total_next,

			uint32 *n_prior,
			uint32 *total_prior,

			acmod_id_t *phone,
			uint32 n_phone,
			model_def_entry_t *defn,

			float32 ***all_tmat)
{
    uint32 tmat_id;
    float32 **model_tmat;
    uint32 i, j, k, l;
    uint32 tp, tn;
    uint32 n_ms;

    tp = 0;
    tn = 0;

    for (i = 0, l = 0; i < n_phone; i++, l += n_ms) {

	/* first state of each model has either zero or
	   one prior states from the prior model */
	if (i > 0) {
	    ++tp;
	    ++n_prior[l];
	}

	tmat_id     = defn[phone[i]].tmat;
	n_ms        = defn[phone[i]].n_state;
	model_tmat  = all_tmat[tmat_id];

	for (j = 0; j < n_ms-1; j++) {
	    for (k = 0; k < n_ms; k++) {
		if (model_tmat[j][k] > 0.0) {
		    ++tn;
		    ++n_next[l + j];
		}
	    }
	}

	for (j = 0; j < n_ms; j++) {
	    for (k = 0; k < n_ms-1; k++) {
		if (model_tmat[k][j] > 0.0) {
		    ++tp;
		    ++n_prior[l + j];
		}
	    }
	}

	/* last state of each model has either zero or
	   one next state in the next model. */

	if (i < n_phone-1) {
	    ++tn;
	    ++n_next[l + n_ms-1];
	}

    }

    *total_prior = tp;
    *total_next = tn;
}

static void
set_next_prior_state(uint32 *next_state,
		     float32 *next_tprob,
		     uint32 *n_next,
		     uint32 total_next,

		     uint32 *prior_state,
		     float32 *prior_tprob,
		     uint32 *n_prior,
		     uint32 total_prior,

		     acmod_id_t *phone,
		     uint32 n_phone,
		     model_def_entry_t *defn,

		     float32 ***all_tmat)

{
    uint32 i;
    uint32 j;
    uint32 k;
    uint32 l;
    uint32 p, p0;
    uint32 n, n0;

    uint32 n_ms;	/* # of model states */
    uint32 tmat_id;
    float32 **model_tmat;

    for (i = 0, l = 0, p = 0, n = 0; i < n_phone; i++, l += n_ms) {
	p0 = p;

	/* 2nd through last phone have prior states to first phone state */
	if (i > 0) {

	  prior_tprob[p] = 1.0;
	  prior_state[p++] = l-1;	/* i.e. prior state of first state
					   is last state of prior model */
	}

	tmat_id     = defn[phone[i]].tmat;
	n_ms        = defn[phone[i]].n_state;
	model_tmat  = all_tmat[tmat_id];
 	
	for (j = 0; j < n_ms-1; j++) {
	    n0 = n;
	    for (k = 0; k < n_ms; k++) {
		if (model_tmat[j][k] > 0.0) {
		    next_tprob[n] = model_tmat[j][k];
		    next_state[n++] = l + k;
		}
	    }

	    assert(n_next[l+j] == (n - n0));
	}

	for (j = 0; j < n_ms; j++) {
	    if (j > 0) p0 = p;
	    for (k = 0; k < n_ms-1; k++) {
		if (model_tmat[k][j] > 0.0) {
		    prior_tprob[p] = model_tmat[k][j];
		    prior_state[p++] = l + k;
		}
	    }

#if STATE_SEQ_BUILD
	    E_INFO("n_prior[l+j] %d, p-p0 %d\n",n_prior[l+j],p-p0);
	    assert(n_prior[l+j] == (p - p0));
#endif
	}

	/* 1st through penultimate phone have next states */
	if (i < n_phone-1) {
	    next_tprob[n] = 1.0;
	    next_state[n++] = l + n_ms;	/* i.e. next state of last state is 
					   first state of next model */
	}
    }
    assert( n == total_next );
    assert( p == total_prior );

}

int
state_seq_free(state_t *s,
	       unsigned int n)
{
    uint32 i;
    int next_freed = FALSE;
    int prior_freed = FALSE;
    
    i = 0;

    /*
     * Scan for the first non-NULL next or prior state list.
     */
    do {
	if (!next_freed && s[i].n_next > 0) {
	    ckd_free(s[i].next_state);
	    ckd_free(s[i].next_tprob);

	    next_freed = TRUE;
	}

	if (!prior_freed && s[i].n_prior > 0) {
	    ckd_free(s[i].prior_state);
	    ckd_free(s[i].prior_tprob);

	    prior_freed = TRUE;
	}

	++i;
    } while (!(next_freed && prior_freed));

    ckd_free(s);

    return S3_SUCCESS;
}

state_t *
state_seq_make(uint32 *n_state,
	       acmod_id_t *phone,
	       uint32 n_phone,
	       model_inventory_t *inv,
	       model_def_t *mdef)
{
    static state_t *state;	/* The states of the sentence HMM graph */
    static uint32 *n_prior;	/* The in-degree of node i in the sent. HMM */
    static uint32 *n_next;	/* The out-degree of node i in the sent. HMM */
    static uint32 max_n_s = 0;	/* max # of states seen so far */
    uint32 n_s;			/* # of states for this sent. HMM */

    static uint32 *next_state;		/* Storage for all sent. HMM adjacency lists */
    static float32 *next_tprob;		/* Storage for all sent. HMM a_{ij} */
    static uint32 max_total_next = 0;	/* max total # of next states seen so far */
    uint32 total_next;			/* total next states for this sent. HMM */

    static uint32 *prior_state;		/* Storage for all sent. HMM adjacency lists */
    static float32 *prior_tprob;	/* Storage for all sent. HMM a_{ji} */
    static uint32 max_total_prior = 0;	/* max total # of prior states seen so far */
    uint32 total_prior;			/* total prior states for this sent. HMM */

    map_t *mixw_map;			/* Maps local (within sent. HMM) mixw id's to global ones */
    map_t *cb_map;			/* Maps local (within sent. HMM) codebook id's to global ones */
    uint32 i, j, k, p, n, u;
    uint32 n_map;
    uint32 n_ms;
    uint32 t_state;
    uint32 t_ci_state;
    float32 ***all_tmat;
    uint32 max_n_next = 0;
    uint32 tmat = NO_ID;
    model_def_entry_t *defn;
    acmod_id_t phn;
    acmod_id_t ci_phn;
    acmod_set_t *acmod_set = mdef->acmod_set;

    defn = mdef->defn;

    /* compute the total # of states */
    for (i = 0, n_s = 0; i < n_phone; i++) {
	n_s += defn[phone[i]].n_state;
    }
	

    /* Create empty local->global mappings */
    mixw_map = remap_init(4 * n_s);
    cb_map = remap_init(4 * n_s);

    if (n_s > max_n_s) {
	/* Utt is largest so far;  (Re)allocate storage appropriately */
	if (max_n_s > 0) {
	    ckd_free(state);
	    ckd_free(n_prior);
	    ckd_free(n_next);
	}
	state = ckd_calloc(n_s, sizeof(state_t));
	n_prior = ckd_calloc(n_s, sizeof(uint32));
	n_next = ckd_calloc(n_s, sizeof(uint32));

	max_n_s = n_s;
    }
    else {
	assert(max_n_s > 0);

	/* Already enough mem, just zero it */
	memset(state, 0, n_s * sizeof(state_t));
	memset(n_prior, 0, n_s * sizeof(uint32));
	memset(n_next, 0, n_s * sizeof(uint32));
    }

    all_tmat = inv->tmat;

    count_next_prior_states(n_next, &total_next,
			    n_prior, &total_prior,
			    phone, n_phone,
			    defn,
			    all_tmat);

    
    if (total_next > max_total_next) {
	/* Largest # of next states so far; (Re)allocate memory */
	if (max_total_next > 0) {
	    ckd_free(next_state);
	    ckd_free(next_tprob);
	}
	next_state = ckd_calloc(total_next, sizeof(uint32));
	next_tprob = ckd_calloc(total_next, sizeof(float32));

	max_total_next = total_next;
    }
    else {
	/* no need to realloc, just set what we need to zero */
	memset(next_state, 0, total_next * sizeof(uint32));
	memset(next_tprob, 0, total_next * sizeof(float32));
    }

    if (total_prior > max_total_prior) { 
	/* Largest # of prior states so far; (Re)allocate memory */
	if (max_total_prior > 0) {
	    ckd_free(prior_state);
	    ckd_free(prior_tprob);
	}
	prior_state = ckd_calloc(total_prior, sizeof(uint32));
	prior_tprob = ckd_calloc(total_prior,  sizeof(float32));

	max_total_prior = total_prior;
    }
    else {
	/* no need to realloc, just set what we need to zero */
	memset(prior_state, 0, total_prior * sizeof(uint32));
	memset(prior_tprob, 0, total_prior * sizeof(float32));
    }

    /* set the next/prior state id lists for each state and
       the next/prior transition probabilities */
    set_next_prior_state(next_state, next_tprob, n_next, total_next,
			 prior_state, prior_tprob, n_prior, total_prior,
			 phone, n_phone, defn,
			 all_tmat);

    /* Define the model states of the sentence HMM */
    for (i = 0, k = 0, n = 0, p = 0; i < n_phone; i++) {
	phn = phone[i];
	ci_phn = acmod_set_base_phone(acmod_set, phone[i]);
	n_ms = defn[phn].n_state;
	assert(n_ms == defn[ci_phn].n_state);

	for (j = 0; j < n_ms-1; j++, k++) {
	    tmat = defn[phn].tmat;
	    t_state = defn[phn].state[j];
	    t_ci_state = defn[ci_phn].state[j];

	    state[k].phn = phn;
	    state[k].mixw = t_state;
	    state[k].cb = mdef->cb[t_state];
	    state[k].ci_mixw = t_ci_state;
	    state[k].ci_cb = mdef->cb[t_ci_state];
	    
	    /* Set next state list and transition prob list (for forward eval) */
	    set_next_state(state, k, n_next, next_state, next_tprob, &n);

	    /* Set prior state list and transition prob list (for backward eval) */
	    set_prior_state(state, k, n_prior, prior_state, prior_tprob, &p);

	    /* Update the out-degree so far */
	    if (state[k].n_next > max_n_next)
		max_n_next = state[k].n_next;

	    state[k].tmat = tmat;
	    state[k].m_state = j;

	    /* Compute local (i.e. local to this utterance) ID's
	       for all parameter ID's (except transition matrices) */

	    state[k].l_mixw = remap(mixw_map, state[k].mixw);
	    state[k].l_cb   = remap(cb_map, state[k].cb);

	    state[k].l_ci_mixw = remap(mixw_map, state[k].ci_mixw);
	    state[k].l_ci_cb   = remap(cb_map, state[k].ci_cb);
	}

	/* last state is non-emitting so set all output probability
	 * parameter id values to NO_ID */
	state[k].mixw = 
	    state[k].ci_mixw = 
	    state[k].l_mixw = 
	    state[k].l_ci_mixw = TYING_NO_ID;

	state[k].cb =
	    state[k].ci_cb =
	    state[k].l_cb =
	    state[k].l_ci_cb = TYING_NO_ID;

	state[k].tmat = tmat;
	state[k].m_state = j;
	state[k].phn = phn;

	set_prior_state(state, k, n_prior, prior_state, prior_tprob, &p);
	set_next_state(state, k, n_next, next_state, next_tprob, &n);

	++k;
    }

    assert( k == n_s );
    assert( n == total_next );
    assert( p == total_prior );

    /* Check for transition matrix consistency */
    for (i = 0; i < n_s; i++) {
	for (u = 0; u < state[i].n_next; u++) {
	    uint32 m_i, m_j;

	    m_i = state[i].m_state;
	    j = state[i].next_state[u];
	    m_j = state[j].m_state;

	    tmat = state[i].tmat;
	    
	    if ((tmat == state[j].tmat) && (m_i <= m_j)) {
		/* Does next state transition prob agree w/ original parameters */
	      /*		assert(state[i].next_tprob[u] == all_tmat[tmat][m_i][m_j]);*/
	    }
	}
    }

    if (inv->l_mixw_acc) {
        ckd_free_3d((void ***)inv->l_mixw_acc);
	inv->l_mixw_acc = NULL;
    }

    if (inv->mixw_inverse)
	ckd_free((void *)inv->mixw_inverse);
    inv->mixw_inverse = remap_inverse(mixw_map, &n_map);
    inv->n_mixw_inverse = n_map;

    if (inv->cb_inverse)
	ckd_free((void *)inv->cb_inverse);
    inv->cb_inverse = remap_inverse(cb_map, &n_map);
    inv->n_cb_inverse = n_map;

    remap_free(mixw_map);
    remap_free(cb_map);

    /* return # of states and the state list to caller */
    *n_state = n_s;

    return state;
}
