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
 * File: kmeans.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef KMEANS_H
#define KMEANS_H

#include <s3/prim_type.h>
#include <s3/vector.h>

typedef uint32 codew_t;

typedef struct {
    uint32 idx;
    float64 d;
} idx_dist_t;


void k_means_set_get_obs(vector_t (*fn)(uint32 i));

float64
k_means(vector_t *mean,			/* initial set of means */
	uint32 n_mean,			/* # of means (should be k_mean?) */

	uint32 n_obs,			/* # of observations */
	uint32 veclen,			/* vector length of means and corpus */
	float32 min_sqerr_ratio,
	uint32 max_iter,		/* If not converged by this count, just quit */
	codew_t **out_label);		/* The final labelling of the corpus according
					   to the adjusted means; if NULL passed, just
					   discarded. */
float64
k_means_label(codew_t *label,
	      vector_t *mean,
	      uint32 n_mean,	/* # of mean vectors */
	      uint32 n_obs,	/* in # of vectors */
	      uint32 vector_len);

float64
k_means_trineq(vector_t *mean,			/* initial set of means */
	       uint32 n_mean,			/* # of means (should be k_mean?) */

	       uint32 n_obs,			/* # of observations */
	       uint32 veclen,			/* vector length of means and corpus */
	       float32 min_sqerr_ratio,
	       uint32 max_iter,		/* If not converged by this count, just quit */
	       codew_t **out_label);		/* The final labelling of the corpus according
						   to the adjusted means; if NULL passed, just
						   discarded. */

float64
k_means_label_trineq(codew_t *label,
		     vector_t *mean,
		     uint32 n_mean,       /* # of mean vectors */
		     idx_dist_t **nnmap,
		     uint32 n_obs,   /* in # of vectors */
		     uint32 veclen);


#define K_MEANS_SUCCESS		 0
#define K_MEANS_EMPTY_CODEWORD	-1

int
k_means_update(vector_t *mean,
	       uint32 n_mean,
	       uint32 vector_len,
	       codew_t *label,
	       uint32 n_obs);


float64
k_means_subset(vector_t *mean,			/* initial set of means */
	       uint32 n_mean,			/* # of means (should be k_mean?) */

	       uint32 *subset,
	       uint32 n_obs_subset,			/* # of observations */
	       uint32 veclen,			/* vector length of means and corpus */
	       float32 min_sqerr_ratio,
	       uint32 max_iter,		/* If not converged by this count, just quit */
	       codew_t **out_label);		/* The final labelling of the corpus according
						   to the adjusted means; if NULL passed, just
						   discarded. */
float64
k_means_label_subset(codew_t *label,
		     vector_t *mean,
		     uint32 n_mean,	/* # of mean vectors */
		     uint32 *subset,
		     uint32 n_obs_subset,	/* in # of vectors */
		     uint32 vector_len);

int
k_means_update_subset(vector_t *mean,
		      uint32 n_mean,
		      uint32 vector_len,
		      uint32 *subset,
		      codew_t *label,
		      uint32 n_obs_subset);


#endif /* KMEANS_H */ 


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
 * Revision 1.2  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.1  96/01/26  18:33:02  eht
 * Initial revision
 * 
 *
 */
