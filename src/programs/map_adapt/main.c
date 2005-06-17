/* -*- c-file-style: "bsd"; c-basic-offset: 4 -*- */
/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1996-2005 Carnegie Mellon University.
 * All rights reserved.
 *********************************************************************
 *
 * File: src/programs/map_adapt/main.c
 * 
 * Description: 
 *	Do one pass of MAP re-estimation (adaptation).
 *
 *      See "Speaker Adaptation Based on MAP Estimation of HMM
 *      Parameters", Chin-Hui Lee and Jean-Luc Gauvain, Proceedings
 *      of ICASSP 1993, p. II-558 for the details of prior density
 *      estimation and forward-backward MAP.
 * 
 * Author: 
 *	David Huggins-Daines <dhuggins@cs.cmu.edu>
 *
 *********************************************************************/

/* The SPHINX-III common library */
#include <stdio.h>
#include <s3/common.h>

#include <sys_compat/file.h>
#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3acc_io.h>
#include <s3/matrix.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s3.h>
#include <s3/err.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "parse_cmd_ln.h"

static void
check_consistency(const char *filename,
		  uint32 n_mgau, uint32 n_mgau_rd,
		  uint32 n_stream, uint32 n_stream_rd,
		  uint32 n_density, uint32 n_density_rd,
		  const uint32 *veclen, 
		  const uint32 *veclen_rd)
{
    uint32 s;

    if (n_mgau != n_mgau_rd)
	E_FATAL("Number of codebooks is mismatched in %s\n",filename);
    if (n_stream != n_stream_rd)
	E_FATAL("Number of streams is mismatched in %s\n",filename);
    if (n_density != n_density_rd)
	E_FATAL("Number of gaussians is mismatched in %s\n",filename);
    for (s = 0; s < n_stream; ++s)
	if (veclen[s] != veclen_rd[s])
	    E_FATAL("Vector length of stream %u mismatched in %s\n",
		    s, filename);
}

