/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: dtree.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef DTREE_H
#define DTREE_H

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <s3/dtree.h>
#include <s3/pset_io.h>
#include <sphinxbase/prim_type.h>
#include <s3/acmod_set.h>
#include <s3/quest.h>

#define NO_CLUST (0xffffffff)

typedef struct dtree_node_str {
    uint32 node_id;

    uint32 clust;		/* If simple tree, which cluster (0 or 1) does
				   this leaf node belong to.

				   If complex tree and a leaf node, clust is the
				   tied state id of the node */

    uint32 *id;                 /* id's of triphones represented by this node */
    uint32 n_id;                /* # of triphones represented */

    float32 ***mixw_occ;
    float32 ***means;
    float32 ***vars;
    float32 occ;                /* # of time state is observed */
    float64 wt_ent;             /* weighted entropy of this node */

    void *q;			/* best question for this node */

    float64 wt_ent_dec;         /* Weighted entropy decrease of doing split */

    struct dtree_node_str *p;   /* parent node */
    struct dtree_node_str *y;   /* yes child node */
    struct dtree_node_str *n;   /* no child node */
} dtree_node_t;

#define IS_LEAF(nd) ((((nd)->y == NULL) && ((nd)->n == NULL)))
#define IS_TWIG(nd) (!IS_LEAF(nd) && IS_LEAF((nd)->y) && IS_LEAF((nd)->n))

typedef struct dtree_s {
    dtree_node_t *node;
    uint32 n_node;
} dtree_t;

uint32
cnt_node(dtree_node_t *node);

uint32
reindex(dtree_node_t *node,
	uint32 *next_id);

uint32
cnt_leaf(dtree_node_t *node);

uint32
label_leaves(dtree_node_t *node, uint32 *id);

dtree_node_t *
get_node(dtree_node_t *node,
	 uint32 id);

uint32
tied_state(dtree_node_t *node,
	   acmod_id_t b,
	   acmod_id_t l,
	   acmod_id_t r,
	   word_posn_t wp,
	   pset_t *pset);

uint32
cnt_twig(dtree_node_t *node);

void
print_node(FILE *fp,
	   dtree_node_t *node,
	   pset_t *pset);

void
print_node_comp(FILE *fp,
		dtree_node_t *node,
		pset_t *pset);

			
void
print_final_tree(FILE *fp,
		 dtree_node_t *node,
		 pset_t *pset);

void
print_final_tree_davinci(FILE *fp,
			 dtree_node_t *node,
			 pset_t *pset);

dtree_t *
read_final_tree(FILE *fp,
		pset_t *pset,
		uint32 n_pset);

void
free_tree(dtree_t *tr);

void
print_tree(FILE *fp,
	   char *label,
	   dtree_node_t *node,
	   pset_t *pset,
	   uint32 lvl);

void
print_tree_comp(FILE *fp,
		char *label,
		dtree_node_t *node,
		pset_t *pset,
		uint32 lvl);

int
mk_node(dtree_node_t *node,
	uint32 node_id,

	uint32 *id,
	uint32 n_id,

	float32 ****mixw,
        float32 ****means,
        float32 ****vars,
        uint32  *veclen,
        uint32 n_model,
	uint32 n_state,
	uint32 n_stream,
	uint32 n_density,
	float32 *stwt,
	
	float32 mwfloor);

float64
set_best_quest(dtree_node_t *node,

	       float32 ****mixw,
               float32 ****means,
               float32 ****vars,
               uint32  *veclen,
	       uint32 n_model,
	       uint32 n_state,
	       uint32 n_stream,
	       uint32 n_density,
	       float32 *stwt,

	       quest_t *all_q,
	       uint32 n_all_q,
	       pset_t *pset,

	       uint32 **dfeat,
	       uint32 n_dfeat,

	       float32 mwfloor);

dtree_t *
mk_tree(float32 ****mixw,
        float32 ****means,
        float32 ****vars,
        uint32  *veclen,
	uint32 n_model,
	uint32 n_state,
	uint32 n_stream,
	uint32 n_density,
	float32 *stwt,
	
	uint32 *id,
	uint32 n_id,

	quest_t *all_q,
	uint32 n_all_q,
	pset_t *pset,

	uint32 **dfeat,
	uint32 n_dfeat,

	uint32 max_split,
	uint32 min_split,
	float32 split_thr,

	float32 mwfloor);

