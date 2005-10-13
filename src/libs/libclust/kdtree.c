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
 * File: kdtree.c
 * 
 * Description: 
 * 	Implement kd-trees using the BBI algorithm as detailed in
 * 	J. Frisch and I. Rogina, "The Bucket Box Intersection
 * 	Algorithm for fast approximate evaluation of Diagonal Mixture
 * 	Gaussians", Proceedings of ICASSP 1996.
 *
 * Author: 
 * 	David Huggins-Daines <dhuggins@cs.cmu.edu>
 *********************************************************************/

#include <s3/common.h>
#include <s3/kdtree.h>
#include <math.h>

static int build_kdtree_level(kd_tree_node_t *node, uint32 n_levels);

kd_tree_node_t *
build_kd_tree(vector_t *means, vector_t *variances,
	      uint32 n_density, uint32 n_comp,
	      float32 threshold, int32 n_levels)
{
	kd_tree_node_t *node;
	int i, j;

	if (threshold < 0 || threshold > 1)
		E_FATAL("Threshold must be between 0 and 1\n");

	node = ckd_calloc(1, sizeof(*node));
	node->means = means;
	node->variances = variances;
	node->n_density = n_density;
	node->n_comp = n_comp;

	/* Precompute Gaussian boxes for all components */
	node->boxes = (float32 **)ckd_calloc_2d(n_density, n_comp, sizeof(**node->boxes));
	for (i = 0; i < n_density; ++i)
		for (j = 0; j < n_comp; ++j)
			node->boxes[i][j] = sqrt(-2 * node->variances[i][j] * log(threshold));
	node->is_root = 1;

	/* Initialize projection for root node. */
	node->upper = vector_alloc(n_comp);
	node->lower = vector_alloc(n_comp);
	for (j = 0; j < n_comp; ++j) {
		/* BIG NUMBERS */
		node->upper[j] = 1e-50;
		node->lower[j] = 1e+50;
	}
	/* Node projection is the union of all Gaussians. */
	for (i = 0; i < n_density; ++i) {
		for (j = 0; j < n_comp; ++j) {
			float32 a, b;

			a = node->means[i][j] - node->boxes[i][j];
			b = node->means[i][j] + node->boxes[i][j];
			if (node->lower[j] > a) node->lower[j] = a;
			if (node->upper[j] < b) node->upper[j] = b;
		}
	}

	build_kdtree_level(node, n_levels);

	return node;
}

static int
compare_float32(const void *a, const void *b)
{
	if ((*(float32 *)a) < (*(float32 *)b))
		return -1;
	if ((*(float32 *)a) > (*(float32 *)b))
		return 1;
	return 1;
}

