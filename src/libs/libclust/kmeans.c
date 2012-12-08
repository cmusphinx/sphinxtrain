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
 * File: kmeans.c
 * 
 * Description: 
 *     Given a set of K mean vectors and any sample vector out of a
 *     corpus of vectors, label the sample vector with the mean vector
 *     index which minimizes the Eucl. distance between the two.
 *      
 * Author: 
 *     Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/profile.h>

#include <s3/kmeans.h>
#include <s3/s3.h>

#include <assert.h>

#ifndef NULL
#define NULL (void *)0
#endif

static vector_t (*get_obs)(uint32 i);

void k_means_set_get_obs(vector_t (*fn)(uint32 i))
{
    get_obs = fn;
}

static void nn_sort_kmeans(vector_t *mean,
			   uint32 n_mean,
			   uint32 veclen,
			   idx_dist_t **nnmap);


float64
k_means(vector_t *mean,			/* initial set of means */
	uint32 n_mean,			/* # of means (should be k_mean?) */
	uint32 n_obs,			/* # of observations */
	uint32 veclen,			/* vector length of means and corpus */
	float32 min_conv_ratio,
	uint32 max_iter,		/* If not converged by this count, just quit */
	codew_t **out_label)		/* The final labelling of the corpus according
					   to the adjusted means; if NULL passed, just
					   discarded. */
{
    uint32 i;
    float32 p_sqerr = MAX_POS_FLOAT32;
    float32 sqerr;
    float32 conv_ratio;
    codew_t *label;
    int ret;

    label = (codew_t *)ckd_calloc(n_obs, sizeof(codew_t));

    sqerr = k_means_label(label, mean, n_mean, n_obs, veclen);

    conv_ratio = (p_sqerr - sqerr) / p_sqerr;

    for (i = 0; (i < max_iter) && (conv_ratio > min_conv_ratio); i++) {
	E_INFO("kmtrineq iter [%u] %e ...\n", i, conv_ratio);

	ret = k_means_update(mean, n_mean, veclen, label, n_obs);
	if (ret != K_MEANS_SUCCESS)
	    return (float64)ret;

	p_sqerr = sqerr;
	sqerr = k_means_label(label, mean, n_mean, n_obs, veclen);

	conv_ratio = (p_sqerr - sqerr) / p_sqerr;
    }
    E_INFO("km n_iter %u sqerr %e conv_ratio %e\n", i, sqerr, conv_ratio);

    if (out_label) {
	*out_label = label;
    }
    else {
	ckd_free(label);
    }

    return sqerr;
}

float64
k_means_trineq(vector_t *mean,			/* initial set of means */
	       uint32 n_mean,			/* # of means (should be k_mean?) */

	       uint32 n_obs,			/* # of observations */
	       uint32 veclen,			/* vector length of means and corpus */
	       float32 min_conv_ratio,
	       uint32 max_iter,		/* If not converged by this count, just quit */
	       codew_t **out_label)		/* The final labelling of the corpus according
						   to the adjusted means; if NULL passed, just
						   discarded. */
{
    uint32 i;
    float32 p_sqerr = MAX_POS_FLOAT32;
    float32 sqerr;
    float32 conv_ratio;
    codew_t *label;
    int ret;
    idx_dist_t **nnmap;	/* for a given codeword,
			   ascendingly sorted list of its nearest neighbors.
			   i.e. nnmap[k][0] would be the nearest neighbor to
			   codeword k, etc. */
    
    label = (codew_t *)ckd_calloc(n_obs, sizeof(codew_t));

    sqerr = k_means_label(label, mean, n_mean, n_obs, veclen);

    conv_ratio = (p_sqerr - sqerr) / p_sqerr;

    nnmap = (idx_dist_t **)ckd_calloc_2d(n_mean, n_mean-1, sizeof(idx_dist_t));

    for (i = 0; (i < max_iter) && (conv_ratio > min_conv_ratio); i++) {
	E_INFO("km iter [%u] %e ...\n", i, conv_ratio);

	ret = k_means_update(mean, n_mean, veclen, label, n_obs);
	if (ret != K_MEANS_SUCCESS)
	    return (float64)ret;

	nn_sort_kmeans(mean, n_mean, veclen, nnmap);

	p_sqerr = sqerr;
	sqerr = k_means_label_trineq(label, mean, n_mean, nnmap, n_obs, veclen);

	conv_ratio = (p_sqerr - sqerr) / p_sqerr;
    }
    E_INFO("kmtrineq n_iter %u sqerr %e conv_ratio %e\n", i, sqerr, conv_ratio);

    ckd_free(nnmap);

    if (out_label) {
	*out_label = label;
    }
    else {
	ckd_free(label);
    }

    return sqerr;
}

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

