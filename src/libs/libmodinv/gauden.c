/* -*- c-basic-offset: 4 -*- */
/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * File: gauden.c
 * 
 * Description: 
 * 	This module contains the functions to implement the evaluation
 *	of diagonal multivariate Gaussian densities:
 *
 *	$${1 \over \prod_{i=1}^d \sqrt{2 \pi} \sigma_i}
 *			e^{-{1 \over 2} \sum_{i=1}^d ({x_i - m_i \over \sigma_i})^2}$$
 *
 *	(just dump the above string into a TeX file)
 *
 *      And the functions for full multivariate Gaussian densities:
 *
 *      $\frac{1}{\sqrt{2 \pi^d ||\Sigma||}} \exp -\frac{1}{2}(x - \mu)\Sigma(x - \mu)^T$
 *		
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *      David Huggins-Daines (dhuggins@cs.cmu.edu)
 *********************************************************************/

#include <s3/gauden.h>

#include <sphinxbase/err.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/matrix.h>

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static float32 min_var = 1e38;	/* just a big num */

/* M_PI is not uniformly defined on all machines.  Do it here
 * to minimize unnecessary differences between results on
 * different systems */

#undef M_PI
#define M_PI       3.1415926535897932385E0

#if defined (_WIN32)
#define finite(x)    _finite(x)
#endif


int gauden_set_min_var(float32 m)
{
    min_var = m;

    E_INFO("min_var=%e\n", min_var);
    return S3_SUCCESS;
}

gauden_t *gauden_alloc()
{
    gauden_t *new;

    new = ckd_calloc(1, sizeof(gauden_t));

    return new;
}

void
gauden_free(gauden_t *g)
{
    /* free the utterance accumulators (if any) */
    gauden_free_l_acc(g);

    /* free the corpus accumulators (if any) */
    gauden_free_acc(g);

    /* free the means (if any) */
    if (g->mean)
	gauden_free_param(g->mean);
    g->mean = NULL;

    /* free the variances (if any) */
    if (g->var)
	gauden_free_param(g->var);
    if (g->fullvar)
	gauden_free_param_full(g->fullvar);
	    
    g->var = NULL;
    g->fullvar = NULL;

    /* free the density normalization terms */
    if (g->norm)
	ckd_free_3d((void ***)g->norm);
    g->norm = NULL;
    
    if (g->veclen)
	ckd_free(g->veclen);
    g->veclen = NULL;

    /* finally free the gauden structure itself */
    ckd_free(g);
}

int
gauden_set_feat(gauden_t *g,
		uint32 n_feat,
		const uint32 *veclen)
{
    uint32 i;

    g->n_feat = n_feat;
    g->veclen = ckd_calloc(n_feat, sizeof(uint32));
    memcpy(g->veclen, veclen, n_feat * sizeof(uint32));

    E_INFO("%d feature streams (", n_feat);
    for (i = 0; i < n_feat; i++) {
	E_INFOCONT("|%d|=%d ", i, g->veclen[i]);
    }
    E_INFOCONT(")\n");

    return S3_SUCCESS;
}

int
gauden_set_n_top(gauden_t *g,
		 uint32 n_t)
{
    g->n_top = n_t;

    E_INFO("compute %d densities/frame\n", n_t);

    return S3_SUCCESS;
}

int
gauden_set_n_mgau(gauden_t *g,
		  uint32 n)
{
    g->n_mgau = n;

    E_INFO("%d total mgau\n", n);
    
    return S3_SUCCESS;
}

int
gauden_set_n_density(gauden_t *g,
		     uint32 n)
{
    g->n_density = n;

    E_INFO("%d total densities\n", n);
    
    return S3_SUCCESS;
}

vector_t ***
gauden_alloc_param(uint32 n_cb,
		   uint32 n_feat,
		   uint32 n_density,
		   const uint32 *veclen)
{
    uint32 blksize;
    vector_t ***param;
    float32 *buf;
    uint32 i, j, k, l;

    for (blksize = 0, j = 0; j < n_feat; j++)
	blksize += veclen[j];

    param = (vector_t ***) ckd_calloc_3d(n_cb, n_feat, n_density,
					 sizeof(vector_t));
    
    buf = ckd_calloc(n_cb * n_density * blksize,
		     sizeof(float32));

    for (i = 0, l = 0; i < n_cb; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		param[i][j][k] = &buf[l];

		l += veclen[j];
	    }
	}
    }
    
    return param;
}

vector_t ****
gauden_alloc_param_full(uint32 n_cb,
			uint32 n_feat,
			uint32 n_density,
			const uint32 *veclen)
{
    uint32 blksize, maxveclen;
    vector_t ****param;
    float32 *buf;
    uint32 i, j, k, l, m;

    for (blksize = 0, maxveclen = 0, j = 0; j < n_feat; j++) {
	blksize += veclen[j];
	if (veclen[j] > maxveclen) maxveclen = veclen[j];
    }

    param = (vector_t ****) ckd_calloc_4d(n_cb, n_feat, n_density,
					  maxveclen, sizeof(vector_t));
    
    buf = ckd_calloc(n_cb * n_density * blksize * blksize,
		     sizeof(float32));

    for (i = 0, m = 0; i < n_cb; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    param[i][j][k][l] = &buf[m];

		    m += veclen[j];
		}
	    }
	}
    }
    
    return param;
}

int
gauden_set_const_dim_mgau(gauden_t *g,
			  uint32 n_m,
			  uint32 n_d)
{
    g->n_mgau = n_m;
    g->n_density = n_d;

    E_INFO("%d mgau, %d densities/mgau\n",
	   n_m, n_d);

    assert(g->n_feat > 0);
    assert(g->n_mgau > 0);
    assert(g->n_density > 0);

    g->mean = gauden_alloc_param(n_m, g->n_feat, n_d, g->veclen);
    g->var  = gauden_alloc_param(n_m, g->n_feat, n_d, g->veclen);
    
    return S3_SUCCESS;
}

vector_t ***
gauden_mean(gauden_t *g)
{
    return g->mean;
}

int
gauden_set_mean(gauden_t *g, vector_t ***mean)
{
    g->mean = mean;

    return S3_SUCCESS;
}

vector_t ***
gauden_var(gauden_t *g)
{
    return g->var;
}

int
gauden_set_var(gauden_t *g, vector_t ***variance)
{
    g->var = variance;

    return S3_SUCCESS;
}
vector_t ****
gauden_fullvar(gauden_t *g)
{
    return g->fullvar;
}

int
gauden_set_fullvar(gauden_t *g, vector_t ****variance)
{
    g->fullvar = variance;

    return S3_SUCCESS;
}

float32
diag_norm(vector_t var,
	  uint32 len)
{
    float32 log_det;
    float32 p;
    uint32 i;

    log_det = 0;

    for (i = 0; i < len; i++) {
	if (var[i] <= 0) {
	    E_FATAL("Sanity check var[i] > 0 failed: %e\n", var[i]);
	}
	log_det += log(var[i]);
    }

    p = len * log(2.0 * M_PI);

    return - 0.5 * (log_det + p);
}

float32
full_norm(vector_t *var,
	  uint32 len)
{
    float64 det;
    float32 p;

    det = determinant(var, len);
    /* Require covariance matrices to be positive-definite. */
    if (det <= 0) {
	E_ERROR("Covariance matrix is not positive-definite: determinant is %g\n",
		det);
	assert(det > 0);
    }
    p = len * log(2.0 * M_PI);

    return - 0.5 * (log(det) + p);
}

