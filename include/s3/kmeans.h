/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
 * Revision 1.1  2000/09/24  21:38:30  awb
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