float64
k_means_label(codew_t *label,
	      vector_t *mean,
	      uint32 n_mean,       /* # of mean vectors */
	      uint32 n_obs,   /* in # of vectors */
	      uint32 veclen)
{
    uint32 i, j, b_j, l;
    float64 t, d;
    float64 b_d;
    float64 sqerr;
    vector_t c;
    vector_t m;

    for (i = 0, sqerr = 0; i < n_obs; i++, sqerr += b_d) {
	c = get_obs(i);
	if (c == NULL) {
	    E_INFO("No observations for %u, but expected up through %u\n", i, n_obs-1);
	}

	/* Get an estimate of best distance (b_d) and codeword (b_j) */
	b_j = label[i];
	m = mean[b_j];
	for (l = 0, b_d = 0.0; l < veclen; l++) {
	    t = m[l] - c[l];
	    b_d += t * t;
	}

	for (j = 0; j < n_mean; j++) {
	    m = mean[j];

	    for (l = 0, d = 0.0; (l < veclen) && (d < b_d); l++) {
		t = m[l] - c[l];
		d += t * t;
	    }

	    if (d < b_d) {
		b_d = d;
		b_j = j;
	    }
	}

	label[i] = b_j;
    }

    return sqerr;
}
int
cmp_dist(const void *a, const void *b)
{
    const idx_dist_t *a_ = (idx_dist_t *)a;
    const idx_dist_t *b_ = (idx_dist_t *)b;

    if (a_->d < b_->d) {
	return -1;
    }
    else if (a_->d > b_->d) {
	return 1;
    }
    else {
	return 0;
    }
}
    
static void
nn_sort_kmeans(vector_t *mean,
	       uint32 n_mean,
	       uint32 veclen,
	       idx_dist_t **nnmap)
{
    uint32 i, j, k, l;
    vector_t ma, mb;
    float64 diff, d;

    for (i = 0; i < n_mean; i++) {
	ma = mean[i];

	for (j = 0, k = 0; j < n_mean; j++) {
	    if (i != j) {
		mb = mean[j];
		for (l = 0, d = 0; l < veclen; l++) {
		    diff = ma[l] - mb[l];
		    d += diff * diff;
		}

		nnmap[i][k].idx = j;
		nnmap[i][k].d = d;

		++k;
	    }
	}
	assert(k == (n_mean-1));
	qsort(&nnmap[i][0], n_mean-1, sizeof(idx_dist_t), cmp_dist);
    }
}

float64
k_means_label_trineq(codew_t *label,
		     vector_t *mean,
		     uint32 n_mean,       /* # of mean vectors */
		     idx_dist_t **nnmap,
		     uint32 n_obs,   /* in # of vectors */
		     uint32 veclen)
{
    uint32 i, eb_j, b_j, l, k;
    float64 t, d;
    float64 b_d, eb_d;
    float64 sqerr;
    vector_t c;
    vector_t m;
    idx_dist_t *nnmap_eb;

    for (i = 0, sqerr = 0; i < n_obs; i++) {
	c = get_obs(i);
	if (c == NULL) {
	    E_INFO("No observations for %u, but expected up through %u\n", i, n_obs-1);
	}

	/* Get an estimate of b_d */
	eb_j = label[i];
	m = mean[eb_j];
	for (l = 0, eb_d = 0.0; l < veclen; l++) {
	    t = m[l] - c[l];
	    eb_d += t * t;
	}

	nnmap_eb = nnmap[eb_j];
	b_d = eb_d;
	b_j = eb_j;

	for (k = 0; k < n_mean-1 && nnmap_eb[k].d <= 4.0 * eb_d; k++) {
	    m = mean[nnmap_eb[k].idx];

	    for (l = 0, d = 0.0; (l < veclen) && (d < b_d); l++) {
		t = m[l] - c[l];
		d += t * t;
	    }
	    
	    if (d < b_d) {
		b_j = nnmap_eb[k].idx;
		b_d = d;
	    }
	}

	sqerr += b_d;

	label[i] = b_j;
    }

    return sqerr;
}

#include <sphinxbase/ckd_alloc.h>

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

int
k_means_update(vector_t *mean,
	       uint32 n_mean,
	       uint32 veclen,
	       codew_t *label,
	       uint32 n_obs)
{
    uint32 i, j, l, *cnt;
    vector_t m;
    vector_t c;
    int ret = K_MEANS_SUCCESS;

    cnt = (uint32 *)ckd_calloc(n_mean, sizeof(uint32));

    for (i = 0; i < n_mean; i++) {
	for (l = 0; l < veclen; l++) {
	    mean[i][l] = 0.0;
	}
    }

    
    for (i = 0; i < n_obs; i++) {
	assert((0 <= label[i]) && (label[i] < n_mean));

	m = mean[label[i]];
	cnt[label[i]]++;

	c = get_obs(i);
	if (c == NULL) {
	    E_INFO("No observations for %u, but expected up through %u\n", i, n_obs-1);
	}

	for (l = 0; l < veclen; l++) {
	    m[l] += c[l];
	}
    }

    for (i = 0; i < n_mean; i++) {
	j = cnt[i];
	if (j != 0) {
	    for (l = 0; l < veclen; l++) {
		mean[i][l] /= (float32) j;
	    }
	}
	else {
	    E_WARN("Empty cluster %u\n", i);
	    ret = K_MEANS_EMPTY_CODEWORD;
	}
    }

    ckd_free(cnt);

    return ret;
}

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