void
gauden_compute_norm(gauden_t *g)
{
    uint32 i, j, k;

    if (g->norm) {
	ckd_free_3d((void ***)g->norm);
    }

    g->norm  = (float32 ***) ckd_calloc_3d(g->n_mgau, g->n_feat, g->n_density,
					  sizeof(float32));

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_density; k++) {
		if (g->fullvar)
		    g->norm[i][j][k] = full_norm(g->fullvar[i][j][k], g->veclen[j]);
		else
		    g->norm[i][j][k] = diag_norm(g->var[i][j][k], g->veclen[j]);
	    }
	}
    }
}

static void
diag_and_floor(vector_t *fullvar, uint32 veclen, float32 floor)
{
    uint32 i, j;
    for (i = 0; i < veclen; ++i) {
	for (j = 0; j < veclen; ++j) {
	    if (i != j)
		fullvar[i][j] = 0.0;
	    else {
		if (fullvar[i][j] < floor)
		    fullvar[i][j] = floor;
	    }
	}
    }
}

int
gauden_floor_variance(gauden_t *g)
{
    uint32 i, j, k;

    if (g->fullvar != NULL) {
	/* Don't try to "floor" full covariances since there's no easy
	   way to do this.  If they are singular, discard the
	   off-diagonals and floor the diagonals. */
	for (i = 0; i < g->n_mgau; i++) {
	    for (j = 0; j < g->n_feat; j++) {
		for (k = 0; k < g->n_density; k++) {
		    if (determinant(g->fullvar[i][j][k], g->veclen[j]) <= 0.0) {
			diag_and_floor(g->fullvar[i][j][k], g->veclen[j], min_var);
		    }
		}
	    }
	}
    }
    if (g->var != NULL) {
	for (i = 0; i < g->n_mgau; i++) {
	    for (j = 0; j < g->n_feat; j++) {
		for (k = 0; k < g->n_density; k++) {
		    vector_floor(g->var[i][j][k], g->veclen[j], min_var);
		}
	    }
	}
    }

    return S3_SUCCESS;
}

void
diag_eval_precomp(vector_t var,
		  uint32 len)
{
    uint32 l;

    for (l = 0; l < len; l++) {
	var[l] = 1.0 / (2 * var[l]);
    }
}

int
gauden_double_variance(gauden_t *g)
{
    uint32 i, j, k, l;

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_density; k++) {
		for (l = 0; l < g->veclen[j]; l++) {
		    g->var[i][j][k][l] += g->var[i][j][k][l];
		}
	    }
	}
    }

    return S3_SUCCESS;
}

int
gauden_invert_variance(gauden_t *g)
{
    uint32 i, j, k, l;

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_density; k++) {
		for (l = 0; l < g->veclen[j]; l++) {
		    g->var[i][j][k][l] = 1.0 / g->var[i][j][k][l];
		}
	    }
	}
    }

    return S3_SUCCESS;
}

int
gauden_invert_variance_full(gauden_t *g)
{
    uint32 i, j, k;

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_density; k++) {
		/* Yes, this works in-place (we hope) */
		if (invert(g->fullvar[i][j][k], g->fullvar[i][j][k],
			   g->veclen[j]) != S3_SUCCESS) {
		    /* Shouldn't get here, due to the check in full_norm() */
		    E_FATAL("Singular covariance matrix ([%d][%d][%d]), can't continue!\n",
			    i, j, k);
		}
	    }
	}
    }

    return S3_SUCCESS;
}

/*
 * Precompute term 1 / (2 * \sigma_i ^ 2) and normalization factor (determinant
 * of covariance matrix).
 */
int
gauden_eval_precomp(gauden_t *g)
{
    gauden_compute_norm(g);	/* compute normalization factor for Gaussians */
    if (g->fullvar) {
	gauden_invert_variance_full(g);
    }
    else {
	gauden_double_variance(g);	/* pre-multiply variances by 2 for EXP dnom */
	gauden_invert_variance(g);	/* compute 1/(2 sigma^2) terms */
    }
    
    return S3_SUCCESS;
}

static vector_t ***
alloc_acc(gauden_t *g, uint32 n_id)
{
    uint32 i, j, k, l, vl, tl;
    float32 *b;
    vector_t ***out;

    for (i = 0, tl = 0; i < g->n_feat; i++) {
	tl += g->n_density * g->veclen[i];
    }

    tl *= n_id;

    b = ckd_calloc(tl, sizeof(float32));

    out = (vector_t ***) ckd_calloc_3d(n_id,
				      g->n_feat,
				      g->n_density,
				      sizeof(vector_t));

    for (i = 0, l = 0; i < n_id; i++) {
	for (j = 0; j < g->n_feat; j++) {

	    vl = g->veclen[j];

	    for (k = 0; k < g->n_density; k++) {
		out[i][j][k] = &b[l];
		
		l += vl;
	    }
	}
    }

    assert(l == tl);

    return out;
}

static vector_t ****
alloc_acc_full(gauden_t *g, uint32 n_id)
{
    /* FIXME: Shouldn't this work above too?  Let's try it... */
    return gauden_alloc_param_full(n_id, g->n_feat, g->n_density, g->veclen);
}

void
gauden_free_acc(gauden_t *g)
{
    if (g->macc) {
	gauden_free_param(g->macc);
    }
    g->macc = NULL;

    if (g->vacc) {
	gauden_free_param(g->vacc);
    }
    g->vacc = NULL;

    if (g->fullvacc) {
	gauden_free_param_full(g->fullvacc);
    }
    g->fullvacc = NULL;

    if (g->dnom) {
	ckd_free_3d((void ***)g->dnom);
    }
    g->dnom = NULL;
}

int32
gauden_alloc_acc(gauden_t *g)
{
    if (cmd_ln_boolean("-meanreest") == TRUE) {
	g->macc = alloc_acc(g, g->n_mgau);
    }
    if (cmd_ln_boolean("-varreest") == TRUE) {
	if (cmd_ln_int32("-fullvar") == TRUE)
	    g->fullvacc = alloc_acc_full(g, g->n_mgau);
	else
	    g->vacc = alloc_acc(g, g->n_mgau);
    }
    
    if (cmd_ln_boolean("-meanreest") == TRUE) {
	g->dnom = (float32  ***) ckd_calloc_3d(g->n_mgau,
					       g->n_feat,
					       g->n_density,
					       sizeof(float32));
    }
    
    return S3_SUCCESS;
}

void
gauden_free_l_acc(gauden_t *g)
{
    if (g->l_macc) {
	gauden_free_param(g->l_macc);
    }
    g->l_macc = NULL;

    if (g->l_vacc) {
	gauden_free_param(g->l_vacc);
    }
    g->l_vacc = NULL;

    if (g->l_fullvacc) {
	gauden_free_param_full(g->l_fullvacc);
    }
    g->l_fullvacc = NULL;

    if (g->l_dnom) {
	ckd_free_3d((void ***)g->l_dnom);
    }
    g->l_dnom = NULL;
}

