/**
 * @file phone_graph.c
 * @brief Implementation of the multi-pron-capable phone graph builder.
 *
 * See phone_graph.h for the data structure and the contract.
 */

#include <s3/phone_graph.h>
#include <s3/s3.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* allocation                                                         */
/* ------------------------------------------------------------------ */

phone_graph_t *
phone_graph_alloc(uint32 n)
{
    phone_graph_t *g = ckd_calloc(1, sizeof(phone_graph_t));
    g->n = n;
    if (n == 0) {
	return g;
    }
    g->phone     = ckd_calloc(n, sizeof(acmod_id_t));
    g->btw_mark  = ckd_calloc(n, sizeof(char));
    g->n_next    = ckd_calloc(n, sizeof(uint32));
    g->next_idx  = ckd_calloc(n, sizeof(uint32 *));
    g->n_prior   = ckd_calloc(n, sizeof(uint32));
    g->prior_idx = ckd_calloc(n, sizeof(uint32 *));
    return g;
}

void
phone_graph_free(phone_graph_t *g)
{
    uint32 i;

    if (g == NULL) return;

    if (g->next_idx) {
	for (i = 0; i < g->n; i++) {
	    if (g->next_idx[i]) ckd_free(g->next_idx[i]);
	}
	ckd_free(g->next_idx);
    }
    if (g->prior_idx) {
	for (i = 0; i < g->n; i++) {
	    if (g->prior_idx[i]) ckd_free(g->prior_idx[i]);
	}
	ckd_free(g->prior_idx);
    }
    if (g->n_next)   ckd_free(g->n_next);
    if (g->n_prior)  ckd_free(g->n_prior);
    if (g->phone)    ckd_free(g->phone);
    if (g->btw_mark) ckd_free(g->btw_mark);

    ckd_free(g);
}

/* ------------------------------------------------------------------ */
/* construction helpers                                               */
/* ------------------------------------------------------------------ */

/* Set a slot's successor list to a single index (the most common
 * "linear" case). */
static void
set_next_one(phone_graph_t *g, uint32 i, uint32 succ)
{
    g->n_next[i] = 1;
    g->next_idx[i] = ckd_calloc(1, sizeof(uint32));
    g->next_idx[i][0] = succ;
}

/* Set a slot's predecessor list to a single index. */
static void
set_prior_one(phone_graph_t *g, uint32 i, uint32 pred)
{
    g->n_prior[i] = 1;
    g->prior_idx[i] = ckd_calloc(1, sizeof(uint32));
    g->prior_idx[i][0] = pred;
}

/* Set a slot's successor list to a freshly-allocated array of
 * `n` indices, copied from `idxs`. */
static void
set_next_many(phone_graph_t *g, uint32 i, const uint32 *idxs, uint32 n)
{
    g->n_next[i] = n;
    if (n == 0) {
	g->next_idx[i] = NULL;
	return;
    }
    g->next_idx[i] = ckd_calloc(n, sizeof(uint32));
    memcpy(g->next_idx[i], idxs, n * sizeof(uint32));
}

/* Set a slot's predecessor list to a freshly-allocated array of
 * `n` indices, copied from `idxs`. */
static void
set_prior_many(phone_graph_t *g, uint32 i, const uint32 *idxs, uint32 n)
{
    g->n_prior[i] = n;
    if (n == 0) {
	g->prior_idx[i] = NULL;
	return;
    }
    g->prior_idx[i] = ckd_calloc(n, sizeof(uint32));
    memcpy(g->prior_idx[i], idxs, n * sizeof(uint32));
}

/* ------------------------------------------------------------------ */
/* mk_phone_graph                                                     */
/* ------------------------------------------------------------------ */

/*
 * Per-word expansion plan. For each word in the transcript we record
 * the list of variants we'll include (1 if !multipron, else all from
 * the base-word index) and where each variant's phone slots sit in
 * the final flat `phone[]` array.
 */
typedef struct word_plan_s {
    uint32 n_variants;
    lex_entry_t **variant;     /* [n_variants] */
    uint32 *first_slot;        /* [n_variants] starting index in phone[] */
    uint32 *last_slot;         /* [n_variants] last index in phone[] */
} word_plan_t;

static void
word_plan_free_all(word_plan_t *plan, uint32 n_word)
{
    uint32 i;
    if (!plan) return;
    for (i = 0; i < n_word; i++) {
	if (plan[i].variant)    ckd_free(plan[i].variant);
	if (plan[i].first_slot) ckd_free(plan[i].first_slot);
	if (plan[i].last_slot)  ckd_free(plan[i].last_slot);
    }
    ckd_free(plan);
}

