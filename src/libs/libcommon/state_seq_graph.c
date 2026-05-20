/**
 * @file state_seq_graph.c
 * @brief Graph-aware utterance HMM builder. See state_seq_graph.h.
 *
 * Architecture mirrors state_seq.c's three-pass design:
 *
 *   1. Counter: compute n_next[s] and n_prior[s] for every state s.
 *      Cross-phone arcs are read from the phone graph's adjacency
 *      lists instead of assumed to be "i -> i+1".
 *   2. Allocate the packed next_state[] / prior_state[] arrays.
 *   3. Writer: fill the packed arrays. Crucially, the write order
 *      must match the read order of the assembly pass (state index 0,
 *      1, 2, ..., n_s - 1), since each set_next_state / set_prior_state
 *      call slices off the next n_next[k] / n_prior[k] entries.
 *   4. Assembly: build the state_t array using state_seq_set_next /
 *      state_seq_set_prior, the same helpers state_seq.c uses for
 *      the linear path.
 *
 * Each phone slot in the graph occupies a contiguous block of states:
 *   state_offset[i] = sum over earlier slots of n_state(phone[j])
 *
 * For a linear graph (single-pron), the resulting state_t array is
 * bit-identical to state_seq_make()'s on the equivalent phone[] list:
 * outgoing arcs for non-emit exits have count 1 with tprob 1.0
 * targeting the first state of the next phone — same as i+1.
 *
 * For wide graphs, the non-emit exit of slot i fans out to the first
 * states of all graph successors, with uniform 1/n_next[i] tprob per
 * arc.
 */

#include <s3/state_seq_graph.h>

#include <s3/model_def_io.h>
#include <s3/remap.h>
#include <s3/s3.h>
#include <s3/state_seq.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <assert.h>
#include <string.h>

#include "state_seq_internal.h"

/* ----- Counter ----- */

static void
count_arcs(const phone_graph_t *graph,
           const uint32 *state_offset,
           const model_def_entry_t *defn,
           float32 ***all_tmat,
           uint32 *n_next,
           uint32 *total_next,
           uint32 *n_prior,
           uint32 *total_prior)
{
    uint32 tn = 0, tp = 0;
    uint32 i, j, k;

    for (i = 0; i < graph->n; i++) {
	uint32 l = state_offset[i];
	uint32 tmat_id = defn[graph->phone[i]].tmat;
	uint32 n_ms   = defn[graph->phone[i]].n_state;
	float32 **mt  = all_tmat[tmat_id];

	/* Intra-phone forward arcs. */
	for (j = 0; j < n_ms - 1; j++) {
	    for (k = 0; k < n_ms; k++) {
		if (mt[j][k] > 0.0f) {
		    ++tn;
		    ++n_next[l + j];
		}
	    }
	}

	/* Intra-phone backward arcs. */
	for (j = 0; j < n_ms; j++) {
	    for (k = 0; k < n_ms - 1; k++) {
		if (mt[k][j] > 0.0f) {
		    ++tp;
		    ++n_prior[l + j];
		}
	    }
	}

	/* Cross-phone arcs: the non-emit exit fans out to the first
	 * states of every graph successor (and the first state has
	 * one incoming arc from every graph predecessor's exit). */
	n_next[l + n_ms - 1] += graph->n_next[i];
	tn += graph->n_next[i];

	n_prior[l] += graph->n_prior[i];
	tp += graph->n_prior[i];
    }

    *total_next = tn;
    *total_prior = tp;
}

/* ----- Writer ----- */