int32
gauden_alloc_l_acc(gauden_t *g, uint32 n_lcl,
		   int32 mean_reest,
		   int32 var_reest,
		   int32 fullvar)
{
    /* free any local accumulators from the prior utterance */
    gauden_free_l_acc(g);

    if (mean_reest) {
	/* allocate mean accumulators */
	assert(g->l_macc == NULL);
	g->l_macc = alloc_acc(g, n_lcl);
    }
    if (var_reest) {
	/* allocate variance accumulators */
	if (fullvar) {
	    assert(g->l_fullvacc == NULL);
	    g->l_fullvacc = alloc_acc_full(g, n_lcl);
	}
	else {
	    assert(g->l_vacc == NULL);
	    g->l_vacc = alloc_acc(g, n_lcl);
	}
    }
    if (mean_reest || var_reest) {
	/* allocate mean/var normalization accumulators */
	assert(g->l_dnom == NULL);
	g->l_dnom = (float32  ***) ckd_calloc_3d(n_lcl,
						 g->n_feat,
						 g->n_density,
						 sizeof(float32));
    }

    return S3_SUCCESS;
}

vector_t ***gauden_l_macc(gauden_t *g)
{
    return g->l_macc;
}

vector_t ***gauden_l_vacc(gauden_t *g)
{
    return g->l_vacc;
}

vector_t ****gauden_l_fullvacc(gauden_t *g)
{
    return g->l_fullvacc;
}

float32 ***gauden_l_dnom(gauden_t *g)
{
    return g->l_dnom;
}

void gauden_free_param(vector_t ***p)
{
    ckd_free(p[0][0][0]);
    ckd_free_3d((void ***)p);
}

void gauden_free_param_full(vector_t ****p)
{
    ckd_free(p[0][0][0][0]);
    ckd_free_4d((void ****)p);
}

#if 0
static void
dist_unrolled(float32 *out,
	      uint32 *w_idx,
	      uint32 n_top,
	      uint32 n_density,
	      uint32 veclen,
	      vector_t obs,
	      vector_t *mean,
	      vector_t *var,
	      float32 *norm)
{
    uint32 i, j, k;
    vector_t m;
    vector_t v;
    vector_t o;
    float32 diff;
    float64 dist;
    float32 worst;
    
    assert (n_top > 0);
    assert (n_top <= n_density);
    assert (n_density > 0);
    assert (n_density < 4000000);
    assert (veclen > 0);
    assert (veclen < 4000000);
    
    for (i = 0; i < n_top; i++) {
	out[i] = MIN_IEEE_NORM_NEG_FLOAT32;
    }

    worst = out[n_top-1];

    for (i = 0; i < n_density; i++) {
	m = mean[i];
	v = var[i];
	o = obs;
	
	dist = norm[i];

	j = veclen;

	if (j & 1) {
	    diff = o[0] - m[0];
	    dist -= (diff * diff) * (v[0]);
	    o++;
	    m++;
	    v++;
	    j--;
	}

	
	if (j & 2) {
	    diff = o[0] - m[0];
	    dist -= (diff * diff) * (v[0]);
	    diff = o[1] - m[1];
	    dist -= (diff * diff) * (v[1]);
	    o += 2;
	    m += 2;
	    v += 2;
	    j -= 2;
	}
	
	if (j & 4) {
	    diff = o[0] - m[0];
	    dist -= (diff * diff) * (v[0]);
	    diff = o[1] - m[1];
	    dist -= (diff * diff) * (v[1]);
	    diff = o[2] - m[2];
	    dist -= (diff * diff) * (v[2]);
	    diff = o[3] - m[3];
	    dist -= (diff * diff) * (v[3]);
	    o += 4;
	    m += 4;
	    v += 4;
	    j -= 4;
	}

	while ((j > 0) && (dist > worst)) {
	    diff = o[0] - m[0];
	    dist -= (diff * diff) * (v[0]);
	    diff = o[1] - m[1];
	    dist -= (diff * diff) * (v[1]);
	    diff = o[2] - m[2];
	    dist -= (diff * diff) * (v[2]);
	    diff = o[3] - m[3];
	    dist -= (diff * diff) * (v[3]);
	    
	    diff = o[4] - m[4];
	    dist -= (diff * diff) * (v[4]);
	    diff = o[5] - m[5];
	    dist -= (diff * diff) * (v[5]);
	    diff = o[6] - m[6];
	    dist -= (diff * diff) * (v[6]);
	    diff = o[7] - m[7];
	    dist -= (diff * diff) * (v[7]);
	    o += 8;
	    m += 8;
	    v += 8;
	    j -= 8;
	}
	
	if (dist > worst) {
	    for (j = 0; j < n_top; j++)
		if (out[j] > dist)
		    break;
	    
	    for (k = n_top-1; k > j; k--) {
		out[k] = out[k-1];
		w_idx[k] = w_idx[k-1];
	    }
	    
	    out[k] = dist;
	    w_idx[k] = i;
	    
	    worst = out[n_top-1];
	}
    }
}
#endif

/* This is a most used function during the training. Be very careful
 * when you modify it */
float64
log_diag_eval(vector_t obs,
	      float32 norm,
	      vector_t mean,
	      vector_t var_fact,
	      uint32 veclen)
{
    float64 d = 0.0, diff;
    uint32 l;

    if (veclen == 39) {
	/* Most common case, optimized for loop unrolling */
        for (l = 0; l < 39; l++) {
	    diff = obs[l] - mean[l];
	    d += var_fact[l] * diff * diff;	/* compute -1 / (2 sigma ^2) * (x - m) ^ 2 terms */
	}
    } else if (veclen == 32) {
	/* Most common case, optimized for loop unrolling */
        for (l = 0; l < 32; l++) {
	    diff = obs[l] - mean[l];
	    d += var_fact[l] * diff * diff;	/* compute -1 / (2 sigma ^2) * (x - m) ^ 2 terms */
	}
    } else {
        for (l = 0; l < veclen; l++) {
	    diff = obs[l] - mean[l];
	    d += var_fact[l] * diff * diff;	/* compute -1 / (2 sigma ^2) * (x - m) ^ 2 terms */
	}
    }
    
    return norm - d;	/* log (1 / 2 pi |sigma^2|) */
}

float64
log_full_eval(vector_t obs,
	      float32 norm,
	      vector_t mean,
	      vector_t *var_inv,
	      uint32 veclen)
{
    float64 *diff, *vtmp, d;
    uint32 i, j;
    
    d = norm;

    /* Precompute x-m */
    diff = ckd_malloc(veclen * sizeof(float64));
    for (i = 0; i < veclen; i++)
	diff[i] = obs[i] - mean[i];

    /* Compute -0.5 * (x-m) * sigma^-1 * (x-m)^T */
    /* FIXME: We could probably use BLAS for this, though it is
     * unclear if that would actually be faster (particularly with
     * refblas) */
    vtmp = ckd_calloc(veclen, sizeof(float64));
    for (i = 0; i < veclen; ++i)
	for (j = 0; j < veclen; ++j)
	    vtmp[j] += var_inv[i][j] * diff[i];
    for (i = 0; i < veclen; ++i)
	d -= 0.5 * diff[i] * vtmp[i];
    ckd_free(vtmp);
    ckd_free(diff);

    return d;
}