phone_graph_t *
mk_phone_graph(char **word,
               uint32 n_word,
               lexicon_t *lex,
               int multipron)
{
    word_plan_t *plan;
    uint32 i, v, p;
    uint32 total_slots;
    phone_graph_t *g;
    /* word_active[i] is false for words we drop entirely (phone_cnt == 0
     * for every variant); pass 3 uses this to splice the predecessor
     * past the dropped word, matching mk_phone_list's behavior of
     * silently skipping silent dict entries. */
    char *word_active;

    if (n_word == 0) {
	E_WARN("Word sequence is empty\n");
	return NULL;
    }

    /* Pass 1: gather the variants for each word and count total slots. */
    plan = ckd_calloc(n_word, sizeof(word_plan_t));
    word_active = ckd_calloc(n_word, sizeof(char));
    total_slots = 0;

    for (i = 0; i < n_word; i++) {
	lex_entry_t *chain_head = NULL;
	lex_entry_t *single = NULL;
	uint32 n_var = 0;

	if (multipron) {
	    chain_head = lexicon_lookup_variants(lex, word[i]);
	    if (chain_head != NULL) {
		lex_entry_t *e;
		for (e = chain_head; e; e = lex_entry_next_variant(e)) {
		    ++n_var;
		}
	    }
	}
	if (n_var == 0) {
	    /* Either !multipron, or multipron with a disambiguated
	     * transcript token like "reading(2)" that isn't a base
	     * word: try the direct ortho lookup. */
	    single = lexicon_lookup(lex, word[i]);
	    n_var = (single != NULL) ? 1 : 0;
	}
	if (n_var == 0) {
	    E_WARN("Unable to look up word '%s' in the dictionary\n", word[i]);
	    word_plan_free_all(plan, n_word);
	    ckd_free(word_active);
	    return NULL;
	}

	plan[i].n_variants = n_var;
	plan[i].variant    = ckd_calloc(n_var, sizeof(lex_entry_t *));
	plan[i].first_slot = ckd_calloc(n_var, sizeof(uint32));
	plan[i].last_slot  = ckd_calloc(n_var, sizeof(uint32));

	if (chain_head != NULL) {
	    lex_entry_t *e;
	    v = 0;
	    for (e = chain_head; e; e = lex_entry_next_variant(e), v++) {
		plan[i].variant[v] = e;
	    }
	} else {
	    plan[i].variant[0] = single;
	}

	/* Each variant contributes phone_cnt slots. Filter out
	 * 0-phone variants (collapse them out of the plan); if every
	 * variant has 0 phones, drop the word entirely to mirror the
	 * historical mk_phone_list behavior. */
	{
	    uint32 kept = 0;
	    lex_entry_t **kept_variants = ckd_calloc(n_var, sizeof(lex_entry_t *));
	    uint32 *kept_first = ckd_calloc(n_var, sizeof(uint32));
	    uint32 *kept_last  = ckd_calloc(n_var, sizeof(uint32));
	    for (v = 0; v < n_var; v++) {
		uint32 m = plan[i].variant[v]->phone_cnt;
		if (m == 0) continue;
		kept_variants[kept] = plan[i].variant[v];
		kept_first[kept] = total_slots;
		kept_last[kept]  = total_slots + m - 1;
		total_slots += m;
		++kept;
	    }
	    ckd_free(plan[i].variant);
	    ckd_free(plan[i].first_slot);
	    ckd_free(plan[i].last_slot);
	    plan[i].variant    = kept_variants;
	    plan[i].first_slot = kept_first;
	    plan[i].last_slot  = kept_last;
	    plan[i].n_variants = kept;
	    word_active[i] = (kept > 0) ? 1 : 0;
	}
    }

    /* Pass 2: allocate the graph and fill in phone[] + btw_mark[]
     * along with intra-variant linear adjacencies. */
    g = phone_graph_alloc(total_slots);

    for (i = 0; i < n_word; i++) {
	for (v = 0; v < plan[i].n_variants; v++) {
	    lex_entry_t *e = plan[i].variant[v];
	    uint32 m = e->phone_cnt;
	    if (m == 0) continue;
	    for (p = 0; p < m; p++) {
		uint32 slot = plan[i].first_slot[v] + p;
		g->phone[slot] = e->ci_acmod_id[p];
		/* btw_mark is set only on the last phone of each
		 * variant (one boundary per variant). */
		g->btw_mark[slot] = (p == m - 1) ? 1 : 0;
		if (p < m - 1) {
		    set_next_one(g, slot, slot + 1);
		}
		if (p > 0) {
		    set_prior_one(g, slot, slot - 1);
		}
	    }
	}
    }

    /* Pass 3: cross-word edges. The last phone of every variant of
     * word i connects forward to the first phone of every variant of
     * the next active word; inactive (0-phone) words are spliced
     * out. For single-pron neighbors this is exactly the linear
     * "last(i) -> first(i+1)" edge.
     *
     * set_next_many / set_prior_many copy from the source array, so
     * we pass plan[*].first_slot / plan[*].last_slot directly; no
     * staging buffer needed. */
    {
	uint32 next_active;

	for (i = 0; i < n_word; i++) {
	    if (!word_active[i]) continue;
	    for (next_active = i + 1;
		 next_active < n_word && !word_active[next_active];
		 next_active++) { /* skip */ }
	    if (next_active >= n_word) continue;

	    {
		uint32 nx = plan[next_active].n_variants;
		uint32 cur = plan[i].n_variants;

		for (v = 0; v < cur; v++) {
		    set_next_many(g, plan[i].last_slot[v],
				  plan[next_active].first_slot, nx);
		}
		for (v = 0; v < nx; v++) {
		    set_prior_many(g, plan[next_active].first_slot[v],
				   plan[i].last_slot, cur);
		}
	    }
	}
    }

    ckd_free(word_active);
    word_plan_free_all(plan, n_word);
    return g;
}

/* ------------------------------------------------------------------ */
/* diagnostics                                                        */
/* ------------------------------------------------------------------ */

void
phone_graph_print(const phone_graph_t *g, acmod_set_t *acmod_set)
{
    uint32 i, j;

    if (!g) {
	printf("(null phone_graph_t)\n");
	return;
    }

    printf("phone_graph_t n=%u\n", g->n);
    for (i = 0; i < g->n; i++) {
	printf("  [%3u] %-10s btw=%d next={",
	       i,
	       acmod_set ? acmod_set_id2name(acmod_set, g->phone[i]) : "?",
	       g->btw_mark[i] ? 1 : 0);
	for (j = 0; j < g->n_next[i]; j++) {
	    printf("%s%u", (j ? "," : ""), g->next_idx[i][j]);
	}
	printf("} prior={");
	for (j = 0; j < g->n_prior[i]; j++) {
	    printf("%s%u", (j ? "," : ""), g->prior_idx[i][j]);
	}
	printf("}\n");
    }
}
