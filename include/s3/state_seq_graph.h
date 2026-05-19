/**
 * @file state_seq_graph.h
 * @brief Graph-aware utterance HMM builder for multi-pron training.
 *
 * Wraps state_seq_make() to operate on a phone_graph_t instead of a
 * linear phone[] array. For a linear graph (all slots with n_next == 1
 * and next_idx[i][0] == i+1) it produces a state_t array bit-identical
 * to state_seq_make()'s output on the equivalent phone[] sequence.
 *
 * For a graph with multi-pron branches it produces a state graph
 * where the non-emitting exit state of a phone slot fans out to the
 * first states of all graph successors (and symmetrically for
 * predecessors). Outgoing transition probabilities at fan-out points
 * are initialized uniformly: 1/n_next[i] per arc.
 *
 * The graph is expected to have unambiguous triphone contexts before
 * this is called: cvt2triphone_graph() handles any required slot
 * duplication at multi-pron join points so each slot's `phone` field
 * carries a single, fully-specified acmod_id (CI or triphone).
 */

#ifndef STATE_SEQ_GRAPH_H
#define STATE_SEQ_GRAPH_H
#ifdef __cplusplus
extern "C" {
#endif

#include <s3/model_def_io.h>
#include <s3/model_inventory.h>
#include <s3/phone_graph.h>
#include <s3/state.h>

/*
 * Build a state-level utterance HMM from a phone graph.
 *
 * On success, returns a freshly-allocated `state_t` array of length
 * `*n_state`; caller is responsible for freeing it via
 * `state_seq_free(state, *n_state)`. The model inventory's
 * mixw_inverse / cb_inverse fields are updated (matching the
 * existing state_seq_make's side effect).
 *
 * Returns NULL on error.
 */
state_t *
state_seq_make_graph(uint32 *n_state,
                     const phone_graph_t *graph,
                     model_inventory_t *inv,
                     model_def_t *mdef);

#ifdef __cplusplus
}
#endif
#endif /* STATE_SEQ_GRAPH_H */