/*********************************************************************
 *
 * Function: 
 * 	log_full_densities
 *
 * Description: 
 * 	This function computes the logs of the component densities in
 *	a mixture density.  It computes the full density for the case
 *	where the number of component densities is small.  Another option
 *	(implemented by log_topn_densities() ) is to compute only the
 *	largest N of a full mixture Gaussian.
 *
 * Function Inputs: 
 * 
 *	float32 *den -
 *		This function will write the component density values
 *		for the given feature vector to this argument.
 *
 *	uint32  *den_idx -
 *		The component density indices are written to this
 *		argument.
 *
 *	uint32   n_density -
 *		This is the total number of component densities
 *		for the mixture.
 *
 *	uint32   veclen -
 *		This is the length of the feature vector.
 *
 *	vector_t obs -
 *		This is a feature vector observed at some time.
 *
 *	vector_t *mean -
 *		These are the means of the component densities.
 *
 *	vector_t *var -
 *		These are the variances of the component densities.
 *
 *	float32  *log_norm -
 *		These are the normalization factors to apply to the
 *		densities 
 *
 * Global Inputs: 
 * 	None
 *
 * Return Values: 
 * 	None
 *
 * Global Outputs: 
 * 	None
 *
 * Pre-conditions:
 *	There must be sufficient space for the densities to be written.
 * 
 * Errors: 
 * 
 *********************************************************************/

static void
log_full_densities(float64 *den,
		   uint32  *den_idx,	/* the indices of the component densities */
		   uint32   n_density,	/* The number of component densities of the mixture */
		   uint32   veclen,	/* the length of the feature vector */
		   vector_t obs,	/* A feature vector observed at some time */
		   vector_t *mean,	/* means of the mixture density */
		   vector_t *var,	/* variances of the mixture density */
		   float32  *log_norm)	/* normalization factor for density */
{
    uint32 i;
    
    for (i = 0; i < n_density; i++) {
	den[i] = log_diag_eval(obs, log_norm[i], mean[i], var[i], veclen);
	den_idx[i] = i;
    }
}

static void
log_full_densities_full(float64 *den,
			uint32  *den_idx,
			uint32   n_density,
			uint32   veclen,
			vector_t obs,
			vector_t *mean,
			vector_t **var,
			float32  *log_norm)
{
    uint32 i;
    
    for (i = 0; i < n_density; i++) {
	den[i] = log_full_eval(obs, log_norm[i], mean[i], var[i], veclen);
	den_idx[i] = i;
    }
}

static void
log_topn_densities(float64 *den,
		   uint32 *den_idx,
		   uint32 n_top,
		   uint32 n_density,
		   uint32 veclen,
		   vector_t obs,
		   vector_t *mean,
		   vector_t *var,
		   float32 *log_norm,
		   uint32 *prev_den_idx)
{
    uint32 i, j, k;
    vector_t m;
    vector_t v;
    float64 diff;
    float64 d;
    float64 worst;	/* worst density value of the top N density values
			   seen so far */

    /* Initialize topn using the previous frame's top codeword indices */
    if (prev_den_idx) {
	for (j = 0; j < n_top; j++) {
	    i = prev_den_idx[j];
	    d = log_diag_eval(obs, log_norm[i],
			      mean[i], var[i], veclen);
	    den_idx[j] = i;
	    den[j] = d;
	    if (j == 0)
		continue;
	    /* Insertion sort them */
	    for (k = j; k > 0 && d > den[k-1]; --k) {
		den_idx[k] = den_idx[k-1];
		den[k] = den[k-1];
	    }
	    den_idx[k] = i;
	    den[k] = d;
	}
    }
    else {
	for (j = 0; j < n_top; j++) {
	    den[j] = MIN_IEEE_NORM_NEG_FLOAT64;
	    den_idx[j] = n_density + 1; /* A non-negative invalid value */
	}
    }

    worst = den[n_top-1];

    for (i = 0; i < n_density; i++) {
	m = mean[i];
	v = var[i];

	/* Initialize distance with the log of the normalization
	   factor for this pdf. */

	d = log_norm[i];

	for (j = 0; (j < veclen) && (d > worst); j++) {
	    diff = obs[j] - m[j];
	    d -= diff * diff * v[j];
	}
	/* Not in topn, so keep going */
	if (j < veclen || d <= worst)
	    continue;

	/* This may already have been in topn from the initialization pass */
	for (j = 0; j < n_top; j++)
	    if (den_idx[j] == i)
		break;
	if (j < n_top)
	    continue; /* It's already there, don't insert it */
	for (k = n_top-1; k > 0 && d > den[k-1]; --k) {
	    den_idx[k] = den_idx[k-1];
	    den[k] = den[k-1];
	}
	den_idx[k] = i;
	den[k] = d;

	worst = den[n_top-1];
    }
    worst = den[n_top-1];
}

static void
euclidean_dist(float64 *out,
	       uint32 *w_idx,
	       uint32 n_top,
	       uint32 n_density,
	       uint32 veclen,
	       vector_t obs,
	       vector_t *mean)
{
    uint32 i, j, k;
    vector_t m;
    float64 diff;
    float64 dist;
    float64 worst;	/* worst density value of the top N density values
			   seen so far */

    for (i = 0; i < n_top; i++) {
	out[i] = MAX_IEEE_NORM_POS_FLOAT64;
    }

    worst = out[n_top-1];

    for (i = 0; i < n_density; i++) {
	m = mean[i];

	dist = 0;

	for (j = 0; (j < veclen) && (dist < worst); j++) {
	    diff = obs[j] - m[j];
	    dist += diff * diff;
	}

	if (dist < worst) {
	    /* codeword i is closer to observation than the worst seen */

	    /* find the insertion point for the new codeword & distance */
	    for (j = 0; j < n_top; j++)
		if (dist < out[j])
		    break;
	    
	    /* make room for the new entry */
	    for (k = n_top-1; k > j; k--) {
		out[k] = out[k-1];
		w_idx[k] = w_idx[k-1];
	    }

	    assert(j == k);

	    /* insert the new distance */
	    out[k] = dist;
	    w_idx[k] = i;

	    worst = out[n_top-1];
	}
    }
}

int
gauden_compute(float64 **den,		/* density array for a mixture Gaussian */
	       uint32 **den_idx,	/* density index array for n_top < n_density eval */
	       vector_t *obs,		/* observation vector for some time */
	       gauden_t *g,		/* Gaussian density structure */
	       uint32 mgau,		/* id of the mixture Gau. to evaluate */
	       uint32 **prev_den_idx)   /* Previous frame's top N densities (or NULL) */
{
    uint32 j, k;

    /* make sure this is true at initialization time */
    assert(g->n_top <= g->n_density);

    /* Top-N computation not (yet) possible for full covariances */
    if (g->fullvar) {
	assert(g->n_top == g->n_density);
	for (j = 0; j < g->n_feat; j++) {
	    log_full_densities_full(den[j],
			       den_idx[j],
			       g->n_density,
			       g->veclen[j],
			       obs[j],
			       g->mean[mgau][j],
			       g->fullvar[mgau][j],
			       g->norm[mgau][j]);

	    for (k = 0; k < g->n_density; k++) {
		den[j][k] = exp( den[j][k] );
	    }
	}
    }
    else if (g->n_top == g->n_density) {
	for (j = 0; j < g->n_feat; j++) {
	    log_full_densities(den[j],
			       den_idx[j],
			       g->n_density,
			       g->veclen[j],
			       obs[j],
			       g->mean[mgau][j],
			       g->var[mgau][j],
			       g->norm[mgau][j]);

	    for (k = 0; k < g->n_density; k++) {
		den[j][k] = exp( den[j][k] );
	    }
	}
    }
    else {
	assert(g->n_top < g->n_density);

	for (j = 0; j < g->n_feat; j++) {
	    log_topn_densities(den[j],
			       den_idx[j],
			       g->n_top,
			       g->n_density,
			       g->veclen[j],
			       obs[j],
			       g->mean[mgau][j],
			       g->var[mgau][j],
			       g->norm[mgau][j],
			       prev_den_idx ? prev_den_idx[j] : NULL);

	    for (k = 0; k < g->n_top; k++) {
		den[j][k] = exp( den[j][k] );
	    }
	}
    }


#ifdef GAUDEN_VERBOSE
    gauden_print(stdout,
		 den, den_idx, g,
		 global_map, n_global_map);
#endif

    return S3_SUCCESS;
}

