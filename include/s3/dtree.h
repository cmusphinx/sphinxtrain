/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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

#include <s3/dtree.h>
#include <s3/pset_io.h>
#include <s3/prim_type.h>
#include <s3/acmod_set.h>
#include <s3/btree.h>
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
/* ADDITION FOR CONTINUOUS_TREES, 19 May 98 */
    float32 ***means;
    float32 ***vars;
/* END ADDITION FOR CONTINUOUS_TREES */
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

typedef struct {
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
/* ADDITION FOR CONTINUOUS_TREES, 18 May 98 */
        float32 ****means,
        float32 ****vars,
        uint32  *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
        uint32 n_model,
	uint32 n_state,
	uint32 n_stream,
	uint32 n_density,
	float32 *stwt,
	
	float32 mwfloor);

float64
set_best_quest(dtree_node_t *node,

	       float32 ****mixw,
/* ADDITION FOR CONTINUOUS_TREES, 20 May 98 */
               float32 ****means,
               float32 ****vars,
               uint32  *veclen,
/* END ADDITION FOR CONTINUOUS TREES */
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
/* ADDITION FOR CONTINUOUS_TREES, 20 May 98 */
        float32 ****means,
        float32 ****vars,
        uint32  *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
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
/* ADDITION FOR CONTINUOUS_TREES, 18 May 98 */
             float32 ****means,
             float32 ****vars,
             uint32  *veclen,
/* END ADDITIONS FOR CONTINUOUS_TREES */
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
/* ADDITION FOR CONTINUOUS_TREES */
               uint32 *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
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
/* ADDITION FOR CONTINUOUS_TREES */
                float32 ****means,
                float32 ****vars,
                uint32  *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
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
/* ADDITION FOR CONTINUOUS_TREES */
           float32 ****means,
           float32 ****vars,
           uint32 *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
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
/* ADDITIONS FOR CONTINUOUS_TREES, 19 May 98 */
              float32 ****means,
              float32 ****vars,
              uint32  *veclen,
 /* END ADDITIONS FOR CONTINUOUS_TREES */
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

/* ADDITION FOR CONTINUOUS_TREES */
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
/* END ADDITION FOR CONTINUOUS_TREES */

#endif /* DTREE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/23  10:48:42  eht
 * Added get_node() function
 * 
 * Revision 1.3  97/07/17  14:29:02  eht
 * Added prune_lowcnt() function for pruning low occupancy count tree nodes.
 * 
 * Revision 1.2  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/07  10:53:27  eht
 * Initial revision
 * 
 *
 */
