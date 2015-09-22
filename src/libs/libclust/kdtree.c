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
#include <sys_compat/misc.h>
#include <string.h>
#include <math.h>

static int build_kdtree_level(kd_tree_node_t *node, uint32 n_levels);
static int compare_float32(const void *a, const void *b);

#define KDTREE_VERSION 1

kd_tree_node_t *
build_kd_tree(const vector_t *means, const vector_t *variances,
	      uint32 n_density, uint32 n_comp,
	      float32 threshold, int32 n_levels,
	      int32 absolute)
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
	if (absolute) {
		for (i = 0; i < n_density; ++i) {
			float32 det = n_comp * log(2 * M_PI);

			for (j = 0; j < n_comp; ++j)
				det += log(node->variances[i][j]);
			for (j = 0; j < n_comp; ++j)
				node->boxes[i][j] = sqrt(-2 * node->variances[i][j]
							 * (threshold + 0.5 * det));
		}
	}
	else {
		for (i = 0; i < n_density; ++i)
			for (j = 0; j < n_comp; ++j)
				node->boxes[i][j] = sqrt(-2 * node->variances[i][j]
							 * log(threshold));
	}
	node->threshold = threshold;
	node->n_level = n_levels;

	/* Initialize projection for root node. */
	node->upper = (vector_t)ckd_calloc(n_comp, sizeof(float32));
	node->lower = (vector_t)ckd_calloc(n_comp, sizeof(float32));
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
	printf(")\n");

	/* Find all Gaussians that intersect the current node's projection. */
	node->bbi = ckd_calloc(node->n_density, sizeof(*node->bbi));
	for (k = i = 0; i < node->n_density; ++i) {
		for (j = 0; j < node->n_comp; ++j) {
			float32 a, b;

			a = node->means[i][j] - node->boxes[i][j];
			b = node->means[i][j] + node->boxes[i][j];

			/* Is it ouside the projection on some dimension? */
			if (a > node->upper[j] || b < node->lower[j]) {
				goto next_density;
			}
		}
		/* Otherwise it intersects. */
		node->bbi[i] = 1;
		++k;
	next_density:
		;
	}
	printf("Intersects %d Gaussians: ", k);
	for (i = 0; i < node->n_density; ++i)
		if (node->bbi[i])
			printf("%d ", i);
	printf("\n");

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

			axis[k++] = a;
			axis[k++] = b;
		}

		qsort(axis, k, sizeof(*axis), compare_float32);
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
			node->split_comp = j;
			node->split_plane = plane;
		}
	}
	printf("Splitting node at component %d on plane %f\n",
	       node->split_comp, node->split_plane);

	node->left = ckd_calloc(1, sizeof(*node->left));
	memcpy(node->left, node, sizeof(*node->left));
	node->left->left = node->left->right = NULL;
	node->left->n_level = 0; /* Mark it as non-root */
	node->left->lower = (vector_t)ckd_calloc(node->n_comp, sizeof(float32));
	node->left->upper = (vector_t)ckd_calloc(node->n_comp, sizeof(float32));
	memcpy(node->left->lower, node->lower, sizeof(*node->left->lower) * node->n_comp);
	memcpy(node->left->upper, node->upper, sizeof(*node->left->upper) * node->n_comp);
	node->left->upper[node->split_comp] = node->split_plane;

	node->right = ckd_calloc(1, sizeof(*node->right));
	memcpy(node->right, node, sizeof(*node->right));
	node->right->left = node->right->right = NULL;
	node->right->n_level = 0; /* Mark it as non-root */
	node->right->lower = (vector_t)ckd_calloc(node->n_comp, sizeof(float32));
	node->right->upper = (vector_t)ckd_calloc(node->n_comp, sizeof(float32));
	memcpy(node->right->lower, node->lower, sizeof(*node->right->lower) * node->n_comp);
	memcpy(node->right->upper, node->upper, sizeof(*node->right->upper) * node->n_comp);
	node->right->lower[node->split_comp] = node->split_plane;

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
	if (tree->n_level)
		ckd_free_2d(tree->boxes);
	ckd_free(tree->bbi);
	ckd_free(tree->lower);
	ckd_free(tree->upper);
	ckd_free(tree);
}