int
gauden_compute_log(float64 **den,		/* density array for a mixture Gaussian */
		   uint32 **den_idx,	/* density index array for n_top < n_density eval */
		   vector_t *obs,		/* observation vector for some time */
		   gauden_t *g,		/* Gaussian density structure */
		   uint32 mgau,		/* id of the mixture Gau. to evaluate */
		   uint32 **prev_den_idx)   /* Previous frame's top N densities (or NULL) */
{
    uint32 j;

    /* make sure this is true at initialization time */
    assert(g->n_top <= g->n_density);

    /* Top-N computation not (yet) possible for full covariances */
    if (g->fullvar) {
	assert(g->n_top == g->n_density);
	for (j = 0; j < g->n_feat; j++) {
	    log_full_densities_full(den[j],
			       den_idx[j],
			       g->n_density,
			       g->veclen[j],
			       obs[j],
			       g->mean[mgau][j],
			       g->fullvar[mgau][j],
			       g->norm[mgau][j]);
	}
    }
    else if (g->n_top == g->n_density) {
	for (j = 0; j < g->n_feat; j++) {
	    log_full_densities(den[j],
			       den_idx[j],
			       g->n_density,
			       g->veclen[j],
			       obs[j],
			       g->mean[mgau][j],
			       g->var[mgau][j],
			       g->norm[mgau][j]);
	}
    }
    else {
	assert(g->n_top < g->n_density);

	for (j = 0; j < g->n_feat; j++) {
	    log_topn_densities(den[j],
			       den_idx[j],
			       g->n_top,
			       g->n_density,
			       g->veclen[j],
			       obs[j],
			       g->mean[mgau][j],
			       g->var[mgau][j],
			       g->norm[mgau][j],
			       prev_den_idx ? prev_den_idx[j] : NULL);
	}
    }


#ifdef GAUDEN_VERBOSE
    gauden_print(stdout,
		 den, den_idx, g,
		 global_map, n_global_map);
#endif

    return S3_SUCCESS;
}

float64 *
gauden_scale_densities_fwd(float64 ***den,		/* density array for a mixture Gaussian */
			   uint32 ***den_idx,
			   uint32 *cb,		/* Codebook indices */
			   uint32 n_cb,		/* total # of codebooks to scale */
			   gauden_t *g)		/* Gaussian density structure */
{
    uint32 i, c, j, k;
    float64 *max_den;

    /* make sure this is true at initialization time */
    assert(g->n_top <= g->n_density);

    max_den = ckd_calloc(g->n_feat, sizeof(float64));

    /* Initialize max_den to some value in the domain */
    for (j = 0; j < g->n_feat; j++) {
	max_den[j] = MIN_IEEE_NORM_NEG_FLOAT64;
    }

    /* Find the max for each feature */
    for (i = 0; i < n_cb; i++) {
	c = cb[i];
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_top; k++) {
		if (max_den[j] < den[c][j][k]) {
		    max_den[j] = den[c][j][k];
		}
	    }
	}
    }

    for (j = 0; j < g->n_feat; j++) {
	max_den[j] -= MAX_LOG_DEN;
    }
    
    /* Subtract/Divide max density from others */
    for (i = 0; i < n_cb; i++) {
	c = cb[i];
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_top; k++) {
		den[c][j][k] = exp(den[c][j][k] - max_den[j]);
	    }
	}
    }

    return max_den;
}

/* log(MIN_IEEE_NORM_POS_FLOAT64) */
#define MINUS_LOG_INFINITY -708.407751281802

int
gauden_scale_densities_bwd(float64 ***den,		/* density array for a mixture Gaussian */
			   uint32 ***den_idx,
			   float64 **scale,
			   uint32 *cb,		/* Codebook indices */
			   uint32 n_cb,		/* total # of codebooks to scale */
			   gauden_t *g)		/* Gaussian density structure */
{
    uint32 i, c, j, k;
    float64 *scl;

    /* make sure this is true at initialization time */
    assert(g->n_top <= g->n_density);

    scl = *scale;

    /* Subtract/Divide max density from others */
    for (i = 0; i < n_cb; i++) {
	c = cb[i];
	for (j = 0; j < g->n_feat; j++) {
	    if (scl[j] <= MINUS_LOG_INFINITY) {
		E_WARN("Scaling factor too small: %f\n", scl[j]);
		scl[j] = MINUS_LOG_INFINITY + MAX_LOG_DEN;
	    }
	    for (k = 0; k < g->n_top; k++) {
		den[c][j][k] = exp(den[c][j][k] - scl[j]);
		assert(finite(den[c][j][k]));
	    }
	}
    }
		
    return S3_SUCCESS;
}

int
gauden_compute_euclidean(float64 ***out,
			 uint32 ***w_idx,
			 vector_t *obs,
			 gauden_t *g)
{
    uint32 i, j;

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {

	    euclidean_dist(out[i][j],
			   w_idx[i][j],
			   g->n_top,
			   g->n_density,
			   g->veclen[j],
			   obs[j],
			   g->mean[i][j]);
	    
#ifdef GAUDEN_VERBOSE
	    gauden_print(stdout,
			 out, w_idx, g,
			 NULL, 0);
#endif
	}
    }

    return S3_SUCCESS;
}

void
gauden_print(FILE *fp,
	     float64 ***den,
	     uint32 ***den_idx,
	     gauden_t *g,
	     uint32 *inv,
	     uint32 n_inv)
{
    uint32 i, ii, j, k;

    if (inv == NULL) {
	for (i = 0; i < g->n_mgau; i++) {
	    for (j = 0; j < g->n_feat; j++) {
		for (k = 0; k < g->n_top; k++) {
		    fprintf(fp, "(%1u %1u %1u)= [%3u] %.2e\n",
			    i, j, k, den_idx[i][j][k], den[i][j][k]);
		}
	    }
	}
    }
    else {
	for (ii = 0; ii < n_inv; ii++) {

	    i = inv[ii];
	    assert(i < g->n_mgau);

	    for (j = 0; j < g->n_feat; j++) {
		for (k = 0; k < g->n_top; k++) {
		    fprintf(fp, "(%1u %1u %1u)= [%3u] %.2e\n",
			    i, j, k, den_idx[i][j][k], den[i][j][k]);
		}
	    }
	}
    }

    fflush(fp);
}