static int
map_update(void)
{
    float32 ***si_mixw = NULL;
    vector_t ***si_mean = NULL;
    vector_t ***si_var = NULL;

    vector_t ***wt_mean = NULL;
    vector_t ***wt_var = NULL;
    float32 ***wt_mixw = NULL;
    float32 ***wt_dcount = NULL;
    int32 pass2var;

    float32 ***map_mixw = NULL;
    vector_t ***map_mean = NULL;
    vector_t ***map_var = NULL;
    float32 ***map_tau = NULL;
    float32 fixed_tau = 2.0f;
    float32 mwfloor = 1e-5f;

    uint32 n_mixw, n_mixw_rd;
    uint32 n_cb, n_cb_rd;
    uint32 n_stream, n_stream_rd;
    uint32 n_density, n_density_rd;
    const uint32 *veclen = NULL;
    const uint32 *veclen_rd = NULL;

    const char **accum_dir;
    const char *si_mixw_fn;
    const char *map_mixw_fn;
/*      const char *si_tmat_fn; */
/*      const char *map_tmat_fn; */
    const char *si_mean_fn;
    const char *map_mean_fn;
    const char *si_var_fn;
    const char *map_var_fn;

    uint32 i, j, k, m;

    accum_dir = (const char **)cmd_ln_access("-accumdir");
    si_mean_fn = (const char *)cmd_ln_access("-meanfn");
    si_var_fn = (const char *)cmd_ln_access("-varfn");
/*      si_tmat_fn = (const char *)cmd_ln_access("-tmatfn"); */
    si_mixw_fn = (const char *)cmd_ln_access("-mixwfn");
    map_mean_fn = (const char *)cmd_ln_access("-mapmeanfn");
    map_var_fn = (const char *)cmd_ln_access("-mapvarfn");
/*      map_tmat_fn = (const char *)cmd_ln_access("-maptmatfn"); */
    map_mixw_fn = (const char *)cmd_ln_access("-mapmixwfn");

    /* Must be at least one accum dir. */
    if (accum_dir == NULL)
	E_FATAL("Must specify at least one -accumdir\n");

    /* Must have means and variances. */
    if (si_mean_fn == NULL || si_var_fn == NULL || si_mixw_fn == NULL)
	E_FATAL("Must specify baseline means, variances, and mixture weights\n");

    /* Read SI model parameters. */
    if (s3gau_read(si_mean_fn, &si_mean,
		   &n_cb, &n_stream, &n_density, &veclen) != S3_SUCCESS)
	E_FATAL("Couldn't read %s\n", si_mean_fn);
    if (s3gau_read(si_var_fn, &si_var,
		   &n_cb_rd, &n_stream_rd, &n_density_rd, &veclen_rd) != S3_SUCCESS)
	E_FATAL("Couldn't read %s\n", si_var_fn);
    check_consistency(si_var_fn, n_cb, n_cb_rd, n_stream, n_stream_rd,
		      n_density, n_density_rd, veclen, veclen_rd);
    ckd_free((void *)veclen_rd);

    /* Read and normalize SI mixture weights. */
    mwfloor = cmd_ln_float32("-mwfloor");
    if (s3mixw_read(si_mixw_fn, &si_mixw, &n_mixw, &n_stream_rd, &n_density_rd)
	!= S3_SUCCESS)
	E_FATAL("Couldn't read %s\n", si_mixw_fn);
    for (i = 0; i < n_mixw; ++i) {
	for (j = 0; j < n_stream; ++j) {
	    float32 sum_si_mixw = 0.0f;
	    for (k = 0; k < n_density; ++k)
		sum_si_mixw += si_mixw[i][j][k];
	    for (k = 0; k < n_density; ++k)
		si_mixw[i][j][k] /= sum_si_mixw;
	}
    }

    /* Read observation counts. */
    for (i = 0; accum_dir[i]; ++i) {
	E_INFO("Reading and accumulating observation counts from %s\n",
	       accum_dir[i]);
        if (rdacc_den(accum_dir[i],
                      &wt_mean,
                      &wt_var,          
                      &pass2var,        
                      &wt_dcount,
                      &n_cb_rd,
                      &n_stream_rd,
                      &n_density_rd,
                      &veclen_rd) != S3_SUCCESS)
            E_FATAL("Error in reading densities from %s\n", accum_dir[i]);
	check_consistency(accum_dir[i],
			  n_cb, n_cb_rd, n_stream, n_stream_rd,
			  n_density, n_density_rd, veclen, veclen_rd);
	if (rdacc_mixw(accum_dir[i],
		       &wt_mixw,
		       &n_mixw_rd, &n_stream_rd, &n_density_rd) != S3_SUCCESS)
            E_FATAL("Error in reading mixture weights from %s\n", accum_dir[i]);
	check_consistency(accum_dir[i],
			  n_mixw, n_mixw_rd, n_stream, n_stream_rd,
			  n_density, n_density_rd, veclen, veclen_rd);
	ckd_free((void *)veclen_rd);
    }

    /* Allocate MAP parameters */
    map_mean  = gauden_alloc_param(n_cb, n_stream, n_density, veclen);
    map_var = gauden_alloc_param(n_cb, n_stream, n_density, veclen);
    map_mixw = (float32 ***)ckd_calloc_3d(n_mixw, n_stream, n_density, sizeof(float32));

    /* Optionally estimate prior tau hyperparameter for each HMM
     * (all other prior parameters can be derived from it). */
    if (cmd_ln_int32("-fixedtau"))
	fixed_tau = cmd_ln_float32("-tau");
    else {
	map_tau = (float32 ***)ckd_calloc_3d(n_mixw, n_stream, n_density, sizeof(float32));
	for (i = 0; i < n_mixw; ++i) {
	    for (j = 0; j < n_stream; ++j) {
		for (k = 0; k < n_density; ++k) {
		    float32 tau_nom, tau_dnom;

		    tau_nom = veclen[j] * wt_mixw[i][j][k];
		    tau_dnom = 0.0f;
		    for (m = 0; m < veclen[j]; ++m) {
			float32 ydiff, wprec, dnom, ml_mu, si_mu, si_sigma;

			if (n_mixw != n_cb && n_cb == 1) {/* Semi-continuous. */
			    dnom = wt_dcount[0][j][k];
			    si_mu = si_mean[0][j][k][m];
			    si_sigma = si_var[0][j][k][m];
			    ml_mu = dnom ? wt_mean[0][j][k][m] / dnom : si_mu;
			}
			else { /* Continuous. */
			    dnom = wt_dcount[i][j][k];
			    si_mu = si_mean[i][j][k][m];
			    si_sigma = si_var[i][j][k][m];
			    ml_mu = dnom ? wt_mean[i][j][k][m] / dnom : si_mu;
			}

			ydiff = ml_mu - si_mu;
			wprec = si_mixw[i][j][k] * si_sigma;
			tau_dnom += dnom * ydiff * wprec * ydiff;
		    }
		    if (tau_dnom > 1e-5 && tau_nom > 1e-5)
			map_tau[i][j][k] = tau_nom / tau_dnom;
		    else
			map_tau[i][j][k] = 1000.0f; /* FIXME: Something big, I guess. */
#if 0
      		    E_INFO("map_tau[%d][%d][%d] = %f / %f = %f\n",
      			   i, j, k, tau_nom, tau_dnom, map_tau[i][j][k]);
#endif
		}
	    }
	}
    }

    /* Re-estimate mixture weights separately for SCHMMs */
    for (i = 0; i < n_mixw; ++i) {
	for (j = 0; j < n_stream; ++j) {
	    float32 sum_tau, sum_nu, sum_wt_mixw;

	    sum_tau = sum_nu = sum_wt_mixw = 0.0f;
	    for (k = 0; k < n_density; ++k)
		sum_tau += (map_tau != NULL) ? map_tau[i][j][k] : fixed_tau;
	    for (k = 0; k < n_density; ++k) {
		float32 nu;

		nu = si_mixw[i][j][k] * sum_tau;
		sum_nu += nu;
		sum_wt_mixw += wt_mixw[i][j][k];
	    }

	    for (k = 0; k < n_density; ++k) {
		float32 tau, nu;

		tau = (map_tau != NULL) ? map_tau[i][j][k] : fixed_tau;
		nu = si_mixw[i][j][k] * sum_tau;

		map_mixw[i][j][k] = (nu - 1 + wt_mixw[i][j][k])
		    / (sum_nu - n_density + sum_wt_mixw);
		/* Floor mixture weights - otherwise they will be
                   negative in cases where si_mixw is very small.
                   FIXME: This might be an error in my implementation?  */
		if (map_mixw[i][j][k] < mwfloor)
		    map_mixw[i][j][k] = mwfloor;
#if 0
		printf("%d %d %d tau %f map_mixw %f =\n"
		       "      nu %f - 1     +     wt_mixw %f\n"
		       "/ sum_nu %f  - %d   + sum_wt_mixw %f\n",
		       i, j, k, tau, 
		       map_mixw[i][j][k], nu, wt_mixw[i][j][k],
		       sum_nu, n_density, sum_wt_mixw);
#endif
	    }
	}
    }

    /* Re-estimate means, variances, transition matrices (possibly) */
    for (i = 0; i < n_cb; ++i) {
	for (j = 0; j < n_stream; ++j) {
	    for (k = 0; k < n_density; ++k) {
		float32 tau, alpha;

		if (map_tau == NULL)
		    tau = fixed_tau;
		else {
		    if (n_mixw != n_cb && n_cb == 1) {/* Semi-continuous. */
			int m;

			for (m = 0; m < n_mixw; ++m)
			    tau += map_tau[m][j][k];
			tau /= n_mixw;
#if 0
			printf("SC tau[%d][%d] = %f\n", j, k, tau);
#endif
		    }
		    else /* Continuous. */
			tau = map_tau[i][j][k];
		}
		for (m = 0; m < veclen[j]; ++m) {
		    float32 beta, mdiff, wt_vdiff;

		    /* Means re-estimation. */
		    if (cmd_ln_int32("-bayesmean")) {
			/* Textbook MAP estimator for single Gaussian.
			   This works better. */
			if (wt_dcount[i][j][k]) {
			    float32 mlmean, mlvar;

			    mlmean = wt_mean[i][j][k][m] / wt_dcount[i][j][k];
			    if (pass2var)
				mlvar = wt_var[i][j][k][m] / wt_dcount[i][j][k];
			    else
				mlvar = (wt_var[i][j][k][m] / wt_dcount[i][j][k]
					 - mlmean * mlmean);
			    if (mlvar < 0.0f) {/* Shouldn't happen, though. */
				E_INFO("mlvar[%d][%d][%d][%d] < 0 (%f)\n", i,j,k,m,mlvar);
				mlvar = 0.0f;
			    }
			    map_mean[i][j][k][m] =
				(wt_dcount[i][j][k] * si_var[i][j][k][m] * mlmean
				 + mlvar * si_mean[i][j][k][m])
				/ (wt_dcount[i][j][k] * si_var[i][j][k][m] + mlvar);
			}
			else
			    map_mean[i][j][k][m] = si_mean[i][j][k][m];
		    }
		    else {
			/* CH Lee mean update equation.  Use this if
                           you want to experiment with values of tau. */
			if (wt_dcount[i][j][k])
			    map_mean[i][j][k][m] =
				(tau * si_mean[i][j][k][m] + wt_mean[i][j][k][m])
				/ (tau + wt_dcount[i][j][k]);
			else
			    map_mean[i][j][k][m] = si_mean[i][j][k][m];
		    }

		    /* Variance re-estimation. */
		    alpha = (tau + 1) / 2;
		    if (alpha < veclen[j] - 1)
			alpha = veclen[j] + tau;
		    beta = (tau / 2) * si_var[i][j][k][m];
		    mdiff = map_mean[i][j][k][m] - si_mean[i][j][k][m];
		    /* FIXME: These calculations are still quite
                       wrong.  I fear that we need two passes to
                       estimate MAP variances. */
		    if (pass2var) /* sort of wrong */
			wt_vdiff = wt_var[i][j][k][m]
			    - map_mean[i][j][k][m] * map_mean[i][j][k][m] * wt_dcount[i][j][k];
		    else /* really wrong */
			wt_vdiff = wt_var[i][j][k][m] / wt_dcount[i][j][k];
		    map_var[i][j][k][m] = (2 * beta
					   + tau * mdiff * mdiff
					   + wt_vdiff)
			/ (2 * alpha - veclen[j] + wt_dcount[i][j][k]);
		    if (map_var[i][j][k][m] < 0.0f) {
/*  			E_INFO("mapvar[%d][%d][%d][%d] < 0 (%f)\n", i,j,k,m, map_var[i][j][k][m]); */
			map_var[i][j][k][m] = 0.0f;
		    }
		}
	    }
	}
    }

    if (map_mean_fn)
	if (s3gau_write(map_mean_fn,
			(const vector_t ***)map_mean,
			n_cb,
			n_stream,
			n_density,
			veclen) != S3_SUCCESS)
	    E_FATAL("Unable to write prior mean to %s\n",map_mean_fn);

    if (map_var_fn)
	if (s3gau_write(map_var_fn,
			(const vector_t ***)map_var,
			n_cb,
			n_stream,
			n_density,
			veclen) != S3_SUCCESS)
	    E_FATAL("Unable to write prior mean to %s\n",map_var_fn);

    if (map_mixw_fn)
	if (s3mixw_write(map_mixw_fn,
			 map_mixw,
			 n_mixw,
			 n_stream,
			 n_density)!= S3_SUCCESS)
	    E_FATAL("Unable to write prior mean to %s\n",map_mixw_fn);

    ckd_free((void *)veclen);
    gauden_free_param(si_mean);
    gauden_free_param(si_var);
    gauden_free_param(wt_mean);
    gauden_free_param(wt_var);
    ckd_free_3d((void *)wt_dcount);
    ckd_free_3d((void *)si_mixw);
    gauden_free_param(map_mean);
    gauden_free_param(map_var);
    if (map_tau)
	ckd_free_3d((void *)map_tau);
    ckd_free_3d((void *)map_mixw);
    
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    if (map_update() != S3_SUCCESS) {
	exit(1);
    }

    exit(0);
}
