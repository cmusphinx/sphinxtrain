/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * File: main.c
 * 
 * Description: 
 *    Prune decision trees.
 *
 * Author: 
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <s3/dtree.h>
#include <s3/model_def_io.h>
#include <s3/quest.h>
#include <s3/pset_io.h>
#include <s3/s3.h>
#include <sys_compat/file.h>

#include <assert.h>

static int
init(model_def_t **out_mdef,
     pset_t **out_pset,
     uint32 *out_n_pset)
{
    const char *moddeffn;
    model_def_t *mdef;

    const char *psetfn;
    pset_t *pset;
    uint32 n_pset;

    moddeffn = cmd_ln_str("-moddeffn");
    if (moddeffn == NULL)
	E_FATAL("Specify -moddeffn\n");
    E_INFO("Reading: %s\n", moddeffn);
    if (model_def_read(&mdef, moddeffn) != S3_SUCCESS)
	return S3_ERROR;
    *out_mdef = mdef;

    psetfn = cmd_ln_str("-psetfn");
    E_INFO("Reading: %s\n", psetfn);
    *out_pset = pset = read_pset_file(psetfn, mdef->acmod_set, &n_pset);
    *out_n_pset = n_pset;

    return S3_SUCCESS;
}

#include <s3/heap.h>

int
heap_ok(uint32 *hkey,
	uint32 sz,

	uint32 *phn,
	uint32 *st,
	uint32 *nd,
	model_def_t *mdef)
{
    uint32 i, k_i;
    uint32 j, k_j;
    int ok = TRUE;

    for (i = 0; i < sz-1; i++) {
	k_i = hkey[i];

	if (phn[k_i] == NO_ID) {
	    E_ERROR("A hole is on the heap for key %u!\n", k_i);

	    ok = FALSE;
	    continue;
	}

	for (j = i+1; j < sz; j++) {
	    k_j = hkey[j];

	    if (phn[k_j] == NO_ID)
		continue;

	    if ((phn[k_i] == phn[k_j]) &&
		(st[k_i] == st[k_j]) &&
		(nd[k_i] == nd[k_j])) {
		E_ERROR("tree (%s %u) node %u on heap more than once\n",
			acmod_set_id2name(mdef->acmod_set, phn[k_i]),
			st[k_i],
			nd[k_i]);
		ok = FALSE;
	    }
	}
    }

    if (phn[hkey[sz-1]] == NO_ID) {
	E_ERROR("A hole is on the heap for key %u!\n", sz-1);
	
	ok = FALSE;
    }

    return ok;
}

static int
read_phone_trees(model_def_t *mdef,
		 const char *itreedir,
		 const char *pname,
		 uint32 p,
		 uint32 n_state,
		 pset_t *pset,
		 uint32 n_pset,
		 dtree_t ***out_tree,
		 uint32 *out_n_seno,
		 uint32 *out_n_twig,
		 uint32 *out_n_node)
{
    uint32 s;

    out_tree[p] = (dtree_t **)ckd_calloc(n_state, sizeof(dtree_t *));
    for (s = 0; s < n_state; s++) {
	char fn[MAXPATHLEN+1];
	dtree_t *tr;
	FILE *fp;

	sprintf(fn, "%s/%s-%u.dtree",
		itreedir, pname, s);

	fp = fopen(fn, "r");
	if (fp == NULL) {
	    E_FATAL_SYSTEM("Unable to open %s for reading",fn);
	}

	out_tree[p][s] = tr = read_final_tree(fp, pset, n_pset);

	if (tr == NULL) {
	    E_ERROR("Error(s) while reading tree\n");
	    return -1;
	}
	else {
	    uint32 n, lt_minocc;

	    lt_minocc = prune_lowcnt(&tr->node[0], cmd_ln_float32("-minocc"));
	    n = 0;
	    reindex(&tr->node[0], &n);
	    *out_n_seno += n = cnt_leaf(&tr->node[0]);
	    E_INFO("%s-%u\t%u [%u < %e]\n",
		   pname, s, n, lt_minocc,
		   cmd_ln_float32("-minocc"));
	    *out_n_twig += cnt_twig(&tr->node[0]);
	    *out_n_node += cnt_node(&tr->node[0]);
	}

	fclose(fp);
    }
    return 0;
}