int
gauden_normalize(float64 ***out,
		 gauden_t *g)
{
    float64 sum;
    uint32 i, j, k;
    int ret = S3_SUCCESS;

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {

	    sum = 0;

	    for (k = 0; k < g->n_top; k++) {
		sum += out[i][j][k];
	    }

	    if (sum != 0) {
		for (k = 0; k < g->n_top; k++) {
		    out[i][j][k] /= sum;
		}
	    }
	    else {
		E_ERROR("densities for feature %u sum to zero\n", j);

		return S3_ERROR;
	    }
	}
    }

    return ret;
}

#define MAX_N_FEAT	8

/*
 * Compute a mixture density given
 * the component density values for a given
 * state and time and the density weights
 * for a state.
 */
float64
gauden_mixture(float64 **den,
	       uint32 **den_idx,
	       float32 **w,
	       gauden_t *g)
{
    float64 out;
    uint32 j;

    out = 1.0;
    for (j = 0; j < g->n_feat; j++) {
	uint32 kk;
	float64 oprob;

	oprob = 0;
	kk = 0;
 	while (kk < g->n_top) {
	    uint32 k;
	    k = den_idx[j][kk];
	    oprob += w[j][k] * den[j][kk];
	    ++kk;
	}

	out *= oprob;
    }

    return out;
}

void
gauden_accum_param(vector_t ***out,
		   vector_t ***in,
		   uint32 n_mgau,
		   uint32 n_feat,
		   uint32 n_density,
		   const uint32 *veclen)
{
    uint32 i, j, k, l;

    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    out[i][j][k][l] += in[i][j][k][l];
		}
	    }
	}
    }
}

void
gauden_accum_param_full(vector_t ****out,
			vector_t ****in,
			uint32 n_mgau,
			uint32 n_feat,
			uint32 n_density,
			const uint32 *veclen)
{
    uint32 i, j, k, l, ll;

    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    for (ll = 0; ll < veclen[j]; ll++) {
			out[i][j][k][l][ll] += in[i][j][k][l][ll];
		    }
		}
	    }
	}
    }
}

void
gauden_norm_wt_mean(vector_t ***in_mean,
		    vector_t ***wt_mean,
		    float32 ***dnom,
		    uint32 n_mgau,
		    uint32 n_feat,
		    uint32 n_density,
		    const uint32 *veclen)
{
    uint32 i, j, k, l;

    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		if (dnom[i][j][k] != 0) {
		    for (l = 0; l < veclen[j]; l++) {
			wt_mean[i][j][k][l] /= dnom[i][j][k];
		    }
		}
		else {
		    E_WARN("(mgau= %u, feat= %u, density= %u) never observed\n",
			   i, j, k);
		    if (in_mean) {
			E_INFO("Copying it from in_mean\n");
			for (l = 0; l < veclen[j]; l++) {
			    wt_mean[i][j][k][l] = in_mean[i][j][k][l];
			}
		    }
		}
	    }
	}
    }
}

static void
gauden_tie_vars_dnoms(vector_t ***wt_var,
		      int32 pass2var,
		      float32 ***dnom,
		      vector_t ***mean,
		      uint32 n_mgau,
		      uint32 n_feat,
		      uint32 n_density,
		      const uint32 *veclen)
{
    uint32 i, j, k, l;

    /* Tie all variances and dnoms together. */
    /* First accumulate first codebook */
    for (j = 0; j < n_feat; j++) {
	if (!pass2var) {
	    /* Unnormalize first mean */
	    for (l = 0; l < veclen[j]; l++) {
		mean[0][j][0][l] *= dnom[0][j][0];
	    }
	}
	for (k = 1; k < n_density; k++) {
	    for (l = 0; l < veclen[j]; l++) {
		wt_var[0][j][0][l] += wt_var[0][j][k][l];
		if (!pass2var) {
		    /* Accumulate unnormalized means */
		    mean[0][j][0][l] += mean[0][j][k][l] * dnom[0][j][k];
		}
	    }
	    dnom[0][j][0] += dnom[0][j][k];
	}
    }
    /* Now add other codebooks to it */
    for (i = 1; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    wt_var[0][j][0][l] += wt_var[i][j][k][l];
		    if (!pass2var) {
			mean[0][j][0][l] += mean[i][j][k][l] * dnom[i][j][k];
		    }
		}
		dnom[0][j][0] += dnom[i][j][k];
	    }
	}
    }
    /* Now spread everything around */
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    if (!pass2var) {
			if (i == 0 && k == 0) { /* Renormalize global mean */
			    mean[i][j][k][l] /= dnom[i][j][k];
			}
			else
			    mean[i][j][k][l] = mean[0][j][0][l];
		    }
		    wt_var[i][j][k][l] = wt_var[0][j][0][l];
		}
		dnom[i][j][k] = dnom[0][j][0];
	    }
	}
    }
}

void
gauden_norm_wt_var(vector_t ***in_var,
		   vector_t ***wt_var,
		   int32 pass2var,
		   float32 ***dnom,
		   vector_t ***mean,
		   uint32 n_mgau,
		   uint32 n_feat,
		   uint32 n_density,
		   const uint32 *veclen,
		   int32 tiedvar)
{
    uint32 i, j, k, l;

    if (tiedvar) {
	gauden_tie_vars_dnoms(wt_var, pass2var, dnom, mean,
			      n_mgau, n_feat, n_density, veclen);
    }
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		if (dnom[i][j][k] != 0) {
		    for (l = 0; l < veclen[j]; l++) {
			if (!pass2var) {
			    wt_var[i][j][k][l] =
				(wt_var[i][j][k][l] / dnom[i][j][k]) -
				(mean[i][j][k][l] * mean[i][j][k][l]);
			}
			else {
			    wt_var[i][j][k][l] /= dnom[i][j][k];
			}

			if (wt_var[i][j][k][l] < 0) {
			    E_ERROR("Variance (mgau= %u, feat= %u, "
				    "density=%u, component=%u) is less then 0. "
				    "Most probably the number of senones is "
				    "too high for such a small training "
				    "database. Use smaller $CFG_N_TIED_STATES.\n",
				    i, j, k, l);
			}
		    }
		}
		else {
		    if (in_var) {
			E_INFO("Copying unseen var (%u, %u, %u) from in_var\n",
			       i, j, k);
			for (l = 0; l < veclen[j]; l++) {
			    wt_var[i][j][k][l] = in_var[i][j][k][l];
			}
		    }
		}
	    }
	}
    }
}