typedef struct bbi_bucket_s bbi_bucket_t;
struct bbi_bucket_s {
	uint32 idx;
	float32 box;
};

static int
compare_boxes(const void *a, const void *b)
{
	if (((bbi_bucket_t *)a)->box < ((bbi_bucket_t *)b)->box)
		return -1;
	if (((bbi_bucket_t *)a)->box > ((bbi_bucket_t *)b)->box)
		return 1;
	return 0;
}

static int32
write_bbi_list(FILE *fp, kd_tree_node_t *node)
{
	int i, j, k;
	bbi_bucket_t *bbi;

	bbi = ckd_calloc(node->n_density, sizeof(*bbi));
	for (i = 0, k = 0; i < node->n_density; ++i) {
		if (node->bbi[i]) {
			float64 box, inter;

			/* Sort them by the ratio of the size of the
			 * Gaussian box to the size of its overlap
			 * with the current node's projection. */
			bbi[k].idx = i;
			box = inter = 0.0;
			for (j = 0; j < node->n_comp; ++j)
				box += log(node->boxes[i][j] * 2);
			for (j = 0; j < node->n_comp; ++j) {
				float32 a, b;

				a = node->means[i][j] - node->boxes[i][j];
				b = node->means[i][j] + node->boxes[i][j];
				if (b >= node->upper[j])
					inter += log(node->upper[j] - a);
				else if (a <= node->lower[j])
					inter += log(b - node->lower[j]);
				else
					inter += log(node->boxes[i][j] * 2);
			}
			bbi[k].box = (float32)(box - inter);
			++k;
		}
	}
	qsort(bbi, k, sizeof(*bbi), compare_boxes);
	for (i = 0; i < k; ++i)
		fprintf(fp, "%d ", bbi[i].idx);
	ckd_free(bbi);
	return 0;
}

static int32
write_kd_nodes(FILE *fp, kd_tree_node_t *node, uint32 level)
{
	if (node == NULL)
		return 0;

	fprintf(fp, "NODE %d\n", level);
	fprintf(fp, "split_comp %d\n", node->split_comp);
	fprintf(fp, "split_plane %f\n", node->split_plane);
	fprintf(fp, "bbi ");
	write_bbi_list(fp, node);
	fprintf(fp, "\n\n");

	write_kd_nodes(fp, node->left, level-1);
	write_kd_nodes(fp, node->right, level-1);

	return 0;
}

