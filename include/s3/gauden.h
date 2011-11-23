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
 * File: gauden.h
 * 
 * Description: 
 *	
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef GAUDEN_H
#define GAUDEN_H

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#define D_FACTOR 2.0
#define MIN_VAR 1.0E-20

#include <s3/s3.h>
#include <s3/vector.h>

#include <stdio.h>

typedef struct gauden_s {
    uint32 n_feat;
    uint32 *veclen;

    uint32 n_mgau;
    uint32 n_density;
    uint32 n_top;

    float32 ***norm;
    vector_t ***mean;
    vector_t ***var;
    vector_t ****fullvar;

    vector_t ***macc;
    vector_t ***vacc;
    vector_t ****fullvacc;
    float32  ***dnom;

    vector_t ***l_macc;
    vector_t ***l_vacc;
    vector_t ****l_fullvacc;
    float32  ***l_dnom;
} gauden_t;

#define MAX_LOG_DEN	10.0


int
gauden_set_min_var(float32 min);

gauden_t *
gauden_alloc(void);

int
gauden_set_feat(gauden_t *g,
		uint32 n_feat,
		const uint32 *veclen);

int
gauden_set_n_top(gauden_t *g,
		 uint32 n_top);	/* # of densities to compute for output prob */
int
gauden_set_n_mgau(gauden_t *g,
		  uint32 n);

int
gauden_set_n_density(gauden_t *g,
		     uint32 n);

vector_t ***
gauden_mean(gauden_t *g);

int
gauden_set_mean(gauden_t *g,
		vector_t ***mean);

vector_t ***
gauden_var(gauden_t *g);

int
gauden_set_var(gauden_t *g,
	       vector_t ***variance);

vector_t ****
gauden_fullvar(gauden_t *g);

int
gauden_set_fullvar(gauden_t *g,
		   vector_t ****variance);

vector_t ***
gauden_alloc_param(uint32 n_cb,
		   uint32 n_stream,
		   uint32 n_density,
		   const uint32 *veclen);

vector_t ****
gauden_alloc_param_full(uint32 n_cb,
			uint32 n_feat,
			uint32 n_density,
			const uint32 *veclen);

int
gauden_set_const_dim_mgau(gauden_t *g,
			  uint32 n_mgau,	/* # of mixture gaussians */
			  uint32 n_density);	/* # of densities per mixture gaussian */

#define gauden_n_mgau(g) (g)->n_mgau
#define gauden_n_top(g) (g)->n_top
#define gauden_n_feat(g) (g)->n_feat
#define gauden_n_density(g) (g)->n_density
#define gauden_veclen(g) (g)->veclen

int
gauden_s2_set_mean(gauden_t *g,
		   uint32 mgau,
		   float32 **mean);
int
gauden_s2_set_var(gauden_t *g,
		  uint32 mgau,
		  float32 **var);

int
gauden_floor_variance(gauden_t *g);

int
gauden_compute(float64 **den,
	       uint32 **den_idx,
	       vector_t *obs,
	       gauden_t *g,
	       uint32 mgau,
	       uint32 **prev_den_idx);   /* Previous frame's top N densities (or NULL) */

int
gauden_compute_log(float64 **den,
		   uint32 **den_idx,
		   vector_t *obs,
		   gauden_t *g,
		   uint32 mgau,
		   uint32 **prev_den_idx);   /* Previous frame's top N densities (or NULL) */

float64 *
gauden_scale_densities_fwd(float64 ***den,
			   uint32 ***den_idx,
			   uint32 *cb,
			   uint32 n_cb,
			   gauden_t *g);

int
gauden_scale_densities_bwd(float64 ***den,
			   uint32 ***den_idx,
			   float64 **scale,
			   uint32 *cb,
			   uint32 n_cb,
			   gauden_t *g);

int
gauden_compute_euclidean(float64 ***den,
			 uint32 ***den_idx,
			 vector_t *obs,
			 gauden_t *g);

int
gauden_normalize(float64 ***den,
		 gauden_t *g);	/* normalize (n_mgau x n_feat x n_top) density values */

void
gauden_print(FILE *fp,
	     float64 ***den,
	     uint32 ***den_idx,
	     gauden_t *g,
	     uint32 *inv,
	     uint32 n_inv);

float64
gauden_mixture(float64 **den,
	       uint32 **den_idx,
	       float32 **w,
	       gauden_t *g);

void
gauden_free(gauden_t *g);

void
gauden_free_acc(gauden_t *g);

int32
gauden_alloc_acc(gauden_t *g);

void
gauden_free_l_acc(gauden_t *g);

int32
gauden_alloc_l_acc(gauden_t *g, uint32 n_lcl,
		   int32 mean_reest,
		   int32 var_reest,
		   int32 fullvar);

void
gauden_free_param(vector_t ***p);

void
gauden_free_param_full(vector_t ****p);

vector_t ***gauden_l_macc(gauden_t *g);

vector_t ***gauden_l_vacc(gauden_t *g);

float32 ***gauden_l_dnom(gauden_t *g);

void
gauden_accum_param(vector_t ***out,
		   vector_t ***in,
		   uint32 n_mgau,
		   uint32 n_feat,
		   uint32 n_density,
		   const uint32 *veclen);

void
gauden_accum_param_full(vector_t ****out,
			vector_t ****in,
			uint32 n_mgau,
			uint32 n_feat,
			uint32 n_density,
			const uint32 *veclen);

void
gauden_norm_wt_mean(vector_t ***in_mean,
		    vector_t ***wt_mean,
		    float32 ***dnom,
		    uint32 n_mgau,
		    uint32 n_feat,
		    uint32 n_density,
		    const uint32 *veclen);

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
		   int32 tiedvar);

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
		       int32 tiedvar);
int
gauden_eval_precomp(gauden_t *g);

int
gauden_var_thresh_percentile(vector_t ***var,
			     float32 **var_thresh,
			     uint32 n_mgau,
			     uint32 n_stream,
			     uint32 n_density,
			     const uint32 *veclen,
			     float32 percent);

int
gauden_smooth_var(vector_t ***var,
		  float32 **var_thresh,
		  uint32 n_mgau,
		  uint32 n_stream,
		  uint32 n_density,
		  const uint32 *veclen);

/* primitives */
float32
diag_norm(vector_t var,
	  uint32 len);

void
diag_eval_precomp(vector_t var,
		  uint32 len);

float64
log_diag_eval(vector_t obs,
	      float32  norm,
	      vector_t mean,
	      vector_t var_fact, /* 1 / (2 * sigma ^ 2) */
	      uint32 veclen);	

float64
log_full_eval(vector_t obs,
	      float32  norm,
	      vector_t mean,
	      vector_t *var_inv, /* var^-1 */
	      uint32 veclen);	
	  
int
gauden_massage_var(vector_t ***var,
                  float32 fudge_factor,
                  uint32 n_mgau,
                  uint32 n_stream,
                  uint32 n_density,
                  const uint32 *veclen);

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
			 float32 constE);

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
			float32 constE);

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
	   float32 constE);

uint32
solve_quadratic(float64 x, float64 y, float64 z, float64 *root1, float64 *root2);

#ifdef __cplusplus
}
#endif

#endif /* GAUDEN_H */ 