float64
k_means_label_subset(codew_t *label,
		     vector_t *mean,
		     uint32 n_mean,       /* # of mean vectors */
		     uint32 *subset,
		     uint32 n_obs_subset,   /* in # of vectors */
		     uint32 veclen)
{
    uint32 i, j, b_j=0, l;
    float64 t, d;
    float64 b_d;
    float64 sqerr;
    vector_t c;
    vector_t m;

    for (i = 0, sqerr = 0; i < n_obs_subset; i++) {
	b_d = 1e300;

	c = get_obs(subset[i]);
	
	for (j = 0; j < n_mean; j++) {
	    m = mean[j];

	    for (l = 0, d = 0.0; (l < veclen) && (d < b_d); l++) {
		t = m[l] - c[l];
		d += t * t;
	    }

	    if (d < b_d) {
		b_d = d;
		b_j = j;
	    }
	}

	label[i] = b_j;

	sqerr += b_d;
    }

    return sqerr;
}

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

int
k_means_update_subset(vector_t *mean,
		      uint32 n_mean,
		      uint32 veclen,
		      uint32 *subset,
		      codew_t *label,
		      uint32 n_obs_subset)
{
    uint32 i, j, l, *cnt;
    vector_t m;
    vector_t c;
    int ret = K_MEANS_SUCCESS;

    cnt = (uint32 *)ckd_calloc(n_mean, sizeof(uint32));

    for (i = 0; i < n_mean; i++) {
	for (l = 0; l < veclen; l++) {
	    mean[i][l] = 0.0;
	}
    }

    for (i = 0; i < n_obs_subset; i++) {
	assert((0 <= label[i]) && (label[i] < n_mean));
	
	m = mean[label[i]];
	cnt[label[i]]++;
	
	c = get_obs(subset[i]);

	for (l = 0; l < veclen; l++) {
	    m[l] += c[l];
	}
    }

    for (i = 0; i < n_mean; i++) {
	j = cnt[i];
	if (j != 0) {
	    for (l = 0; l < veclen; l++) {
		mean[i][l] /= (float32) j;
	    }
	}
	else {
	    E_WARN("Empty cluster %u\n", i);
	    ret = K_MEANS_EMPTY_CODEWORD;
	}
    }

    ckd_free(cnt);

    return ret;
}

float64
k_means_subset(vector_t *mean,			/* initial set of means */
	       uint32 n_mean,			/* # of means (should be k_mean?) */

	       uint32 *subset,
	       uint32 n_obs_subset,		/* # of observations */
	       uint32 veclen,			/* vector length of means and corpus */
	       float32 min_conv_ratio,
	       uint32 max_iter,			/* If not converged by this count, just quit */
	       codew_t **out_label)		/* The final labelling of the corpus according
						   to the adjusted means; if NULL passed, just
						   discarded. */
{
    uint32 i;
    float32 p_sqerr = MAX_POS_FLOAT32;
    float32 sqerr;
    float32 conv_ratio;
    codew_t *label;
    int ret;
    
    label = (codew_t *)ckd_calloc(n_obs_subset, sizeof(codew_t));

    sqerr = k_means_label_subset(label, mean, n_mean, subset, n_obs_subset, veclen);
    conv_ratio = (p_sqerr - sqerr) / p_sqerr;
    E_INFO("sqerr %e conv_ratio %e\n", sqerr, conv_ratio);

    for (i = 0; (i < max_iter) && (conv_ratio > min_conv_ratio); i++) {
	ret = k_means_update_subset(mean, n_mean, veclen, subset, label, n_obs_subset);
	if (ret != K_MEANS_SUCCESS)
	    return (float64)ret;

	p_sqerr = sqerr;
	sqerr = k_means_label_subset(label, mean, n_mean, subset, n_obs_subset, veclen);
	conv_ratio = (p_sqerr - sqerr) / p_sqerr;
	E_INFO("sqerr %e conv_ratio %e\n", sqerr, conv_ratio);
    }

    if (out_label) {
	/* caller provided place to save the VQ labels */
	*out_label = label;
    }
    else {
	/* caller provided no way to save 'em, so chunk 'em */
	ckd_free(label);
    }

    return sqerr;
}
