/**
 * @file state_seq_internal.h
 * @brief Private helpers shared between state_seq.c and state_seq_graph.c.
 *
 * Not for use outside libcommon. The signatures slice the next
 * n_next[s] / n_prior[s] entries out of the packed next_state[] /
 * prior_state[] (and matching tprob[]) arrays into state[s] and
 * advance the running cursor `*n` / `*p`.
 */

#ifndef STATE_SEQ_INTERNAL_H
#define STATE_SEQ_INTERNAL_H

#include <s3/state.h>
#include <sphinxbase/prim_type.h>

void
state_seq_set_next(state_t *state,
		   uint32 s,
		   const uint32 *n_next,
		   uint32 *next_state,
		   float32 *next_tprob,
		   uint32 *n);

void
state_seq_set_prior(state_t *state,
		    uint32 s,
		    const uint32 *n_prior,
		    uint32 *prior_state,
		    float32 *prior_tprob,
		    uint32 *p);

#endif /* STATE_SEQ_INTERNAL_H */