static int
build_kdtree_level(kd_tree_node_t *node, uint32 n_levels)
{
	int i, j, k;
	uint32 best_split;

	printf("Gaussian box for node at level %d:\n", n_levels);
	printf(" (");
	for (j = 0; j < node->n_comp; ++j)
		printf("%.3f ", node->lower[j]);
	printf(")\n");
	printf(" (");
	for (j = 0; j < node->n_comp; ++j)
		printf("%.3f ", node->upper[j]);
	printf(")\nIntersects Gaussians: ");
	/* Find all gaussians that intersect the current node's projection. */
	node->bbi = ckd_calloc(node->n_density, sizeof(*node->bbi));
	for (k = i = 0; i < node->n_density; ++i) {
		for (j = 0; j < node->n_comp; ++j) {
			float32 a, b;

			a = node->means[i][j] - node->boxes[i][j];
			b = node->means[i][j] + node->boxes[i][j];

			/* Is it ouside the projection on some dimension? */
			if (a > node->upper[j] || b < node->lower[j]) {
				//printf("%d %d %f %f\n", i, j, a, b);
				goto next_density;
			}
		}
		/* Otherwise it intersects. */
		node->bbi[i] = 1;
		++k;
		printf("%d ", i);
	next_density:
		;
	}
	printf("(total %d)\n", k);

	/* Terminate the recursion. */
	if (--n_levels == 0)
		return 0;

	/* Now find the median hyperplane for each component. */
	best_split = 0x7fffffff;
	for (j = 0; j < node->n_comp; ++j) {
		float32 plane, *axis;
		uint32 split;

		axis = ckd_calloc(node->n_density*2, sizeof(*axis));
		for (i = 0, k = 0; i < node->n_density; ++i) {
			float32 a, b;

			if (node->bbi[i] == 0)
				continue;
			a = node->means[i][j] - node->boxes[i][j];
			b = node->means[i][j] + node->boxes[i][j];


#if 0
			printf("%d = %d (%f L)\n",
			       k, i, a);
			printf("%d = %d (%f R)\n",
			       k+1, i, b);
#endif

			axis[k++] = a;
			axis[k++] = b;
		}

		qsort(axis, k, sizeof(*axis), compare_float32);
#if 0
		for (i = 0; i < k; ++i) 
			printf("%d = %f\n", i, axis[i]);
#endif
		/* The hyperplane with the same number of Ls to the
		 * left as Rs to the right is just the median of all
		 * the Ls and Rs.  To see why this is, first note that
		 * the total number of Ls is the same as the total
		 * number of Rs which is k/2. Now consider that at any
		 * point n in the array you have l Ls to the the left,
		 * n-l Rs to the left, and thus r = k/2-n-l Rs to the
		 * right.  At n=k/2, r = k/2-n-l = l.  Therefore k/2
		 * is the point at which there is the equal number of
		 * Ls to the left as Rs to the right.
		 */
		plane = (k & 0x1) ? ((axis[k/2]+axis[k/2+1])/2) : axis[k/2];
		free(axis);
		
		/* How many Gaussian boxes does this plane split? */
		for (split = 0, i = 0; i < node->n_density; ++i) {
			float32 a, b;

			if (node->bbi[i] == 0)
				continue;
			a = node->means[i][j] - node->boxes[i][j];
			b = node->means[i][j] + node->boxes[i][j];

			if (a < plane && b > plane)
				++split;
		}
		printf("Component %d plane %f splits %d boxes\n",
		       j, plane, split);
		if (split < best_split) {
			best_split = split;
			node->split_idx = j;
			node->split_plane = plane;
		}
	}
	printf("Splitting node at component %d on plane %f\n",
	       node->split_idx, node->split_plane);

	node->left = ckd_calloc(1, sizeof(*node->left));
	memcpy(node->left, node, sizeof(*node->left));
	node->left->lower = vector_alloc(node->n_comp);
	node->left->upper = vector_alloc(node->n_comp);
	memcpy(node->left->lower, node->lower, sizeof(*node->left->lower) * node->n_comp);
	memcpy(node->left->upper, node->upper, sizeof(*node->left->upper) * node->n_comp);
	node->left->upper[node->split_idx] = node->split_plane;

	node->right = ckd_calloc(1, sizeof(*node->right));
	memcpy(node->right, node, sizeof(*node->right));
	node->right->lower = vector_alloc(node->n_comp);
	node->right->upper = vector_alloc(node->n_comp);
	memcpy(node->right->lower, node->lower, sizeof(*node->right->lower) * node->n_comp);
	memcpy(node->right->upper, node->upper, sizeof(*node->right->upper) * node->n_comp);
	node->right->lower[node->split_idx] = node->split_plane;

	build_kdtree_level(node->left, n_levels);
	build_kdtree_level(node->right, n_levels);

	return 0;
}

void
free_kd_tree(kd_tree_node_t *tree)
{
	if (tree == NULL)
		return;
	free_kd_tree(tree->left);
	free_kd_tree(tree->right);
	if (tree->is_root)
		ckd_free(tree->boxes);
	ckd_free(tree->bbi);
	ckd_free(tree->lower);
	ckd_free(tree->upper);
	ckd_free(tree);
}