static void
gauden_tie_fullvars_dnoms(vector_t ****wt_fullvar,
			  int32 pass2var,
			  float32 ***dnom,
			  vector_t ***mean,
			  uint32 n_mgau,
			  uint32 n_feat,
			  uint32 n_density,
			  const uint32 *veclen)
{
    uint32 i, j, k, l, ll;

    /* Tie all variances and dnoms together. */
    /* First accumulate first codebook */
    for (j = 0; j < n_feat; j++) {
	if (!pass2var) {
	    /* Unnormalize first mean */
	    for (l = 0; l < veclen[j]; l++) {
		mean[0][j][0][l] *= dnom[0][j][0];
	    }
	}
	for (k = 1; k < n_density; k++) {
	    for (l = 0; l < veclen[j]; l++) {
		for (ll = 0; ll < veclen[j]; ll++) {
		    wt_fullvar[0][j][0][l][ll] += wt_fullvar[0][j][k][l][ll];
		}
		if (!pass2var) {
		    /* Accumulate unnormalized means */
		    mean[0][j][0][l] += mean[0][j][k][l] * dnom[0][j][k];
		}
	    }
	    dnom[0][j][0] += dnom[0][j][k];
	}
    }
    /* Now add other codebooks to it */
    for (i = 1; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    for (ll = 0; ll < veclen[j]; ll++) {
			wt_fullvar[0][j][0][l][ll] += wt_fullvar[i][j][k][l][ll];
		    }
		    if (!pass2var) {
			mean[0][j][0][l] += mean[i][j][k][l] * dnom[i][j][k];
		    }
		}
		dnom[0][j][0] += dnom[i][j][k];
	    }
	}
    }
    /* Now spread everything around */
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    if (!pass2var) {
			if (i == 0 && k == 0) /* Renormalize global mean */
			    mean[i][j][k][l] /= dnom[i][j][k];
			else
			    mean[i][j][k][l] = mean[0][j][0][l];
		    }
		    for (ll = 0; ll < veclen[j]; ll++) {
			wt_fullvar[i][j][k][l][ll] = wt_fullvar[0][j][0][l][ll];
		    }
		}
		dnom[i][j][k] = dnom[0][j][0];
	    }
	}
    }
}

void
gauden_norm_wt_fullvar(vector_t ****in_var,
		       vector_t ****wt_var,
		       int32 pass2var,
		       float32 ***dnom,
		       vector_t ***mean,
		       uint32 n_mgau,
		       uint32 n_feat,
		       uint32 n_density,
		       const uint32 *veclen,
		       int32 tiedvar)
{
    uint32 i, j, k, l, ll;

    if (tiedvar) {
	gauden_tie_fullvars_dnoms(wt_var, pass2var, dnom, mean,
				  n_mgau, n_feat, n_density, veclen);
    }
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    vector_t *outermean = NULL;
	    if (!pass2var)
		outermean = (vector_t *)ckd_calloc_2d(veclen[j], veclen[j], sizeof(float32));
		
	    for (k = 0; k < n_density; k++) {
		if (!pass2var)
		    outerproduct(outermean,
				 mean[i][j][k], mean[i][j][k],
				 veclen[j]);
		if (dnom[i][j][k] != 0) {
		    for (l = 0; l < veclen[j]; l++) {
			for (ll = 0; ll < veclen[j]; ll++) {
			    if (!pass2var) {
				wt_var[i][j][k][l][ll] =
				    (wt_var[i][j][k][l][ll] / dnom[i][j][k]) -
				    outermean[l][ll];
			    }
			    else {
				wt_var[i][j][k][l][ll] /= dnom[i][j][k];
			    }
			}
		    }
		}
		else {
		    if (in_var) {
			E_INFO("Copying unseen var (%u, %u, %u) from in_var\n",
			       i, j, k);
			for (l = 0; l < veclen[j]; l++) {
			    wt_var[i][j][k][l] = in_var[i][j][k][l];
			}
		    }
		}
	    }
	    if (!pass2var)
		ckd_free_2d((void **)outermean);
	}
    }
}

#define N_BIN	4000

int
gauden_var_thresh_percentile(vector_t ***var,
			     float32 **var_thresh,
			     uint32 n_mgau,
			     uint32 n_stream,
			     uint32 n_density,
			     const uint32 *veclen,
			     float32 percent)
{
    float32 f = percent / 100.0;
    uint32 ***histo;
    float32 **min;
    float32 **max;
    float32 **bin_wdth;
    uint32 i, j, k, l, b, t;
    float32 gt;

    histo = (uint32 ***)ckd_calloc(n_stream, sizeof(uint32 **));
    min = (float32 **)ckd_calloc(n_stream, sizeof(float32 *));
    max = (float32 **)ckd_calloc(n_stream, sizeof(float32 *));

    for (j = 0; j < n_stream; j++) {
	histo[j] = ckd_calloc(veclen[j], sizeof(uint32 *));
	min[j] = ckd_calloc(veclen[j], sizeof(float32));
	max[j] = ckd_calloc(veclen[j], sizeof(float32));
	for (l = 0; l < veclen[j]; l++) {
	    histo[j][l] = ckd_calloc(N_BIN, sizeof(uint32));
	    max[j][l] = MIN_NEG_FLOAT32;
	    min[j][l] = MAX_POS_FLOAT32;
	}
    }

    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_stream; j++) {
	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    if (var[i][j][k][l] > max[j][l]) {
			max[j][l] = var[i][j][k][l];
		    }
		    if ((var[i][j][k][l] > 0) && (var[i][j][k][l] < min[j][l])) {
			min[j][l] = var[i][j][k][l];
		    }
		}
	    }
	}
    }

    E_INFO("var_extrema:\n");
    for (j = 0; j < n_stream; j++) {
	E_INFOCONT("\tstream %u: ", j);
	for (l = 0; l < veclen[j]; l++) {
	    E_INFOCONT("\t\t[%.4e %.4e]", min[j][l], max[j][l]);
	}
        E_INFOCONT("\n");
    }

    for (j = 0; j < n_stream; j++) {
	for (l = 0; l < veclen[j]; l++) {
	    max[j][l] = (max[j][l] - min[j][l]) / (float32)N_BIN;
	}
    }

    bin_wdth = max;

    for (j = 0; j < n_stream; j++) {
	for (l = 0; l < veclen[j]; l++) {
	    for (i = 0; i < n_mgau; i++) {
		for (k = 0; k < n_density; k++) {
		    if (var[i][j][k][l] > 0) {
			b = (uint32)floor((var[i][j][k][l] - min[j][l]) / bin_wdth[j][l]);
			++histo[j][l][b];
		    }
		}
	    }
	}
    }

    /* grand total # of variances for each stream */
    gt = n_mgau * n_density;

    for (j = 0; j < n_stream; j++) {
	for (l = 0; l < veclen[j]; l++) {
	    t = 0;
	    for (b = 0; b < N_BIN; b++) {
		t += histo[j][l][b];
		if (((float32)t / gt) > f)
		    break;
	    }
	    var_thresh[j][l] = min[j][l] + (b+1) * bin_wdth[j][l];
	}
    }
    
    E_INFO("var_extrema:\n");
    for (j = 0; j < n_stream; j++) {
	E_INFOCONT("\tstream %u: ", j);
	for (l = 0; l < veclen[j]; l++) {
	    E_INFOCONT("\t\t%.4e", var_thresh[j][l]);
	}
        E_INFOCONT("\n");
    }

    return S3_SUCCESS;
}

int
gauden_smooth_var(vector_t ***var,
		  float32 **var_floor,
		  uint32 n_mgau,
		  uint32 n_stream,
		  uint32 n_density,
		  const uint32 *veclen)
{
    uint32 i, j, k, l;
    float32 flr;
    
    for (j = 0; j < n_stream; j++) {
	for (l = 0; l < veclen[j]; l++) {

	    flr = var_floor[j][l];

	    for (i = 0; i < n_mgau; i++) {
		for (k = 0; k < n_density; k++) {
		    if (var[i][j][k][l] < flr) var[i][j][k][l] = flr;
		}
	    }
	}
    }

    return S3_SUCCESS;
}

