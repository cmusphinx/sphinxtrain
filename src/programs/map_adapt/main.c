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

#include <s3/common.h>
#include <sys_compat/file.h>
#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3acc_io.h>
#include <s3/s3.h>
#include <s3/ts2cb.h>
#include <s3/s3ts2cb_io.h>
#include <sphinxbase/matrix.h>
#include <sphinxbase/err.h>

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

static float32 ***
estimate_tau(vector_t ***si_mean, vector_t ***si_var, float32 ***si_mixw,
	     uint32 n_cb, uint32 n_stream, uint32 n_density, uint32 n_mixw, const uint32 *veclen,
	     vector_t ***wt_mean, float32 ***wt_mixw, float32 ***wt_dcount)
{
    float32 ***map_tau;
    uint32 i, j, k, m;

    E_INFO("Estimating tau hyperparameter from variances and observations\n");
    map_tau = (float32 ***)ckd_calloc_3d(n_cb, n_stream, n_density, sizeof(float32));
    for (i = 0; i < n_cb; ++i) {
	for (j = 0; j < n_stream; ++j) {
	    for (k = 0; k < n_density; ++k) {
		float32 tau_nom, tau_dnom;

		tau_nom = veclen[j] * wt_mixw[i][j][k];
		tau_dnom = 0.0f;
		for (m = 0; m < veclen[j]; ++m) {
		    float32 ydiff, wvar, dnom, ml_mu, si_mu, si_sigma;

		    dnom = wt_dcount[i][j][k];
		    si_mu = si_mean[i][j][k][m];
		    si_sigma = si_var[i][j][k][m];
		    ml_mu = dnom ? wt_mean[i][j][k][m] / dnom : si_mu;
		    
		    ydiff = ml_mu - si_mu;
		    /* Gauvain/Lee's estimation of this makes no
		     * sense as I read it, it seems to simply
		     * equal the precision matrix.  We want to use
		     * the variance anyway, because higher
		     * variance in the SI models should lead to
		     * stronger adaptation. */
		    /* And this is still less than ideal, because
		     * it way overestimates tau for SCHMM due to
		     * the large number of mixtures.  So for
		     * semi-continuous models you probably want to
		     * use -fixedtau. */
		    wvar = si_mixw[i][j][k] * si_sigma;
		    tau_dnom += dnom * ydiff * wvar * ydiff;
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

    return map_tau;
}

static int
map_mixw_reest(model_def_t *mdef, float32 ***map_tau, float32 fixed_tau,
	       float32 ***si_mixw, float32 ***wt_mixw, float32 ***map_mixw,
	       float32 mwfloor, uint32 n_cb, uint32 n_mixw, uint32 n_stream, uint32 n_density)
{
    uint32 i, j, k, cb;

    E_INFO("Re-estimating mixture weights using MAP\n");
    for (i = 0; i < n_mixw; ++i) {

        if (n_mixw == n_cb) {
	    cb = i;
	} else if (n_cb == 1) {
	    cb = 0;
	} else {
	    if (mdef == NULL) {
		E_ERROR("Failed to adapt mixtures. Please specify -moddeffn and -ts2cbfn\n");
		return S3_ERROR;
	    }
	    cb = mdef->cb[i];
	}

	for (j = 0; j < n_stream; ++j) {
	    float32 sum_tau, sum_nu, sum_wt_mixw;
	    

	    sum_tau = sum_nu = sum_wt_mixw = 0.0f;
	    for (k = 0; k < n_density; ++k)
		sum_tau += (map_tau != NULL) ? map_tau[cb][j][k] : fixed_tau;
	    for (k = 0; k < n_density; ++k) {
		float32 nu;

		/* NOTE: We estimate nu such that the SI mixture
		 * weight is the mode of the posterior distribution,
		 * hence the + 1.  This allows the MAP estimate to
		 * converge to the SI one in the case of no adaptation
		 * data (clearly, this is desirable!) */
		nu = si_mixw[i][j][k] * sum_tau + 1;
		sum_nu += nu;
		sum_wt_mixw += wt_mixw[i][j][k];
	    }

	    for (k = 0; k < n_density; ++k) {
		float32 tau, nu;

		tau = (map_tau != NULL) ? map_tau[cb][j][k] : fixed_tau;
		nu = si_mixw[i][j][k] * sum_tau + 1;

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
    return S3_SUCCESS;
}

static int
map_tmat_reest(float32 ***si_tmat, float32 ***wt_tmat,
	       float32 ***map_tmat, float32 tpfloor,
	       uint32 n_tmat, uint32 n_state)
{
    uint32 t, i, j;

    E_INFO("Re-estimating transition probabilities using MAP\n");
    for (t = 0; t < n_tmat; ++t) {
	for (i = 0; i < n_state-1; ++i) {
	    float32 sum_si_tmat = 0.0f, sum_wt_tmat = 0.0f;

	    for (j = 0; j < n_state; ++j) {
		sum_si_tmat += si_tmat[t][i][j];
		sum_wt_tmat += si_tmat[t][i][j];
	    }
	    for (j = 0; j < n_state; ++j) {
		if (si_tmat[t][i][j] + wt_tmat[t][i][j] < 0) continue;

		map_tmat[t][i][j] =
		    (si_tmat[t][i][j] + wt_tmat[t][i][j])
		    / (sum_si_tmat + sum_wt_tmat);
		if (map_tmat[t][i][j] < 0.0f) {
		    E_WARN("map_tmat[%d][%d][%d] < 0 (%f)\n",
			   t, i, j, map_tmat[t][i][j]);
		    map_tmat[t][i][j] = 0.0f;
		}
	    }
	}
    }

    return S3_SUCCESS;
}

static int32
bayes_mean_reest(vector_t ***si_mean, vector_t ***si_var,
		 vector_t ***wt_mean, vector_t ***wt_var,
		 float32 ***wt_dcount, int32 pass2var,
		 vector_t ***map_mean, float32 varfloor,
		 uint32 i, uint32 j, uint32 k, const uint32 *veclen)
{
    uint32 m;

    /* Textbook MAP estimator for single Gaussian.
       This works better if tau is unknown. */
    for (m = 0; m < veclen[j]; ++m) {
	if (wt_dcount[i][j][k]) {
	    float32 mlmean, mlvar;

	    mlmean = wt_mean[i][j][k][m] / wt_dcount[i][j][k];
	    if (pass2var)
		mlvar = wt_var[i][j][k][m] / wt_dcount[i][j][k];
	    else
		mlvar = (wt_var[i][j][k][m] / wt_dcount[i][j][k]
			 - mlmean * mlmean);
	    /* Perfectly normal if -2passvar isn't specified. */
	    if (mlvar < 0.0f) {
		if (pass2var)
		    E_WARN("mlvar[%d][%d][%d][%d] < 0 (%f)\n", i,j,k,m,mlvar);
		mlvar = varfloor;
	    }
	    map_mean[i][j][k][m] =
		(wt_dcount[i][j][k] * si_var[i][j][k][m] * mlmean
		 + mlvar * si_mean[i][j][k][m])
		/ (wt_dcount[i][j][k] * si_var[i][j][k][m] + mlvar);
	}
	else
	    map_mean[i][j][k][m] = si_mean[i][j][k][m];
    }
    return S3_SUCCESS;
}

static int
map_mean_reest(float32 tau, vector_t ***si_mean, vector_t ***wt_mean,
	       float32 ***wt_dcount, vector_t ***map_mean,
	       uint32 i, uint32 j, uint32 k, const uint32 *veclen)
{
    uint32 m;

    /* CH Lee mean update equation.  Use this if
       you want to experiment with values of tau. */
    for (m = 0; m < veclen[j]; ++m) {
	if (wt_dcount[i][j][k])
	    map_mean[i][j][k][m] =
		(tau * si_mean[i][j][k][m] + wt_mean[i][j][k][m])
		/ (tau + wt_dcount[i][j][k]);
	else
	    map_mean[i][j][k][m] = si_mean[i][j][k][m];
    }
    return S3_SUCCESS;
}

static int
map_var_reest(float32 tau, vector_t ***si_mean, vector_t ***si_var,
	      vector_t ***wt_mean, vector_t ***wt_var, float32 ***wt_dcount,
	      vector_t ***map_mean, vector_t ***map_var, float32 varfloor,
	      uint32 i, uint32 j, uint32 k, const uint32 *veclen)
{
    uint32 m;

    for (m = 0; m < veclen[j]; ++m) {
	float32 alpha, beta, mdiff;

	/* Somewhat different estimates of alpha and beta from the
	 * ones given in Gauvain & Lee.  These actually converge to
	 * the SI variance with no observations, and also seem to
	 * perform better in at least one case.  */
	alpha = tau + 1;
	beta = tau * si_var[i][j][k][m];

	mdiff = si_mean[i][j][k][m] - map_mean[i][j][k][m];
	/* This should be the correct update equation for diagonal
	 * covariance matrices. */
	map_var[i][j][k][m] = (beta
			       + wt_var[i][j][k][m]
			       + tau * mdiff * mdiff)
	    / (alpha - 1 + wt_dcount[i][j][k]);
	if (map_var[i][j][k][m] < 0.0f) {
	    /* This is bad and shouldn't happen! */
	    E_WARN("mapvar[%d][%d][%d][%d] < 0 (%f)\n", i,j,k,m, map_var[i][j][k][m]);
	    map_var[i][j][k][m] = varfloor;
	}
	if (map_var[i][j][k][m] < varfloor)
	    map_var[i][j][k][m] = varfloor;
    }
    return S3_SUCCESS;
}

static int
map_update(void)
{
    float32 ***si_mixw = NULL;
    float32 ***si_tmat = NULL;
    vector_t ***si_mean = NULL;
    vector_t ***si_var = NULL;

    vector_t ***wt_mean = NULL;
    vector_t ***wt_var = NULL;
    float32 ***wt_mixw = NULL;
    float32 ***wt_tmat = NULL;
    float32 ***wt_dcount = NULL;
    int32 pass2var;

    float32 ***map_mixw = NULL;
    float32 ***map_tmat = NULL;
    vector_t ***map_mean = NULL;
    vector_t ***map_var = NULL;
    float32 ***map_tau = NULL;
    float32 fixed_tau = 10.0f;
    float32 mwfloor = 1e-5f;
    float32 varfloor = 1e-5f;
    float32 tpfloor = 1e-4f;

    uint32 n_mixw, n_mixw_rd;
    uint32 n_tmat, n_tmat_rd, n_state, n_state_rd;
    uint32 n_cb, n_cb_rd;
    uint32 n_stream, n_stream_rd;
    uint32 n_density, n_density_rd;
    uint32 *veclen = NULL;
    uint32 *veclen_rd = NULL;

    const char **accum_dir;
    const char *si_mixw_fn;
    const char *map_mixw_fn;
    const char *si_tmat_fn;
    const char *map_tmat_fn;
    const char *si_mean_fn;
    const char *map_mean_fn;
    const char *si_var_fn;
    const char *map_var_fn;

    uint32 i, j, k;

    accum_dir = cmd_ln_str_list("-accumdir");
    si_mean_fn = cmd_ln_str("-meanfn");
    si_var_fn = cmd_ln_str("-varfn");
    si_tmat_fn = cmd_ln_str("-tmatfn");
    si_mixw_fn = cmd_ln_str("-mixwfn");
    map_mean_fn = cmd_ln_str("-mapmeanfn");
    map_var_fn = cmd_ln_str("-mapvarfn");
    map_tmat_fn = cmd_ln_str("-maptmatfn");
    map_mixw_fn = cmd_ln_str("-mapmixwfn");

    /* Must be at least one accum dir. */
    if (accum_dir == NULL)
	E_FATAL("Must specify at least one -accumdir\n");

    /* Must have means and variances. */
    if (si_mean_fn == NULL || si_var_fn == NULL || si_mixw_fn == NULL)
	E_FATAL("Must specify baseline means, variances, and mixture weights\n");

    /* Must specify output means. */
    if (map_mean_fn == NULL)
	E_FATAL("Must at least specify output MAP means\n");

    /* Read SI model parameters. */
    if (s3gau_read(si_mean_fn, &si_mean,
		   &n_cb, &n_stream, &n_density, &veclen) != S3_SUCCESS)
	E_FATAL("Couldn't read %s\n", si_mean_fn);
    if (s3gau_read(si_var_fn, &si_var,
		   &n_cb_rd, &n_stream_rd, &n_density_rd, &veclen_rd) != S3_SUCCESS)
	E_FATAL("Couldn't read %s\n", si_var_fn);
    check_consistency(si_var_fn, n_cb, n_cb_rd, n_stream, n_stream_rd,
		      n_density, n_density_rd, veclen, veclen_rd);
    /* Don't free veclen_rd, as rdacc_den needs it. */

    /* Read and normalize SI mixture weights. */
    if (si_mixw_fn) {
	mwfloor = cmd_ln_float32("-mwfloor");
	if (s3mixw_read(si_mixw_fn, &si_mixw, &n_mixw, &n_stream_rd, &n_density_rd)
	    != S3_SUCCESS)
	    E_FATAL("Couldn't read %s\n", si_mixw_fn);
	for (i = 0; i < n_mixw; ++i) {
	    for (j = 0; j < n_stream; ++j) {
		float32 sum_si_mixw = 0.0f;
		for (k = 0; k < n_density; ++k) {
		    if (si_mixw[i][j][k] < mwfloor)
			si_mixw[i][j][k] = mwfloor;
		    sum_si_mixw += si_mixw[i][j][k];
		}
		for (k = 0; k < n_density; ++k)
		    si_mixw[i][j][k] /= sum_si_mixw;
	    }
	}
    }

    /* Read SI transition matrices. */
    /* FIXME: We may want to normalize these if we do more interesting
     * estimation of the eta hyperparameters (i.e. using tau) */
    if (si_tmat_fn) {
	tpfloor = cmd_ln_float32("-tpfloor");
	if (s3tmat_read(si_tmat_fn, &si_tmat, &n_tmat, &n_state)
	    != S3_SUCCESS)
	    E_FATAL("Couldn't read %s\n", si_tmat_fn);
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
	if (pass2var && map_var_fn)
	    E_FATAL("Variance re-estimation requested, but -2passvar was specified in bw.");
	if (map_mixw_fn || !cmd_ln_int32("-fixedtau")) {
	    if (rdacc_mixw(accum_dir[i],
			   &wt_mixw,
			   &n_mixw_rd, &n_stream_rd, &n_density_rd) != S3_SUCCESS)
		E_FATAL("Error in reading mixture weights from %s\n", accum_dir[i]);
	    check_consistency(accum_dir[i],
			      n_mixw, n_mixw_rd, n_stream, n_stream_rd,
			      n_density, n_density_rd, veclen, veclen_rd);
	}
	if (map_tmat_fn) {
	    if (rdacc_tmat(accum_dir[i],
			   &wt_tmat,
			   &n_tmat_rd, &n_state_rd) != S3_SUCCESS)
		E_FATAL("Error in reading transition matrices from %s\n", accum_dir[i]);
	    if (n_tmat_rd != n_tmat || n_state_rd != n_state)
		E_FATAL("Mimsatch in tranition matrices from %s\n", accum_dir[i]);
	}
    }
    ckd_free(veclen_rd);

    /* Allocate MAP parameters */
    map_mean  = gauden_alloc_param(n_cb, n_stream, n_density, veclen);
    if (map_var_fn)
	map_var = gauden_alloc_param(n_cb, n_stream, n_density, veclen);
    if (map_mixw_fn)
	map_mixw = (float32 ***)ckd_calloc_3d(n_mixw, n_stream, n_density, sizeof(float32));
    if (map_tmat_fn)
	map_tmat = (float32 ***)ckd_calloc_3d(n_tmat, n_state-1, n_state, sizeof(float32));

    /* Optionally estimate prior tau hyperparameter for each HMM
     * (all other prior parameters can be derived from it). */
    if (cmd_ln_int32("-fixedtau")) {
	fixed_tau = cmd_ln_float32("-tau");
	E_INFO("tau hyperparameter fixed at %f\n", fixed_tau);
    }
    else
	map_tau = estimate_tau(si_mean, si_var, si_mixw,
			       n_cb, n_stream, n_density, n_mixw, veclen,
			       wt_mean, wt_mixw, wt_dcount);

    /* Re-estimate mixture weights. */
    if (map_mixw) {
	model_def_t *mdef = NULL;
	
	if (cmd_ln_str("-moddeffn")) {
	    const char *ts2cbfn;

	    E_INFO("Reading %s\n", cmd_ln_str("-moddeffn"));
	    if (model_def_read(&mdef,
	    	   cmd_ln_str("-moddeffn")) != S3_SUCCESS) {
		return S3_ERROR;
	    }
    	    ts2cbfn = cmd_ln_str("-ts2cbfn");
	    if (strcmp(SEMI_LABEL, ts2cbfn) == 0) {
		mdef->cb = semi_ts2cb(mdef->n_tied_state);
	    }
	    else if (strcmp(CONT_LABEL, ts2cbfn) == 0) {
		mdef->cb = cont_ts2cb(mdef->n_tied_state);
	    }
	    else if (strcmp(PTM_LABEL, ts2cbfn) == 0) {
	        mdef->cb = ptm_ts2cb(mdef);
	    }
	    else if (s3ts2cb_read(ts2cbfn,
			      &mdef->cb,
			      NULL,
			      NULL) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	}

	map_mixw_reest(mdef, map_tau, fixed_tau,
		       si_mixw, wt_mixw, map_mixw, mwfloor,
		       n_cb, n_mixw, n_stream, n_density);
		      
	if (mdef)
	    model_def_free(mdef);
    }

    /* Re-estimate transition matrices. */
    if (map_tmat)
	map_tmat_reest(si_tmat, wt_tmat, map_tmat, tpfloor,
		       n_tmat, n_state);

    /* Re-estimate means and variances */
    if (cmd_ln_int32("-bayesmean"))
	E_INFO("Re-estimating means using Bayesian interpolation\n");
    else
	E_INFO("Re-estimating means using MAP\n");
    if (n_mixw != n_cb && n_cb == 1)
	E_INFO("Interpolating tau hyperparameter for semi-continuous models\n");
    if (n_mixw != n_cb && n_cb != 1)
	E_INFO("Interpolating tau hyperparameter for PTM models\n");
    if (map_var)
	E_INFO("Re-estimating variances using MAP\n");

    for (i = 0; i < n_cb; ++i) {
	for (j = 0; j < n_stream; ++j) {
	    for (k = 0; k < n_density; ++k) {
		float32 tau;

		if (map_tau == NULL)
		    tau = fixed_tau;
		else {
		    tau = map_tau[i][j][k];
		}

		/* Means re-estimation. */
		if (cmd_ln_int32("-bayesmean"))
		    bayes_mean_reest(si_mean, si_var,
				     wt_mean, wt_var,
				     wt_dcount, pass2var,
				     map_mean, varfloor,
				     i, j, k, veclen);
		else
		    map_mean_reest(tau, si_mean, wt_mean, wt_dcount,
				   map_mean, i, j, k, veclen);


		/* Variance re-estimation.  Doesn't work with
		 * -2passvar, and in many cases this can actually
		 * degrade accuracy, so use it with caution. */
		if (map_var)
		    map_var_reest(tau, si_mean, si_var, wt_mean, wt_var,
				  wt_dcount, map_mean, map_var, varfloor,
				  i, j, k, veclen);
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
	    E_FATAL("Unable to write MAP mean to %s\n",map_mean_fn);

    if (map_var && map_var_fn)
	if (s3gau_write(map_var_fn,
			(const vector_t ***)map_var,
			n_cb,
			n_stream,
			n_density,
			veclen) != S3_SUCCESS)
	    E_FATAL("Unable to write MAP variance to %s\n",map_var_fn);

    if (map_mixw && map_mixw_fn)
	if (s3mixw_write(map_mixw_fn,
			 map_mixw,
			 n_mixw,
			 n_stream,
			 n_density)!= S3_SUCCESS)
	    E_FATAL("Unable to write MAP mixture weights to %s\n",map_mixw_fn);

    if (map_tmat && map_tmat_fn)
	if (s3tmat_write(map_tmat_fn,
			 map_tmat,
			 n_tmat,
			 n_state)!= S3_SUCCESS)
	    E_FATAL("Unable to write MAP transition matrices to %s\n",map_tmat_fn);

    ckd_free(veclen);
    gauden_free_param(si_mean);
    gauden_free_param(si_var);
    if (si_mixw)
	ckd_free_3d(si_mixw);
    if (si_tmat)
	ckd_free_3d(si_tmat);
    gauden_free_param(wt_mean);
    gauden_free_param(wt_var);
    ckd_free_3d(wt_dcount);
    if (map_mean)
	gauden_free_param(map_mean);
    if (map_var)
	gauden_free_param(map_var);
    if (map_tau)
	ckd_free_3d(map_tau);
    if (map_mixw)
	ckd_free_3d(map_mixw);
    if (map_tmat)
	ckd_free_3d(map_tmat);
    
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