static void
write_arcs(const phone_graph_t *graph,
           const uint32 *state_offset,
           const model_def_entry_t *defn,
           float32 ***all_tmat,
           uint32 *next_state,
           float32 *next_tprob,
           const uint32 *n_next,
           uint32 total_next,
           uint32 *prior_state,
           float32 *prior_tprob,
           const uint32 *n_prior,
           uint32 total_prior)
{
    uint32 i, j, k, u;
    uint32 n = 0, p = 0;

    for (i = 0; i < graph->n; i++) {
	uint32 l = state_offset[i];
	uint32 tmat_id = defn[graph->phone[i]].tmat;
	uint32 n_ms   = defn[graph->phone[i]].n_state;
	float32 **mt  = all_tmat[tmat_id];

	/* Cross-phone incoming arc(s) to first state. Each
	 * predecessor's outgoing weight to this slot is
	 * 1 / n_next[predecessor]. */
	for (u = 0; u < graph->n_prior[i]; u++) {
	    uint32 pred_slot = graph->prior_idx[i][u];
	    uint32 pred_n_ms = defn[graph->phone[pred_slot]].n_state;
	    uint32 pred_exit = state_offset[pred_slot] + pred_n_ms - 1;
	    float32 weight = 1.0f / (float32) graph->n_next[pred_slot];
	    prior_tprob[p] = weight;
	    prior_state[p++] = pred_exit;
	}

	/* Intra-phone forward arcs from each emit state. */
	for (j = 0; j < n_ms - 1; j++) {
	    uint32 n0 = n;
	    for (k = 0; k < n_ms; k++) {
		if (mt[j][k] > 0.0f) {
		    next_tprob[n] = mt[j][k];
		    next_state[n++] = l + k;
		}
	    }
	    assert(n_next[l + j] == (n - n0));
	    (void) n0;
	}

	/* Intra-phone backward arcs to each state. */
	for (j = 0; j < n_ms; j++) {
	    for (k = 0; k < n_ms - 1; k++) {
		if (mt[k][j] > 0.0f) {
		    prior_tprob[p] = mt[k][j];
		    prior_state[p++] = l + k;
		}
	    }
	}

	/* Cross-phone outgoing arcs from non-emit exit to each
	 * graph successor's first state, uniform 1/n_next[i]. */
	if (graph->n_next[i] > 0) {
	    float32 weight = 1.0f / (float32) graph->n_next[i];
	    for (u = 0; u < graph->n_next[i]; u++) {
		uint32 succ_slot = graph->next_idx[i][u];
		next_tprob[n] = weight;
		next_state[n++] = state_offset[succ_slot];
	    }
	}
    }

    assert(n == total_next);
    assert(p == total_prior);
    (void) total_next;
    (void) total_prior;
}

/* ----- Top-level builder ----- */