int
gauden_massage_var(vector_t ***var,
		  float32 fudge_factor,
		  uint32 n_mgau,
		  uint32 n_stream,
		  uint32 n_density,
		  const uint32 *veclen)
{
    uint32 i, j, k, l;
    
    for (j = 0; j < n_stream; j++) {
	for (l = 0; l < veclen[j]; l++) {
	    for (i = 0; i < n_mgau; i++) {
		for (k = 0; k < n_density; k++) {
		    var[i][j][k][l] *= fudge_factor;
		}
	    }
	}
    }

    return S3_SUCCESS;
}

uint32
solve_quadratic(float64 x, float64 y, float64 z, float64 *root1, float64 *root2)
{
    float64 temp;
    
    if (x == 0) {
	if (y != 0) {
	    (*root1) = (*root2) = -z/y;
	    return TRUE;
	}
	else
	    return FALSE;
    }
    
    temp = y * y - (4 * x * z);
    if (temp < 0) {
	if (y != 0 && fabs(temp/(y*y)) < 0.0001) {
	    (*root1) = (*root2) = (-y / (2*x));
	    return TRUE;
	}
	else
	    return FALSE;
    }
    
    temp = sqrt(temp);
    
    (*root1) = (temp - y) / (2*x);
    (*root2) = (-temp - y) / (2*x);
    
    return TRUE;
}

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

float32
cal_constD(vector_t in_mean,
	   vector_t wt_num_mean,
	   vector_t wt_den_mean,
	   vector_t in_var,
	   vector_t wt_num_var,
	   vector_t wt_den_var,
	   float32 num_dnom,
	   float32 den_dnom,
	   uint32 n_veclen,
	   float32 constE)
{
    uint32 l;
    float32 d = 0.0;
    float64 x, y, z;
    float64 root1, root2;
    float32 d_mmi = 0.0;
    
    for (l = 0; l < n_veclen; l++) {
	x = in_var[l];
	y = (in_var[l] + in_mean[l]*in_mean[l]) * (num_dnom - den_dnom) + (wt_num_var[l] - wt_den_var[l])
	    - 2*in_mean[l]*(wt_num_mean[l] - wt_den_mean[l]);
	z = (wt_num_var[l] - wt_den_var[l]) * (num_dnom - den_dnom) - (wt_num_mean[l] - wt_den_mean[l]) * (wt_num_mean[l] - wt_den_mean[l]);
	
	if (solve_quadratic(x, y, z, &root1, &root2))
	    d = MAX(root1, root2) * D_FACTOR;
	
	d = MAX(d, constE*den_dnom);
	
	d_mmi = MAX(d_mmi, d);
    }
    
    return d_mmi;
}

void
gauden_norm_wt_mmie_mean(vector_t ***in_mean,
			 vector_t ***wt_mean,
			 vector_t ***wt_num_mean,
			 vector_t ***wt_den_mean,
			 vector_t ***in_var,
			 vector_t ***wt_num_var,
			 vector_t ***wt_den_var,
			 float32 ***num_dnom,
			 float32 ***den_dnom,
			 uint32 n_mgau,
			 uint32 n_feat,
			 uint32 n_density,
			 const uint32 *veclen,
			 float32 constE)
{
    uint32 i, j, k, l;
    float32 d_mmi  = 0.0;
    
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		if (num_dnom[i][j][k] != 0 || den_dnom[i][j][k] != 0) {
		        
		    /* compute constant D, which controls the convergence speed and accuracy */
		    d_mmi = cal_constD(in_mean[i][j][k], wt_num_mean[i][j][k], wt_den_mean[i][j][k],
				       in_var[i][j][k], wt_num_var[i][j][k], wt_den_var[i][j][k], num_dnom[i][j][k], den_dnom[i][j][k], veclen[j], constE);
		    if (!finite(d_mmi)) {
			E_ERROR("Constant D:%f (gau:%d feat:%d density:%d) is infinite, maybe unsufficient data\n", d_mmi, i, j, k);
    			for (l = 0; l < veclen[j]; l++) {
			    wt_mean[i][j][k][l] = in_mean[i][j][k][l];
    			}
			continue;
		    }
		        
		    /* update mean parameters */
		    for (l = 0; l < veclen[j]; l++) {
			wt_mean[i][j][k][l] = (wt_num_mean[i][j][k][l] - wt_den_mean[i][j][k][l] + d_mmi * in_mean[i][j][k][l]) / (num_dnom[i][j][k] - den_dnom[i][j][k] + d_mmi);
			if (!finite(wt_mean[i][j][k][l])) {
			    E_ERROR("The new mean:%f (gau:%d feat:%d density:%d vec:%d) is infinite, maybe unsufficient data\n", wt_mean[i][j][k][l], i, j, k, l);
			    wt_mean[i][j][k][l] = in_mean[i][j][k][l];
			    continue;
			}
		    }
		}
	    }
	}
    }
}


void
gauden_norm_wt_mmie_var(vector_t ***in_var,
			vector_t ***wt_var,
			vector_t ***wt_num_var,
			vector_t ***wt_den_var,
			float32 ***num_dnom,
			float32 ***den_dnom,
			vector_t ***in_mean,
			vector_t ***wt_mean,
			vector_t ***wt_num_mean,
			vector_t ***wt_den_mean,
			uint32 n_mgau,
			uint32 n_feat,
			uint32 n_density,
			const uint32 *veclen,
			float32 constE)
{
    uint32 i, j, k, l;
    float32 d_mmi = 0.0;
    
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		if (num_dnom[i][j][k] != 0 || den_dnom[i][j][k] != 0) {
		        
		    /* compute constant D, which controls the convergence speed and accuracy */
		    d_mmi = cal_constD(in_mean[i][j][k], wt_num_mean[i][j][k], wt_den_mean[i][j][k],
				       in_var[i][j][k], wt_num_var[i][j][k], wt_den_var[i][j][k], num_dnom[i][j][k], den_dnom[i][j][k], veclen[j], constE);
		    if (!finite(d_mmi)) {
			E_ERROR("Constant D:%f (gau:%d feat:%d density:%d) is infinite, maybe unsufficient data\n", d_mmi, i, j, k);
    			for (l = 0; l < veclen[j]; l++) {
			    wt_var[i][j][k][l] = in_var[i][j][k][l];
    			}
			continue;
		    }
		        
		    /* update variance parameters */
		    for (l = 0; l < veclen[j]; l++) {
			wt_var[i][j][k][l] = ((wt_num_var[i][j][k][l] - wt_den_var[i][j][k][l]) + 
					      d_mmi * (in_var[i][j][k][l] + in_mean[i][j][k][l] * in_mean[i][j][k][l])) / 
			    (num_dnom[i][j][k] - den_dnom[i][j][k] + d_mmi)
			    - (wt_mean[i][j][k][l] * wt_mean[i][j][k][l]);
			if (!finite(wt_var[i][j][k][l])) {
			    E_ERROR("The new variance:%f (gau:%d feat:%d density:%d vec:%d) is infinite, maybe unsufficient data\n", wt_var[i][j][k][l], i, j, k, l);
			    wt_var[i][j][k][l] = in_var[i][j][k][l];
			    continue;
			}
			
			/* if the new var<0, keep it unchanged */
			if (wt_var[i][j][k][l] < 0) {
			    E_ERROR("Variance of the senone (mgau= %u, feat= %u, ""density=%u, component=%u) < 0\n", i, j, k, l);
			    wt_var[i][j][k][l] = in_var[i][j][k][l];
			}
		    }
		}
	    }
	}
    }
}