dtree_t *
mk_tree_comp(float32 ****mixw,
             float32 ****means,
             float32 ****vars,
             uint32  *veclen,
	     uint32 n_model,
	     uint32 n_state,
	     uint32 n_stream,
	     uint32 n_density,
	     float32 *stwt,
	
	     uint32 *id,
	     uint32 n_id,

	     quest_t *all_q,
	     uint32 n_all_q,
	     pset_t *pset,
	     uint32 n_base_phone,
	     
	     uint32 **dfeat,
	     uint32 n_dfeat,

	     uint32  split_min,
	     uint32  split_max,
	     float32 split_thr,

	     uint32  split_min_comp,
	     uint32  split_max_comp,
	     float32 split_thr_comp,

	     float32 mwfloor);

void
cluster_leaves(dtree_t *tr,
               uint32 *veclen,
	       float64 *wt_ent_dec,
	       uint32 *out_n_a,
	       uint32 *out_n_b,
	       pset_t *pset,
	       uint32 n_state,
	       uint32 n_stream,
	       uint32 n_density,
	       float32 *stwt,
	       float32 mwfloor);

void
split_node_comp(dtree_t *tr,
                uint32 node_id,
                float32 ****mixw,
                float32 ****means,
                float32 ****vars,
                uint32  *veclen,
                uint32 n_model,
                uint32 n_state,
                uint32 n_stream,
                uint32 n_density,
                float32 *stwt,
                quest_t *all_q,
                uint32 n_all_q,
                pset_t *pset,
                uint32 n_base_phone,
                uint32 **dfeat,
                uint32 n_dfeat,
                uint32  split_min,
                uint32  split_max,
                float32 split_thr,
                float32 mwfloor);

void
split_node(dtree_t *tr,
	   uint32 node_id,
	   float32 ****mixw,
           float32 ****means,
           float32 ****vars,
           uint32 *veclen,
	   uint32 n_model,
	   uint32 n_state,
	   uint32 n_stream,
	   uint32 n_density,
	   float32 *stwt,

	   quest_t *all_q,
	   uint32 n_all_q,
	   pset_t *pset,
	   
	   uint32 **dfeat,
	   uint32 n_dfeat,

	   float32 mwfloor);

int
split_node_nobest(dtree_t *tr,
		  uint32 node_id,
	   
		  float32 ****mixw,
		  uint32 n_model,
		  uint32 n_state,
		  uint32 n_stream,
		  uint32 n_density,
		  float32 *stwt,

		  uint32 **dfeat,
		  uint32 n_dfeat);

comp_quest_t *
mk_comp_quest(float64 *wt_ent_dec,
	      float32 ****mixw,
              float32 ****means,
              float32 ****vars,
              uint32  *veclen,
	      uint32 n_model,
	      uint32 n_state,
	      uint32 n_stream,
	      uint32 n_density,
	      float32 *stwt,
	
	      uint32 *id,
	      uint32 n_id,

	      quest_t *all_q,
	      uint32 n_all_q,
	      pset_t *pset,
	      uint32 n_base_phone,

	      uint32 **dfeat,
	      uint32 n_dfeat,
	      
	      uint32 split_min,
	      uint32 split_max,
	      float32 split_thr,

	      float32 mwfloor);

int
ins_twigs(dtree_node_t *node,
	  uint32 phnid,
	  uint32 state,
	  float32 *twig_heap,
	  uint32 *twig_hkey,
	  uint32 *phnidlst,
	  uint32 *statelst,
	  uint32 *nidlst,
	  uint32 *free_key);

uint32
prune_subtrees(dtree_node_t *node);

uint32
prune_lowcnt(dtree_node_t *node, float32 cnt_thr);

uint32
leaf_mean_vars(dtree_node_t *node,
               pset_t *pset,
               float32 ****means,
               float32 ****vars,
               uint32 *node_id,
               uint32 n_state,
               uint32 n_stream,
               uint32 *veclen,
               uint32 off);

#ifdef __cplusplus
}
#endif
#endif /* DTREE_H */ 
