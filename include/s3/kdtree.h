/* ====================================================================
 * Copyright (c) 2005 Carnegie Mellon University.  All rights 
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
 * File: kdtree.h
 * 
 * Description: kd-tree implementation for mixture Gaussians
 * 
 * Author: David Huggins-Daines <dhuggins@cs.cmu.edu>
 * 
 *********************************************************************/

#ifndef __KDTREE_H__
#define __KDTREE_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include "sphinxbase/prim_type.h"
#include "s3/gauden.h"
#include "s3/vector.h"

typedef struct kd_tree_node_s kd_tree_node_t;
struct kd_tree_node_s {
	const vector_t *means, *variances; /* Codebook of Gaussians, shared by all nodes */
	float32 **boxes;	     /* Gaussian boxes for codebook, shared by all nodes */
	float32 threshold;           /* Threshold for Gaussian boxes */
	uint32 n_level;	             /* Number of levels (0 for non-root) */
	uint32 n_density, n_comp;    /* Number of densities, number of components. */
	uint8 *bbi;		/* BBI vector of intersecting Gaussians */
	vector_t lower, upper;	/* Lower and upper coordinates of projection */
	uint32 split_comp;	/* Dimension in which split is done */
	float32 split_plane;	/* Hyperplane splitting this node */
	kd_tree_node_t *left, *right; /* Child nodes */
};

kd_tree_node_t *build_kd_tree(const vector_t *means, const vector_t *variances,
			      uint32 n_density, uint32 n_comp,
			      float32 threshold, int32 n_levels,
			      int32 absolute);
void free_kd_tree(kd_tree_node_t *tree);

int32 write_kd_trees(const char *outfile, kd_tree_node_t **trees, uint32 n_trees);
int32 read_kd_trees(const char *infile, kd_tree_node_t ***out_trees, uint32 *out_n_trees);

#ifdef __cplusplus
}
#endif
#endif /* __KDTREE_H__ */