int32
write_kd_trees(const char *outfile, kd_tree_node_t **trees, uint32 n_trees)
{
	FILE *fp;
	uint32 i;

	if ((fp = fopen(outfile, "w"))  == NULL) {
		E_ERROR_SYSTEM("Failed to open %s", outfile);
		return -1;
	}
	fprintf(fp, "KD-TREES\n");
	fprintf(fp, "version %d\n", KDTREE_VERSION);
	fprintf(fp, "n_trees %d\n", n_trees);
	for (i = 0; i < n_trees; ++i) {
		fprintf(fp, "TREE %d\n", i);
		fprintf(fp, "n_density %d\n", trees[i]->n_density);
		fprintf(fp, "n_comp %d\n", trees[i]->n_comp);
		fprintf(fp, "n_level %d\n", trees[i]->n_level);
		fprintf(fp, "threshold %f\n", trees[i]->threshold);
		/* Output the nodes in depth-first ordering */
		write_kd_nodes(fp, trees[i], trees[i]->n_level);
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}

static int32
read_tree_int(FILE *fp, const char *name, uint32 *out, int32 optional)
{
	char line[256];
	int n;

	n = fscanf(fp, "%255s %u", line, out);
	if ((optional == 0 && n != 2) || strcmp(line, name)) {
		E_ERROR("%s not found: %d %s %u\n", name, n, line, out);
		return -1;
	}
	return n;
}

static int32
read_tree_float(FILE *fp, const char *name, float32 *out, int32 optional)
{
	char line[256];
	int n;

	n = fscanf(fp, "%255s %f", line, out);
	if ((optional == 0 && n != 2) || strcmp(line, name)) {
		E_ERROR("%s not found: %d %s %f\n", name, n, line, out);
		return -1;
	}
	return n;
}

int32
read_kd_nodes(FILE *fp, kd_tree_node_t *node, uint32 level)
{
	uint32 n;
	int i;

	if (read_tree_int(fp, "NODE", &n, FALSE) < 0)
		return -1;
	if (n != level) {
		E_ERROR("Levels for node don't match (%d != %d)\n", n, level);
		return -1;
	}
	if (read_tree_int(fp, "split_comp", &node->split_comp, FALSE) < 0)
		return -1;
	if (read_tree_float(fp, "split_plane", &node->split_plane, FALSE) < 0)
		return -1;
	if ((i = read_tree_int(fp, "bbi", &n, TRUE)) < 0)
		return -1;
	node->bbi = ckd_calloc(node->n_density, sizeof(*node->bbi));
	if (i > 1) {
		if (n >= node->n_density) {
			E_ERROR("BBI Gaussian %d out of range! %d\n", n);
			return -1;
		}
		node->bbi[n] = 1;
		while ((i = fscanf(fp, "%d", &n))) {
			if (feof(fp))
				break;
			if (n >= node->n_density) {
				E_ERROR("BBI Gaussian %d out of range! %d\n", n, i);
				return -1;
			}
			node->bbi[n] = 1;
		}
	}

	if (level == 1)
		return 0;

	node->left = ckd_calloc(1, sizeof(*node->left));
	node->left->n_density = node->n_density;
	node->left->n_comp = node->n_comp;
	if (read_kd_nodes(fp, node->left, level-1) < 0)
		return -1;
	node->right = ckd_calloc(1, sizeof(*node->left));
	node->right->n_density = node->n_density;
	node->right->n_comp = node->n_comp;
	if (read_kd_nodes(fp, node->right, level-1) < 0)
		return -1;
	return 0;
}

int32
read_kd_trees(const char *infile, kd_tree_node_t ***out_trees, uint32 *out_n_trees)
{
	FILE *fp;
	char line[256];
	int n, version;
	uint32 i, m;

	if ((fp = fopen(infile, "r"))  == NULL) {
		E_ERROR_SYSTEM("Failed to open %s", infile);
		return -1;
	}
	n = fscanf(fp, "%255s", line);
	if (n != 1 || strcmp(line, "KD-TREES")) {
		E_ERROR("Doesn't appear to be a kd-tree file: %s\n");
		return -1;
	}
	n = fscanf(fp, "%255s %d", line, &version);
	if (n != 2 || strcmp(line, "version") || version > KDTREE_VERSION) {
		E_ERROR("Unsupported kd-tree file format %s %d\n", line, version);
		return -1;
	}
	if (read_tree_int(fp, "n_trees", out_n_trees, FALSE) < 0)
		return -1;

	*out_trees = ckd_calloc(*out_n_trees, sizeof(kd_tree_node_t **));
	for (i = 0; i < *out_n_trees; ++i) {
		kd_tree_node_t *tree;

		if (read_tree_int(fp, "TREE", &m, FALSE) < 0)
			goto error_out;
		if (m != i) {
			E_ERROR("Tree number %u out of sequence\n", m);
			goto error_out;
		}

		(*out_trees)[i] = tree = ckd_calloc(1, sizeof(*tree));
		if (read_tree_int(fp, "n_density", &tree->n_density, FALSE) < 0)
			goto error_out;
		if (read_tree_int(fp, "n_comp", &tree->n_comp, FALSE) < 0)
			goto error_out;
		if (read_tree_int(fp, "n_level", &tree->n_level, FALSE) < 0)
			goto error_out;
		if (read_tree_float(fp, "threshold", &tree->threshold, FALSE) < 0)
			goto error_out;
		if (read_kd_nodes(fp, tree, tree->n_level) < 0)
			goto error_out;
	}
	fclose(fp);
	return 0;

error_out:
	fclose(fp);
	for (i = 0; i < *out_n_trees; ++i) {
		free_kd_tree((*out_trees)[i]);
		(*out_trees)[i] = NULL;
	}
	ckd_free(*out_trees);
	*out_trees = NULL;
	return -1;
}
