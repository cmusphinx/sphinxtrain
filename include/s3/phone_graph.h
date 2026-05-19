/**
 * @file phone_graph.h
 * @brief Phone-level utterance graph supporting multiple pronunciations per word.
 *
 * Replaces the implicit linear chain produced by mk_phone_list with
 * an explicit graph that captures word-boundary branches.
 *
 * For utterances where every word has a single pronunciation, the
 * resulting graph is a strict linear chain (n_next[i] = 1,
 * next_idx[i][0] = i+1 for all i; mirror for prior), so callers that
 * traverse via adjacency lists produce output identical to the
 * historical mk_phone_list path.
 *
 * Triphone context expansion at multi-pron join points (where the
 * first phone of the next word would otherwise have two different
 * left contexts) is handled by the consumer (cvt2triphone_graph /
 * state_seq_make_graph), not by this layer; this module only
 * describes the combinatorial structure of the utterance.
 */

#ifndef PHONE_GRAPH_H
#define PHONE_GRAPH_H
#ifdef __cplusplus
extern "C" {
#endif

#include <s3/acmod_set.h>
#include <s3/lexicon.h>
#include <sphinxbase/prim_type.h>

typedef struct phone_graph_s {
    /* Number of phone slots in the graph (sum over all variants of
     * all words). For a single-pron utterance this equals the
     * sum of word phone counts, matching the old `n_phone`. */
    uint32 n;

    /* Per-slot data, all length n. */
    acmod_id_t *phone;     /* CI phone id at this slot */
    char       *btw_mark;  /* nonzero if a word boundary follows this slot */

    /* Per-slot successor lists. n_next[i] is the number of successor
     * slots; next_idx[i] is an array of indices into this graph. For
     * the linear case n_next[i] == 1 and next_idx[i][0] == i+1
     * (last slot has n_next[i] == 0). At a multi-pron word boundary
     * the previous word's last phone branches into the first phones
     * of each variant of the next word; at a join the last phones of
     * each variant fan in to the first phone of the next word. */
    uint32  *n_next;
    uint32 **next_idx;

    /* Symmetric predecessor lists (n_prior[i] entries each). */
    uint32  *n_prior;
    uint32 **prior_idx;
} phone_graph_t;

/*
 * Build a phone graph from a word sequence.
 *
 * For each word, all pronunciation variants registered in the
 * lexicon's base-word index are included as parallel paths. The
 * lexicon must have been read with the phase-1 base-word index
 * populated (any lexicon_read() since the addition does).
 *
 * Behavior:
 * - If `multipron` is FALSE, every word is expanded using exactly the
 *   first variant (this matches the historical mk_phone_list
 *   behavior; useful for parity testing).
 * - If TRUE, every word with k variants contributes k parallel
 *   pronunciation chains. Single-variant words still contribute one
 *   linear chain.
 *
 * Returns NULL on lookup failure (the offending word is logged with
 * E_WARN). The caller owns the returned phone_graph_t and must free
 * it with phone_graph_free().
 */
phone_graph_t *
mk_phone_graph(char **word,
               uint32 n_word,
               lexicon_t *lex,
               int multipron);

/* Allocate a graph with `n` slots and zeroed adjacency lists. The
 * caller fills in `phone`, `btw_mark`, n_next/next_idx,
 * n_prior/prior_idx. Used by mk_phone_graph and by tests. */
phone_graph_t *
phone_graph_alloc(uint32 n);

/* Free a graph and all owned arrays. NULL-safe. */
void
phone_graph_free(phone_graph_t *graph);

/* Diagnostic: print the graph in a readable form. */
void
phone_graph_print(const phone_graph_t *graph, acmod_set_t *acmod_set);

/*
 * Return a copy of `in` in which every slot has at most one distinct
 * CI-phone predecessor — i.e. every slot has an unambiguous left
 * context for triphone resolution. Where the input has a slot whose
 * predecessors carry multiple distinct CI phones, that slot is
 * duplicated once per distinct predecessor CI phone, and the
 * predecessor edges are partitioned accordingly.
 *
 * For an input with no ambiguous-context slots (the single-pron
 * case), the output is structurally identical to the input.
 *
 * The caller owns both `in` and the returned graph and must free
 * each with phone_graph_free().
 */
phone_graph_t *
phone_graph_split_contexts(const phone_graph_t *in);

/*
 * Walk an already-unambiguous-context graph (typically the output of
 * phone_graph_split_contexts) and replace each slot's CI `phone`
 * field with the triphone acmod_id appropriate for its (l, c, r,
 * word_position) context. Slots with no successor get SIL as the
 * right context (matching cvt2triphone()'s convention). Filler
 * phones are left as their CI ids.
 *
 * Returns S3_SUCCESS or S3_ERROR.
 */
int
cvt2triphone_graph(phone_graph_t *graph, acmod_set_t *acmod_set);

#ifdef __cplusplus
}
#endif
#endif /* PHONE_GRAPH_H */
