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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/gauden.h>

#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/feat.h>
#include <s3/err.h>

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

/* On the Alphas, we can have this nice fast routine */
#if __alpha
#define	EXPF	expf
#else
#define EXPF	exp
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
    g->var = NULL;

    /* free the density normalization terms */
    if (g->norm)
	ckd_free_3d((void ***)g->norm);
    g->norm = NULL;

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
    g->veclen = veclen;

    fflush(stdout);
    
    E_INFO("%d feature streams (", n_feat);
    for (i = 0; i < n_feat; i++) {
	printf("|%d|=%d ", i, veclen[i]);
    }
    printf(")\n");

    fflush(stdout);

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

uint32 gauden_n_mgau(gauden_t *g)
{
    return g->n_mgau;
}

uint32 gauden_n_density(gauden_t *g)
{
    return g->n_density;
}

uint32 gauden_n_top(gauden_t *g)
{
    return g->n_top;
}

uint32 gauden_n_feat(gauden_t *g)
{
    return g->n_feat;
}

vector_t ***
gauden_mean(gauden_t *g)
{
    return g->mean;
}

const uint32 *
gauden_veclen(gauden_t *g)
{
    return g->veclen;
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

float32
diag_norm(vector_t var,
	  uint32 len)
{
    float32 log_det;
    float32 p;
    uint32 i;

    log_det = 0;

    for (i = 0; i < len; i++)
	log_det += log(var[i]);

    p = len * log(2.0 * M_PI);

    return - 0.5 * (log_det + p);
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
		g->norm[i][j][k] = diag_norm(g->var[i][j][k], g->veclen[j]);
	    }
	}
    }
}