static int
prune_tree(model_def_t *mdef,
	   pset_t *pset,
	   uint32 n_pset)
{
    const char *itreedir;
    const char *otreedir;
    char fn[MAXPATHLEN+1];
    FILE *fp;

    dtree_t ***tree;	/* Decision trees indexed by phone and state */
    dtree_t *tr;
    dtree_node_t *node, *prnt;
    float32 *twig_heap;	/* Heap of wt_ent_dec of split quest */
    uint32 *twig_hkey;	/* Key's for items in the heap */
    uint32 *twig_phnid;	/* Phone id of items on heap */
    uint32 *twig_state;	/* State id of items on heap */
    uint32 *twig_nid;	/* Node id of items on heap */
    uint32 free_key;	/* Next unused heap key */
    uint32 free_idx;	/* Next unused node index */
    uint32 n_ci, p, s, n;
    uint32 *n_state_ci;	/* # of state of models in the same base phone class */
    uint32 n_seno;
    uint32 n_seno_wanted;
    uint32 n_twig;
    uint32 n_node;
    float32 wt_ent_dec;
    uint32 key;
    uint32 sz;
    uint32 i;
    int allphones;
    int err;

    itreedir = cmd_ln_str("-itreedir");
    allphones = cmd_ln_int32("-allphones");
    n_ci = acmod_set_n_ci(mdef->acmod_set);

    tree = (dtree_t ***)ckd_calloc(n_ci, sizeof(dtree_t **));
    n_state_ci = (uint32 *)ckd_calloc(n_ci, sizeof(uint32));

    err = FALSE;
    if (allphones) {
	uint32 n_state;

	n_state = mdef->defn[n_ci].n_state-1;
	n_state_ci[0] = n_state;
	n_ci = 1;
	n_seno = 0;
	n_twig = 0;
	if (read_phone_trees(mdef, itreedir, "ALLPHONES", 0, n_state,
			     pset, n_pset, tree, &n_seno, &n_twig, &n_node) < 0)
	    err = TRUE;
    }
    else {
	for (p = 0, err = FALSE, n_seno = 0, n_twig = 0; p < n_ci; p++) {
	    if (!acmod_set_has_attrib(mdef->acmod_set, (acmod_id_t)p, "filler")) {
		const char *pname;
		uint32 n_state;

		pname = acmod_set_id2name(mdef->acmod_set, (acmod_id_t)p);
		n_state = mdef->defn[p].n_state-1;
		n_state_ci[p] = n_state;
		if (read_phone_trees(mdef, itreedir, pname, p, n_state,
				     pset, n_pset, tree,
				     &n_seno, &n_twig, &n_node) < 0)
		    err = TRUE;
	    }
	}
    }

    if (err) {
	E_ERROR("Error(s) while reading trees; pruning not done\n");
	return S3_ERROR;
    }

    E_INFO("Prior to pruning n_seno= %u\n", n_seno);

    n_seno_wanted = cmd_ln_int32("-nseno");
    if (n_seno < n_seno_wanted) {
	E_WARN("n_seno_wanted= %u, but only %u defined by trees\n",
	       n_seno_wanted, n_seno);
    }

    E_INFO("n_twig= %u\n", n_twig);
    
    if (n_seno_wanted < n_seno) {
	/* Heap of wt_ent_dec for each "twig" question */
	twig_heap  = (float32 *)ckd_calloc(n_twig, sizeof(float32));
	twig_hkey  = (uint32 *)ckd_calloc(n_twig, sizeof(uint32));

	twig_phnid = (uint32 *)ckd_calloc(n_twig, sizeof(uint32));
	twig_state = (uint32 *)ckd_calloc(n_twig, sizeof(uint32));
	twig_nid   = (uint32 *)ckd_calloc(n_twig, sizeof(uint32));
    
	/* Insert all twig questions over all trees into the heap */
	for (p = 0, free_key = 0; p < n_ci; p++) {
	    for (s = 0; s < n_state_ci[p]; s++) {
		if (allphones
		    || !acmod_set_has_attrib(mdef->acmod_set, (acmod_id_t)p, "filler")) {
		    tr = tree[p][s];

		    ins_twigs(&tr->node[0],
			      p, s,
			      twig_heap,
			      twig_hkey,
			      twig_phnid,
			      twig_state,
			      twig_nid,
			      &free_key);
		}
	    }
	}

	E_INFO("Pruning %u nodes\n", n_seno - n_seno_wanted);

	for (i = n_seno, sz = n_twig; (i > n_seno_wanted) && (sz > 0); i--) {
#if 0
	    if (!heap_ok(twig_hkey, sz,
			 twig_phnid, twig_state, twig_nid,
			 mdef)) {
		E_FATAL("heap problems; bug.\n");
	    }
#endif

	    /* extract the top (minimum wt_ent_dec) node off the
	       heap; this is the worst question of the tree. */
	    sz = heap32b_extr_top(&wt_ent_dec, &key,
				  twig_heap, twig_hkey, sz, heap32b_min_comp);

	    /* Get the node to prune */
	    p = twig_phnid[key];
	    s = twig_state[key];
	    n = twig_nid[key];
	    tr = tree[p][s];
	    node = get_node(&tr->node[0], n);

	    assert(IS_TWIG(node));

	    /* Make twig node a leaf by pruning its leaves */
	    prune_subtrees(node);

	    assert(IS_LEAF(node));

	    prnt = node->p;
	    if (prnt == NULL) {
		E_INFO("Root node extracted (%s %u) from heap\n",
		       acmod_set_id2name(mdef->acmod_set, (acmod_id_t)p), s);
	    }

	    /* Is the parent (if any) now a twig? */
	    if (prnt && IS_TWIG(prnt)) {
		/* Put it on the heap and reuse the heap-key for the child */

		twig_nid[key] = prnt->node_id;
		
		sz = heap32b_ins(twig_heap, twig_hkey, sz,
				 prnt->wt_ent_dec, key, heap32b_min_comp);
	    }
	    else {
		/* Parent node not a "twig" as a result of pruning */

		/* Set "holes" to values that are almost certain to
		 * cause a seg fault if used as an index */
		twig_phnid[key] = NO_ID;
		twig_state[key] = NO_ID;
		twig_nid[key] = NO_ID;
	    }
	}

	if ((sz == 0) && (i > n_seno_wanted)) {
	    E_WARN("%u seno's not generated because heap ran out\n", n_seno_wanted);
	}
    }

    otreedir = cmd_ln_str("-otreedir");
    for (p = 0, n_node = 0; p < n_ci; p++) {
	const char *pname;

	if (allphones)
	    pname = "ALLPHONES";
	else
	    pname = acmod_set_id2name(mdef->acmod_set, (acmod_id_t)p);

	for (s = 0; s < n_state_ci[p]; s++) {
	    if (allphones
		|| !acmod_set_has_attrib(mdef->acmod_set, (acmod_id_t)p, "filler")) {
		tr = tree[p][s];

		free_idx = 0;
		n_node += n = reindex(&tr->node[0], &free_idx);

		E_INFO("%s-%u\t%u\n", pname, s, n);

		sprintf(fn, "%s/%s-%u.dtree", otreedir, pname, s);
		
		fp = fopen(fn, "w");
		if (fp == NULL) {
		    E_FATAL_SYSTEM("Unable to open %s for writing", fn);
		}
		print_final_tree(fp, &tr->node[0], pset);
		fclose(fp);

	    }
	}
    }


    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    model_def_t *mdef = NULL;
    pset_t *pset = NULL;
    uint32 n_pset = 0;

    parse_cmd_ln(argc, argv);

    if (init(&mdef, &pset, &n_pset) != S3_SUCCESS) {
	E_FATAL("Initialization failed\n");
    }

    prune_tree(mdef, pset, n_pset);
    
    return 0;
}

