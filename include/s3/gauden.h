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
 * File: gauden.h
 * 
 * Description: 
 *	
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef GAUDEN_H
#define GAUDEN_H

#include <s3/s3.h>
#include <s3/vector.h>

#include <stdio.h>

typedef struct {
    uint32 n_feat;
    const uint32 *veclen;

    uint32 n_mgau;
    uint32 n_density;
    uint32 n_top;

    float32 ***norm;
    vector_t ***mean;
    vector_t ***var;

    vector_t ***macc;
    vector_t ***vacc;
    float32  ***dnom;

    vector_t ***l_macc;
    vector_t ***l_vacc;
    float32  ***l_dnom;

    uint32 *mllr_idx;		/* MLLR class for each mixture Gaussian */
    uint32 n_mllr_class;	/* Total # of MLLR classes */

    float32  ****l_regr_acc;
    float32  *****l_regl_acc;

    float32  ****regr_acc;
    float32  *****regl_acc;
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

vector_t ***
gauden_alloc_param(uint32 n_cb,
		   uint32 n_stream,
		   uint32 n_density,
		   const uint32 *veclen);
int
gauden_set_const_dim_mgau(gauden_t *g,
			  uint32 n_mgau,	/* # of mixture gaussians */
			  uint32 n_density);	/* # of densities per mixture gaussian */

uint32
gauden_n_mgau(gauden_t *g);

uint32
gauden_n_density(gauden_t *g);

uint32
gauden_n_top(gauden_t *g);

uint32
gauden_n_feat(gauden_t *g);

const uint32 *
gauden_veclen(gauden_t *g);

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
	       uint32 mgau);

int
gauden_compute_log(float64 **den,
		   uint32 **den_idx,
		   vector_t *obs,
		   gauden_t *g,
		   uint32 mgau);

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
		   int32 mllr_mult,
		   int32 mllr_add);

void
gauden_free_param(vector_t ***p);

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
		   const uint32 *veclen);

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
	  
int
gauden_massage_var(vector_t ***var,
                  float32 fudge_factor,
                  uint32 n_mgau,
                  uint32 n_stream,
                  uint32 n_density,
                  const uint32 *veclen);

#endif /* GAUDEN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:30  awb
 * *** empty log message ***
 *
 * Revision 1.6  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.5  1996/07/29  16:49:12  eht
 * made call to gauden_compute() consistent w/ gauden_mixture()
 *
 * Revision 1.4  1996/03/25  15:53:14  eht
 * Changes to deal w/ case where # of densities referenced
 * by a single utterance is << # total densities
 *
 * Revision 1.3  1996/01/26  18:28:23  eht
 * Added 'const' specifier for veclen argument that is not modified
 * w/in.
 *
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/09/08  15:21:06  eht
 * Initial revision
 *
 *
 */