state_t *
state_seq_make_graph(uint32 *n_state,
                     const phone_graph_t *graph,
                     model_inventory_t *inv,
                     model_def_t *mdef)
{
    state_t *state = NULL;
    uint32 *n_prior = NULL;
    uint32 *n_next = NULL;
    uint32 *next_state = NULL;
    float32 *next_tprob = NULL;
    uint32 *prior_state = NULL;
    float32 *prior_tprob = NULL;
    uint32 *state_offset = NULL;
    map_t *mixw_map = NULL;
    map_t *cb_map = NULL;
    uint32 i, j, k;
    uint32 n_s = 0;
    uint32 total_next = 0;
    uint32 total_prior = 0;
    uint32 n, p;
    model_def_entry_t *defn = mdef->defn;
    acmod_set_t *acmod_set = mdef->acmod_set;
    float32 ***all_tmat = inv->tmat;
    uint32 n_map;

    if (graph == NULL || graph->n == 0) {
	return NULL;
    }

    /* Compute total state count and per-slot state offsets. */
    state_offset = ckd_calloc(graph->n, sizeof(uint32));
    for (i = 0; i < graph->n; i++) {
	state_offset[i] = n_s;
	n_s += defn[graph->phone[i]].n_state;
    }

    state    = ckd_calloc(n_s, sizeof(state_t));
    n_prior  = ckd_calloc(n_s, sizeof(uint32));
    n_next   = ckd_calloc(n_s, sizeof(uint32));

    mixw_map = remap_init(4 * n_s);
    cb_map   = remap_init(4 * n_s);

    count_arcs(graph, state_offset, defn, all_tmat,
               n_next, &total_next,
               n_prior, &total_prior);

    next_state  = ckd_calloc(total_next,  sizeof(uint32));
    next_tprob  = ckd_calloc(total_next,  sizeof(float32));
    prior_state = ckd_calloc(total_prior, sizeof(uint32));
    prior_tprob = ckd_calloc(total_prior, sizeof(float32));

    write_arcs(graph, state_offset, defn, all_tmat,
               next_state, next_tprob, n_next, total_next,
               prior_state, prior_tprob, n_prior, total_prior);

    /* Assemble state_t array, mirroring state_seq.c's per-state
     * initialization. */
    n = 0;
    p = 0;
    for (i = 0, k = 0; i < graph->n; i++) {
	acmod_id_t phn = graph->phone[i];
	acmod_id_t ci_phn = acmod_set_base_phone(acmod_set, phn);
	uint32 n_ms = defn[phn].n_state;
	uint32 tmat = defn[phn].tmat;
	uint32 t_state;
	uint32 t_ci_state;

	assert(n_ms == defn[ci_phn].n_state);

	for (j = 0; j < n_ms - 1; j++, k++) {
	    t_state    = defn[phn].state[j];
	    t_ci_state = defn[ci_phn].state[j];

	    state[k].phn      = phn;
	    state[k].mixw     = t_state;
	    state[k].cb       = mdef->cb[t_state];
	    state[k].ci_mixw  = t_ci_state;
	    state[k].ci_cb    = mdef->cb[t_ci_state];

	    state_seq_set_next(state, k, n_next,
			       next_state, next_tprob, &n);
	    state_seq_set_prior(state, k, n_prior,
				prior_state, prior_tprob, &p);

	    state[k].tmat    = tmat;
	    state[k].m_state = j;

	    state[k].l_mixw    = remap(mixw_map, state[k].mixw);
	    state[k].l_cb      = remap(cb_map,   state[k].cb);
	    state[k].l_ci_mixw = remap(mixw_map, state[k].ci_mixw);
	    state[k].l_ci_cb   = remap(cb_map,   state[k].ci_cb);
	}

	/* Non-emit exit state. */
	state[k].mixw =
	    state[k].ci_mixw =
	    state[k].l_mixw =
	    state[k].l_ci_mixw = TYING_NO_ID;

	state[k].cb =
	    state[k].ci_cb =
	    state[k].l_cb =
	    state[k].l_ci_cb = TYING_NO_ID;

	state[k].tmat    = tmat;
	state[k].m_state = j;
	state[k].phn     = phn;

	state_seq_set_prior(state, k, n_prior,
			    prior_state, prior_tprob, &p);
	state_seq_set_next(state, k, n_next,
			   next_state, next_tprob, &n);

	++k;
    }

    assert(k == n_s);
    assert(n == total_next);
    assert(p == total_prior);

    /* Side effect parity with state_seq_make: refresh the inventory's
     * mixw_inverse / cb_inverse maps. */
    if (inv->l_mixw_acc) {
	ckd_free_3d((void ***) inv->l_mixw_acc);
	inv->l_mixw_acc = NULL;
    }
    if (inv->mixw_inverse) ckd_free((void *) inv->mixw_inverse);
    inv->mixw_inverse = remap_inverse(mixw_map, &n_map);
    inv->n_mixw_inverse = n_map;

    if (inv->cb_inverse) ckd_free((void *) inv->cb_inverse);
    inv->cb_inverse = remap_inverse(cb_map, &n_map);
    inv->n_cb_inverse = n_map;

    remap_free(mixw_map);
    remap_free(cb_map);

    ckd_free(n_prior);
    ckd_free(n_next);
    ckd_free(state_offset);

    /* next_state/next_tprob/prior_state/prior_tprob are NOT freed;
     * they're now referenced by state[].next_state / prior_state /
     * tprob fields. state_seq_free() releases them. */

    *n_state = n_s;
    return state;
}