int
gauden_floor_variance(gauden_t *g)
{
    uint32 i, j, k;

    for (i = 0; i < g->n_mgau; i++) {
	for (j = 0; j < g->n_feat; j++) {
	    for (k = 0; k < g->n_density; k++) {
		vector_floor(g->var[i][j][k], g->veclen[j], min_var);
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

/*
 * Precompute term 1 / (2 * \sigma_i ^ 2) and normalization factor (determinant
 * of covariance matrix).
 */
int
gauden_eval_precomp(gauden_t *g)
{
    gauden_compute_norm(g);	/* compute normalization factor for Gaussians */
    gauden_double_variance(g);	/* pre-multiply variances by 2 for EXP dnom */
    gauden_invert_variance(g);	/* compute 1/(2 sigma^2) terms */
    
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

void
gauden_free_acc(gauden_t *g)
{
    uint32 f, m;

    if (g->macc) {
	gauden_free_param(g->macc);
    }
    g->macc = NULL;

    if (g->vacc) {
	gauden_free_param(g->vacc);
    }
    g->vacc = NULL;

    if (g->dnom) {
	ckd_free_3d((void ***)g->dnom);
    }
    g->dnom = NULL;

    if (g->regr_acc && g->regl_acc) {
	for (m = 0; m < g->n_mllr_class; m++) {
	    for (f = 0; f < g->n_feat; f++) {
		ckd_free_3d((void ***)g->regl_acc[m][f]);
		ckd_free_2d((void **)g->regr_acc[m][f]);
	    }
	}
	ckd_free_2d((void **)g->regl_acc);
	ckd_free_2d((void **)g->regr_acc);
    }
    g->regl_acc = NULL;
    g->regr_acc = NULL;
}

int32
gauden_alloc_acc(gauden_t *g)
{
    uint32 f, m;

    if (*(int32 *)cmd_ln_access("-meanreest") == TRUE) {
	g->macc = alloc_acc(g, g->n_mgau);
    }
    if (*(int32 *)cmd_ln_access("-varreest") == TRUE) {
	g->vacc = alloc_acc(g, g->n_mgau);
    }
    
    if (*(int32 *)cmd_ln_access("-meanreest") == TRUE) {
	g->dnom = (float32  ***) ckd_calloc_3d(g->n_mgau,
					       g->n_feat,
					       g->n_density,
					       sizeof(float32));
    }
    if ((*(int32 *)cmd_ln_access("-mllrmult") == TRUE) ||
        (*(int32 *)cmd_ln_access("-mllradd") == TRUE)) {
        g->regr_acc = (float32 ****) ckd_calloc_2d(g->n_mllr_class, g->n_feat, sizeof(float32 **));
	g->regl_acc = (float32 *****) ckd_calloc_2d(g->n_mllr_class, g->n_feat, sizeof(float32 ***));

	for (m = 0; m < g->n_mllr_class; m++) {
	    for (f = 0; f < g->n_feat; f++) {
		g->regr_acc[m][f] = (float32 **) ckd_calloc_2d(g->veclen[f],
							       g->veclen[f]+1,
							       sizeof(float32));
		g->regl_acc[m][f] = (float32 ***) ckd_calloc_3d(g->veclen[f],
								g->veclen[f]+1,
								g->veclen[f]+1,
								sizeof(float32));
	    }
	}
    }
    
    return S3_SUCCESS;
}

void
gauden_free_l_acc(gauden_t *g)
{
    uint32 f, m;

    if (g->l_macc) {
	gauden_free_param(g->l_macc);
    }
    g->l_macc = NULL;

    if (g->l_vacc) {
	gauden_free_param(g->l_vacc);
    }
    g->l_vacc = NULL;

    if (g->l_dnom) {
	ckd_free_3d((void ***)g->l_dnom);
    }
    g->l_dnom = NULL;

    if (g->l_regr_acc && g->l_regl_acc) {
	for (m = 0; m < g->n_mllr_class; m++) {
	    for (f = 0; f < g->n_feat; f++) {
		ckd_free_3d((void ***)g->l_regl_acc[m][f]);
		ckd_free_2d((void **)g->l_regr_acc[m][f]);
	    }
	}
	ckd_free_2d((void **)g->l_regl_acc);
	ckd_free_2d((void **)g->l_regr_acc);
    }
    g->l_regl_acc = NULL;
    g->l_regr_acc = NULL;
}

int32
gauden_alloc_l_acc(gauden_t *g, uint32 n_lcl,
		   int32 mean_reest,
		   int32 var_reest,
		   int32 mllr_mult,
		   int32 mllr_add)
{
    uint32 f, m;

    /* free any local accumulators from the prior utterance */
    gauden_free_l_acc(g);

    if (mean_reest) {
	/* allocate mean accumulators */
	assert(g->l_macc == NULL);
	g->l_macc = alloc_acc(g, n_lcl);
    }
    if (var_reest) {
	/* allocate variance accumulators */
	assert(g->l_vacc == NULL);
	g->l_vacc = alloc_acc(g, n_lcl);
    }
    if (mean_reest || var_reest) {
	/* allocate mean/var normalization accumulators */
	assert(g->l_dnom == NULL);
	g->l_dnom = (float32  ***) ckd_calloc_3d(n_lcl,
						 g->n_feat,
						 g->n_density,
						 sizeof(float32));
    }

    if (mllr_mult || mllr_add) {
	/* MLLR matrix accumulators */
	assert(g->l_regr_acc == NULL);
	g->l_regr_acc = (float32 ****) ckd_calloc_2d(g->n_mllr_class, g->n_feat, sizeof(float32 **));
	assert(g->l_regl_acc == NULL);
	g->l_regl_acc = (float32 *****) ckd_calloc_2d(g->n_mllr_class, g->n_feat, sizeof(float32 ***));

	for (m = 0; m < g->n_mllr_class; m++) {
	    for (f = 0; f < g->n_feat; f++) {
		g->l_regr_acc[m][f] = (float32 **) ckd_calloc_2d(g->veclen[f],
								 g->veclen[f]+1,
								 sizeof(float32));
		g->l_regl_acc[m][f] = (float32 ***) ckd_calloc_3d(g->veclen[f],
								  g->veclen[f]+1,
								  g->veclen[f]+1,
								  sizeof(float32));
	    }
	}
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

float32 ***gauden_l_dnom(gauden_t *g)
{
    return g->l_dnom;
}

void gauden_free_param(vector_t ***p)
{
    ckd_free(p[0][0][0]);
    ckd_free_3d((void ***)p);
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

float64
log_diag_eval(vector_t obs,
	      float32 norm,
	      vector_t mean,
	      vector_t var_fact,
	      uint32 veclen)
{
    float64 d, diff;
    uint32 l;
    
    d = norm;	/* log (1 / 2 pi |sigma^2|) */

    for (l = 0; l < veclen; l++) {
	diff = obs[l] - mean[l];
	d -= var_fact[l] * diff * diff;	/* compute -1 / (2 sigma ^2) * (x - m) ^ 2 terms */
    }

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

void
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

void
log_topn_densities(float64 *den,
		   uint32 *den_idx,
		   uint32 n_top,
		   uint32 n_density,
		   uint32 veclen,
		   vector_t obs,
		   vector_t *mean,
		   vector_t *var,
		   float32 *log_norm)
{
    uint32 i, j, k;
    vector_t m;
    vector_t v;
    float64 diff;
    float64 d;
    float64 worst;	/* worst density value of the top N density values
			   seen so far */

    /* a better initialization for this would be to use
       the top N indices from the prior frame and compute
       the density values for them for the given frame */
    for (i = 0; i < n_top; i++) {
	den[i] = MIN_IEEE_NORM_NEG_FLOAT64;
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

	if (d > worst) {
	    /*
	    * codeword i is more likely than the Nth of the top N
	    * observed so far.
	    */

	    /* find the insertion point for the new codeword
	       and log liklihood */
	    for (j = 0; j < n_top; j++)
		if (d > den[j])
		    break;

	    /* make room for the new entry at position j */
	    for (k = n_top-1; k > j; k--) {
		den[k] = den[k-1];
		den_idx[k] = den_idx[k-1];
	    }

	    assert(j == k);

	    /* insert the new values */
	    den[k] = d;
	    den_idx[k] = i;

	    worst = den[n_top-1];
	}
    }
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
gauden_compute(float64 **den,		/* density array for a mixture Gaussian */
	       uint32 **den_idx,	/* density index array for n_top < n_density eval */
	       vector_t *obs,		/* observation vector for some time */
	       gauden_t *g,		/* Gaussian density structure */
	       uint32 mgau)		/* id of the mixture Gau. to evaluate */
{
    uint32 j, k;

    /* make sure this is true at initialization time */
    assert(g->n_top <= g->n_density);

    if (g->n_top == g->n_density) {
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
		den[j][k] = EXPF( den[j][k] );
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
			       g->norm[mgau][j]);

	    for (k = 0; k < g->n_top; k++) {
		den[j][k] = EXPF( den[j][k] );
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
		   uint32 mgau)		/* id of the mixture Gau. to evaluate */
{
    uint32 j;

    /* make sure this is true at initialization time */
    assert(g->n_top <= g->n_density);

    if (g->n_top == g->n_density) {
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
			       g->norm[mgau][j]);
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
/* BHIKSHA: converted g->n_density to g->n_top; possible bugfix 6 Apr 98 */
	    for (k = 0; k < g->n_top; k++) {
/* BHIKSHA: converted g->n_density to g->n_top; possible bugfix ENDS */
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
/* BHIKSHA: converted g->n_density to g->n_top; possible bugfix 6 Apr 98 */
	    for (k = 0; k < g->n_top; k++) {
/* BHIKSHA: converted g->n_density to g->n_top; possible bugfix ENDS */
		den[c][j][k] = EXPF(den[c][j][k] - max_den[j]);
	    }
	}
    }

    return max_den;
}

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
/* BHIKSHA converged g->n_density to g->n_top; possible bugfix, APR 6  98 */
	    for (k = 0; k < g->n_top; k++) {
/* BHIKSHA converged g->n_density to g->n_top; possible bugfix, END */
		den[c][j][k] = EXPF(den[c][j][k] - scl[j]);
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
    float64 oprob[MAX_N_FEAT];
    float64 out;
    uint32 j, kk, k;
    uint32 n_top;
    uint32 n_feat;

    n_top = g->n_top;
    n_feat = g->n_feat;

    assert(n_feat < MAX_N_FEAT);

    for (j = 0; j < n_feat; j++) {
	k = den_idx[j][0];
	oprob[j] = w[j][k] * den[j][0];

	for (kk = 1; kk < n_top; kk++) {
	    k = den_idx[j][kk];
	    oprob[j] += w[j][k] * den[j][kk];
	}
    }

    /* combine all n_feat independent stream output prob */
    out = oprob[0];
    for (j = 1; j < n_feat; j++) {
	out *= oprob[j];
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

void
gauden_norm_wt_var(vector_t ***in_var,
		   vector_t ***wt_var,
		   int32 pass2var,
		   float32 ***dnom,
		   vector_t ***mean,
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
			if (!pass2var) {
			    wt_var[i][j][k][l] =
				(wt_var[i][j][k][l] / dnom[i][j][k]) -
				(mean[i][j][k][l] * mean[i][j][k][l]);
			}
			else {
			    wt_var[i][j][k][l] /= dnom[i][j][k];
			}

			if (wt_var[i][j][k][l] < 0) {
			    E_ERROR("var (mgau= %u, feat= %u, "
				    "density=%u, component=%u) < 0\n",
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

    for (j = 0; j < n_stream; j++) {
	E_INFO("var_extrema:\n");
	E_INFO("\tstream %u: ", j);
	for (l = 0; l < veclen[j]; l++) {
	    fprintf(stderr, "\t\t[%.4e %.4e]", min[j][l], max[j][l]);
	}
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
    
    for (j = 0; j < n_stream; j++) {
	E_INFO("var_extrema:\n");
	E_INFO("\tstream %u: ", j);
	for (l = 0; l < veclen[j]; l++) {
	    fprintf(stderr, "\t\t%.4e", var_thresh[j][l]);
	}
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

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.15  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.14  1996/07/29  16:46:51  eht
 * - made call to gauden_compute consistent w/ gauden_mixture
 * - made density values float64
 * - use ckd_calloc_{2d,3d}() etc. calls
 * - deal w/ unobserved means and variances by copying from another set of "good" ones
 *
 * Revision 1.13  1996/03/25  15:26:21  eht
 * Deal w/ case when there are many more densities total than
 * referenced in any given utterance.
 *
 * Revision 1.12  1996/02/02  17:30:42  eht
 * Added code to only evaluate the Gaussians in the current utterance.
 *
 * Revision 1.11  1996/01/26  17:51:34  eht
 * Define a allocation function for mean and variance parameter matrices
 * Fix the determinant calculation for diagonal Gaussians
 * Add a function for the case of evaluating full mixture Gaussians rather than
 * 	topn.
 * Add a Euclidean distance function
 *
 * Revision 1.10  1995/12/15  18:37:07  eht
 * Added some type cases for memory alloc/free
 *
 * Revision 1.9  1995/12/14  20:14:49  eht
 * Fixed some inconsistencies between mathematical definition
 * and code.  Shouldn't affect results, but now is consistent
 * w/ the definitions.
 *
 * Revision 1.8  1995/11/10  19:18:43  eht
 * Get rid of timer calls
 *
 * Revision 1.7  1995/10/18  11:23:58  eht
 * Changed MAXFLOAT to MAX_IEEE_NORM_POS_FLOAT32 because MAXFLOAT
 * does not seem to be defined always in math.h on all machines.
 *
 * Revision 1.6  1995/10/10  13:09:40  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.5  1995/10/09  15:11:53  eht
 * Changed interface to ckd_alloc to remove need for __FILE__, __LINE__ arguments
 *
 * Revision 1.4  1995/08/09  20:34:01  eht
 * Fix determinant calculation and add some normalization functions
 *
 * Revision 1.3  1995/07/07  12:03:08  eht
 * Made determinant calculation consistent with SPHINX-II.
 * Changed verbose output format and content.
 *
 * Revision 1.2  1995/06/02  16:49:44  eht
 * Fixed goof while including PWP's error stuff
 *
 * Revision 1.1  1995/06/02  16:47:48  eht
 * Initial revision
 *
 *
 */
