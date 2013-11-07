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
 * File: accum.c
 * 
 * Description: 
 *
 *    The routines contained in this file accumulate the reestimation
 * numerators and denominators of Baum-Welch over multiple observation
 * sequences (i.e. utterances).  The overall story is to:
 *
 *	1. Accumulate the reestimation sums for a an utterance alone.
 *
 * 	2. Accumulate the utterance reestimation sums in global sums
 *	   assuming the utterance looks ok (i.e. no alpha underflow
 *	   and no (alpha != beta) error occurs).
 *
 * Steps 1 and 2 are repeated until there are no more utterances in the
 * (sub)corpus.  Then, the global reestimation sums are written out to
 * files.
 *
 * An overview of the theory behind this routine can be found in
 * Chapter 6 of Rabiner & Juang, "Fundamentals of Speech Recognition",
 * Prentice-Hall 1993 with particular attention to p. 343 & p. 369.
 *
 * Extensions to the theory for parameter tying have been made.
 *
 * Scaling of alpha and beta by a consistent set of scaling
 * coefficients is assumed (p. 365).
 *
 * The functions exported to the outside by this file are:
 *
 *	accum_non_emit_trans -
 *		Accumulate the probability of making a given
 *		(in backward.c) non-emitting transition.
 *
 *	accum_trans -
 *		Accumulate the probability of making a given
 *		(in backward.c) emitting transition.
 *
 *	accum_gauden -
 *		Accumulate observations of for the top N
 *		codewords weighted by the initial model estimates
 *		for the probability of observing the codeword
 *		at a time t.  This is called for each frame in
 *		backward.c to compute the utterance totals.
 *
 *	accum_global -
 *		Add the utterance totals to the (sub)corpus totals.
 *
 *	accum_dump -
 *		Write the current (sub)corpus totals to files.
 *
 * There are no global variables required/exported by this file.
 *
 * Author: 
 * 	Eric H. Thayer, Carnegie Mellon University, 28-Aug-1995
 *********************************************************************/

#include "accum.h"

#include <s3/gauden.h>
#include <s3/s3gau_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/corpus.h>
#include <s3/model_inventory.h>
#include <s3/model_def.h>
#include <s3/s3.h>
#include <s3/state_seq.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/matrix.h>
#include <sphinxbase/feat.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

void
accum_den_terms(float32 **acc,
		float64 **den_terms,
		uint32 **den_idx,
		uint32 n_feat,
		uint32 n_top)
{
    uint32 j, k, kk;

    for (j = 0; j < n_feat; j++) {
	for (kk = 0; kk < n_top; kk++) {
	    k = den_idx[j][kk];

	    acc[j][k] += den_terms[j][kk];
	}
    }
}

/*********************************************************************
 *
 * Function: accum_gauden
 * 
 * Description: 
 *	Updates the mean and variance accumulators with the results
 *	of evaluating the current frame (i.e. all the calls to
 *	accum_emit_trans made for the current time).
 *
 * Function Inputs: 
 *	float32 ***denacc -
 *		contains probability of observing the top N codewords
 *		for the given time.
 *
 *	vector_t *frame -
 *		All feature vectors for the current time.
 *
 *	uint32 ***den_idx -
 *		Top N density indices for all feature streams for time t.
 *
 *	gauden_t *g -
 * 		Pointer to a structure containing the initial mean/var
 *		estimates for the iteration and mean/var reestimation
 *		numerators and denominators.
 *
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	S3_SUCCESS - if accumulation was successful.
 *
 * 	Currently, this is the only return value, but may
 *	want error conditions in the future.
 * 
 * Global Outputs: 
 *	None
 * 
 * Errors: 
 * 	None
 *
 *********************************************************************/

int
accum_gauden(float32 ***denacc,
	     uint32 *lcl2gbl,
	     uint32 n_lcl2gbl,
	     vector_t *frame,
	     uint32 ***den_idx,
	     gauden_t *g,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 pass2var,
	     float32 ***wacc,
	     int32 var_is_full,
	     FILE *pdumpfh,
	     feat_t* fcb)
{
    uint32 g_i, i, j, k, kk, l;

    vector_t ***macc = g->l_macc;	/* local to utt */
    vector_t m;
    vector_t ***mean = g->mean;
    vector_t pm = NULL;

    vector_t ***vacc = g->l_vacc;
    vector_t v = NULL;

    vector_t ****fullvacc = g->l_fullvacc;
    vector_t *fv = NULL;
    vector_t *cov = NULL;
    vector_t dvec = NULL;

    float32 ***dnom = g->l_dnom;

    float32 diff;
    float32 obs_cnt;
    
    int n_top = gauden_n_top(g);
    int n_density = gauden_n_density(g);

    /* for each density family found in the utterance */
    for (i = 0; i < n_lcl2gbl; i++) {

	g_i = lcl2gbl[i];

	/* for each feature */
	for (j = 0; j < gauden_n_feat(g); j++) {
	    vector_t feat = frame[j];
	    uint32* den_idx_row = den_idx[i][j];
	    float32* denacc_row = denacc[i][j];

	    if (var_is_full) {
		ckd_free_2d((void **)cov);
		cov = (vector_t *)ckd_calloc_2d(g->veclen[j], g->veclen[j], sizeof(float32));
		ckd_free(dvec);
		dvec = ckd_calloc(g->veclen[j], sizeof(float32));
	    }
	    

	    /* for each density in the mixture density */
	    for (kk = 0; kk < n_top; kk++) {
		
		if (n_top == n_density)
		    k = kk;
		else
		    k = den_idx_row[kk];	/* i.e. density k is one of the n_top densities */
		
		obs_cnt = denacc_row[k];	/* observation count for density (k) at this time frame
						   given the model */
		
		/* don't bother adding a bunch of
		   essentially zero values */
		if (obs_cnt <= MIN_POS_FLOAT32)
		    continue;

#ifdef ACCUM_VERBOSE
		printf("denacc[%u][%u][%3u] == %.3e\n", i, j, k, obs_cnt);
#endif
		if (pdumpfh)
			fprintf(pdumpfh, "%u %u %u %g\n", lcl2gbl[i], j, k, obs_cnt);

		m = macc[i][j][k];	/* the vector accumulator for mean (i,j,k) */

		if (pass2var) {
		    g_i = lcl2gbl[i];
		    pm = mean[g_i][j][k];	/* the mean (i, j, k) */
		}

		if (var_reest) {
		    if (var_is_full)
			fv = fullvacc[i][j][k];
		    else
			v = vacc[i][j][k];	/* the vector accumulator for variance (i,j,k) */
		}

		if (var_reest && var_is_full) {
		    if (!pass2var)
			outerproduct(cov, feat, feat, g->veclen[j]);
		    else {
			for (l = 0; l < g->veclen[j]; ++l)
			    dvec[l] = feat[l] - pm[l];
			outerproduct(cov, dvec, dvec, g->veclen[j]);
		    }
		    scalarmultiply(cov, obs_cnt, g->veclen[j]);
		    matrixadd(fv, cov, g->veclen[j]);
		}

    	        if (mean_reest) {
		    for (l = 0; l < g->veclen[j]; l++) {
			m[l] += obs_cnt * feat[l];
		    }
		}
		
		if (var_reest && !var_is_full) {
		    if (!pass2var) {
			for (l = 0; l < g->veclen[j]; l++) {
			    /* Always reest vars on untransformed features for now */
			    /* This does NOT work with -2passvar no (for pretty obvious reasons) */
			    v[l] += obs_cnt * feat[l] * feat[l];
			}
		    } else {
			for (l = 0; l < g->veclen[j]; l++) {
			    diff = feat[l] - pm[l];    
			    diff *= diff;
			    v[l] += obs_cnt * diff;
			}
		    }
		}

		/* accumulate observation count for all densities */
		dnom[i][j][k] += obs_cnt;
	    }
	    if (var_is_full) {
		ckd_free_2d((void **)cov);
		cov = NULL;
		ckd_free(dvec);
		dvec = NULL;
	    }
	}
    }

    if (pdumpfh)
	    fputs("\n", pdumpfh);

    return S3_SUCCESS;
}

void
accum_global_gauden(vector_t ***acc,
		    vector_t ***l_acc,
		    gauden_t *g,
		    uint32 *lcl2glb,
		    uint32 n_lcl2glb)
{
    uint32 n_feat;
    uint32 n_density;
    uint32 ii, i, j, k, l;

    if (acc == NULL) {
	/* nothing to do */

	return;
    }

    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);

    /* for each mixture density */
    for (ii = 0; ii < n_lcl2glb; ii++) {
	/* map local density id to global one */
	i = lcl2glb[ii];

	/* for each feature */
	for (j = 0; j < n_feat; j++) {

	    /* for each density in the mixture density */
	    for (k = 0; k < n_density; k++) {
		
		for (l = 0; l < g->veclen[j]; l++) {
		    acc[i][j][k][l] += l_acc[ii][j][k][l];
		}
	    }
	}
    }
}

void
accum_global_gauden_full(vector_t ****acc,
			 vector_t ****l_acc,
			 gauden_t *g,
			 uint32 *lcl2glb,
			 uint32 n_lcl2glb)
{
    uint32 n_feat;
    uint32 n_density;
    uint32 ii, i, j, k, l, ll;

    if (acc == NULL) {
	/* nothing to do */

	return;
    }

    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);

    /* for each mixture density */
    for (ii = 0; ii < n_lcl2glb; ii++) {
	/* map local density id to global one */
	i = lcl2glb[ii];

	/* for each feature */
	for (j = 0; j < n_feat; j++) {

	    /* for each density in the mixture density */
	    for (k = 0; k < n_density; k++) {
		
		for (l = 0; l < g->veclen[j]; l++) {
		    for (ll = 0; ll < g->veclen[j]; ll++) {
			acc[i][j][k][l][ll] += l_acc[ii][j][k][l][ll];
		    }
		}
	    }
	}
    }
}

void
accum_global_gauden_dnom(float32 ***dnom,
			 float32 ***l_dnom,
			 gauden_t *g,
			 uint32 *lcl2glb,
			 uint32 n_lcl2glb)
{
    uint32 n_feat;
    uint32 n_density;
    uint32 ii, i, j, k;

    if (dnom == NULL) {
	/* nothing to do */
	E_WARN("accum_global_gauden_dnom() called with NULL dnom\n");
	return;
    }

    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);

    for (ii = 0; ii < n_lcl2glb; ii++) {
	i = lcl2glb[ii];	/* find out the global id */
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		/* accumulate the local posterior into the global one */
		dnom[i][j][k] += l_dnom[ii][j][k];
	    }
	}
    }
}

void
accum_global_mixw(model_inventory_t *inv, gauden_t *g)
{
    uint32 *global_mixw;	/* a mapping from local mixing weight id to global one */
    uint32 n_local;		/* # of local mixing weights */
    uint32 n_feat;		/* # of independent feature streams */
    uint32 n_density;		/* # of densities per mixture density */
    float32 ***mixw_acc;	/* global mixing weight accumulators */
    float32 ***l_mixw_acc;	/* local mixing weight accumulators */
    uint32 i, ii, j, k;		/* iterators and temporary variables */

    global_mixw = inv->mixw_inverse;
    n_local = inv->n_mixw_inverse;
    n_feat = gauden_n_feat(g);
    n_density = gauden_n_density(g);

    mixw_acc = inv->mixw_acc;
    l_mixw_acc = inv->l_mixw_acc;

    for (ii = 0; ii < n_local; ii++) {
	i = global_mixw[ii];
	    
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		mixw_acc[i][j][k] += l_mixw_acc[ii][j][k];
	    }
	}
    }
}

void
accum_global_tmat(model_inventory_t *inv,
		  state_t *state,
		  uint32 n_state)
{
    float32 ***tmat_acc;	/* global tmat accumulators */
    float32 **l_tmat_acc;	/* local tmat accumulators (indexed by sentence HMM state id) */
    uint32 tmat;		/* a transition matrix id */
    uint32 model_i;		/* a model source state */
    uint32 model_j;		/* a model destination state */
    uint32 next_tmat;		/* the transition matrix associated w/ the destination state */
    uint32 i, u, j;		/* iterators and temporary variables */

    tmat_acc = inv->tmat_acc;
    l_tmat_acc = inv->l_tmat_acc;

    for (i = 0; i < n_state; i++) {
	tmat = state[i].tmat;
	model_i = state[i].m_state;

	for (u = 0; u < state[i].n_next; u++) {
	    j = state[i].next_state[u];

	    next_tmat = state[j].tmat;
	    model_j = state[j].m_state;
	    if ((next_tmat == tmat) && (model_i <= model_j)) {
	        /* internal phone model transition */
#ifdef ACCUM_VERBOSE
		printf("(%s %d %d) += (%d %d)\n",
		       acmod_set_id2name(inv->acmod_set, tmat), model_i, model_j,
		       i, j);
#endif
		
		tmat_acc[tmat][model_i][model_j] +=
		    l_tmat_acc[i][j-i];
	    }
	}
    }
}

/*********************************************************************
 *
 * Function: 
 *	accum_global
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	S3_SUCCESS - if accumulation was successful.
 *
 * 	Currently, this is the only return value, but may
 *	want error conditions in the future.
 * 
 * Global Outputs: 
 *	None
 * 
 * Errors: 
 * 	None
 *********************************************************************/
int32
accum_global(model_inventory_t *inv,
	     state_t *state,
	     uint32 n_state,
	     int32 mixw_reest,
	     int32 tmat_reest,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 var_is_full)
{
    gauden_t *g;

    g = inv->gauden;

    if (mixw_reest) {
	/* add local mixing weight accumulators to global ones */
	accum_global_mixw(inv, g);
    }

    if (tmat_reest) {
	/* add local transition probability matrix accumulators to global ones */
	accum_global_tmat(inv, state, n_state);
    }

    if (mean_reest) {
	/* add local mean accumulators to global ones */
	accum_global_gauden(g->macc, g->l_macc, g,
			    inv->cb_inverse, inv->n_cb_inverse);
    }
    if (var_reest) {
	/* add local variance accumulators to global ones */
	if (var_is_full)
	    accum_global_gauden_full(g->fullvacc, g->l_fullvacc, g,
				     inv->cb_inverse, inv->n_cb_inverse);
	else
	    accum_global_gauden(g->vacc, g->l_vacc, g,
				inv->cb_inverse, inv->n_cb_inverse);
    }
    if (mean_reest || var_reest) {
	/* add local mean/variance denominator accumulators to global ones */
	accum_global_gauden_dnom(g->dnom, g->l_dnom, g,
				 inv->cb_inverse, inv->n_cb_inverse);
    }
    
    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: 
 *	accum_stat
 * 
 * Description: 
 *	Print out some information about the accumulated counts.
 *	Currently, this routine does not do anything except determine
 *	when a tied state never occurs in the (sub)corpus.
 * 
 * Function Inputs: 
 *	model_inventory_t *inv -
 *		A pointer to the model inventory structure.  Its purpose
 *		is to allow the model definitions, parameters,
 *		reestimation numerators and denominators to be accessed via
 *		one pointer.
 *
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 * 	None
 *
 * Global Outputs: 
 * 	None
 *
 * Errors: 
 * 	When one or tied states do not appear in the (sub)corpus,
 *		a warning is printed.
 *
 *********************************************************************/
void
accum_stat(model_inventory_t *inv,
	   int32 mixw_reest)
{
    uint32 i, j, k;
    uint32 n_z;

    if (!mixw_reest)
	return;

    for (i = 0, n_z = 0; i < inv->n_mixw; i++) {
	for (j = 0; j < inv->gauden->n_feat; j++) {
	    for (k = 0; k < inv->gauden->n_density; k++) {
		if (inv->mixw_acc[i][j][k] != 0) {
		    break;
		}
	    }
	    if (k == inv->gauden->n_density)
	      n_z++;
	}
    }
    if (n_z > 500) {
      E_WARN("Over 500 senones never occur in the input data. "
             "This is normal for context-dependent untied senone training or for adaptation, "
             "but could indicate a serious problem otherwise.\n");
      return;
    }

    for (i = 0, n_z = 0; i < inv->n_mixw; i++) {
	for (j = 0; j < inv->gauden->n_feat; j++) {
	    for (k = 0; k < inv->gauden->n_density; k++) {
		if (inv->mixw_acc[i][j][k] != 0) {
		    break;
		}
	    }

	    if (k == inv->gauden->n_density) {
		if (n_z == 0) {
		    E_WARN("The following senones never occur in the input data\n\t");
		}

		fprintf(stderr, "%u ", i);
		++n_z;
		if (!(n_z % 10)) {
		    fprintf(stderr, "\n\t");
		}

		break;
	    }
	}
    }

    fprintf(stderr, "\n");
    fflush(stderr);
}

#include <sys_compat/file.h>

#include <time.h>


static int32
mk_bkp(int32 mixw_reest,
       int32 tmat_reest,
       int32 mean_reest,
       int32 var_reest,
       const char *out_dir)
{
    char fn[MAXPATHLEN+1];
    char fn_bkp[MAXPATHLEN+1];
    FILE *fp;

    if (mixw_reest) {
	sprintf(fn, "%s/mixw_counts", out_dir);
	sprintf(fn_bkp, "%s/mixw_counts.bkp", out_dir);

	fp = fopen(fn, "rb");
	if (fp != NULL) {
	    fclose(fp);
	    if (rename(fn, fn_bkp) < 0) {
		E_ERROR_SYSTEM("Couldn't backup %s\n", fn);
		return S3_ERROR;
	    }
	}
    }
    if (tmat_reest) {
	sprintf(fn, "%s/tmat_counts", out_dir);
	sprintf(fn_bkp, "%s/tmat_counts.bkp", out_dir);

	fp = fopen(fn, "rb");
	if (fp != NULL) {
	    fclose(fp);
	    if (rename(fn, fn_bkp) < 0) {
		E_ERROR_SYSTEM("Couldn't backup %s\n", fn);
		return S3_ERROR;
	    }
	}
    }
    if (mean_reest || var_reest) {
	sprintf(fn, "%s/gauden_counts", out_dir);
	sprintf(fn_bkp, "%s/gauden_counts.bkp", out_dir);

	fp = fopen(fn, "rb");
	if (fp != NULL) {
	    fclose(fp);
	    if (rename(fn, fn_bkp) < 0) {
		E_ERROR_SYSTEM("Couldn't backup %s\n", fn);
		return S3_ERROR;
	    }
	}
    }

    return S3_SUCCESS;
}
static int32
revert_bkp(int32 mixw_reest,
	   int32 tmat_reest,
	   int32 mean_reest,
	   int32 var_reest,
	   const char *out_dir)
{
    char fn[MAXPATHLEN+1];
    char fn_bkp[MAXPATHLEN+1];

    if (mixw_reest) {
	sprintf(fn, "%s/mixw_counts", out_dir);
	sprintf(fn_bkp, "%s/mixw_counts.bkp", out_dir);

	if (rename(fn_bkp, fn) < 0) {
	    E_ERROR_SYSTEM("Couldn't revert to backup of %s\n", fn);
	    
	    return S3_ERROR;
	}
    }
    if (tmat_reest) {
	sprintf(fn, "%s/tmat_counts", out_dir);
	sprintf(fn_bkp, "%s/tmat_counts.bkp", out_dir);

	if (rename(fn_bkp, fn) < 0) {
	    E_ERROR_SYSTEM("Couldn't revert to backup of %s\n", fn);
	    return S3_ERROR;
	}
    }
    if (mean_reest || var_reest) {
	sprintf(fn, "%s/gauden_counts", out_dir);
	sprintf(fn_bkp, "%s/gauden_counts.bkp", out_dir);

	if (rename(fn_bkp, fn) < 0) {
	    E_ERROR_SYSTEM("Couldn't revert to backup of %s\n", fn);
	    return S3_ERROR;
	}
    }

    return S3_SUCCESS;
}

static int32
commit(int32 mixw_reest,
       int32 tmat_reest,
       int32 mean_reest,
       int32 var_reest,
       int32 ckpt,
       const char *out_dir)
{
    char fn[MAXPATHLEN+1];
    char fn_bkp[MAXPATHLEN+1];

    /* no problem (other than a bit of wasted space) if
       the unlink()'s below fail */
    if (mixw_reest) {
	sprintf(fn_bkp, "%s/mixw_counts.bkp", out_dir);

	unlink(fn_bkp);
    }
    if (tmat_reest) {
	sprintf(fn_bkp, "%s/tmat_counts.bkp", out_dir);

	unlink(fn_bkp);
    }
    if (mean_reest || var_reest) {
	sprintf(fn_bkp, "%s/gauden_counts.bkp", out_dir);


	unlink(fn_bkp);
    }

    sprintf(fn, "%s/ckpt", out_dir);
    
    if (ckpt) {
	/* write a file containing the current ctl file offset and
	   # of utts to go */
	if (corpus_ckpt(fn) != S3_SUCCESS) {
	    
	    return S3_ERROR;
	}
    }
    else if (!ckpt) {
	FILE *fp;

	fp = fopen(fn, "r");
	if (fp != NULL) {
	    fclose(fp);

	    if (unlink(fn) < 0) {
		E_ERROR("Unable to unlink checkpoint file\n");
		return S3_ERROR;
	    }
	}
    }

    return S3_SUCCESS;
}


/*********************************************************************
 *
 * Function: 
 *	accum_dump
 * 
 * Description: 
 *	This functions writes the reestimation numerators and
 *	denominators to files.
 *
 *	This routine may be called repeatedly until success to
 *	try to cope with exceptional conditions like insufficient disk
 *	space, etc.
 * 
 * Function Inputs: 
 *	const char *out_dir -
 *	model_inventory_t *inv -
 *	int32 mixw_reest -
 *	int32 tmat_reest -
 *	int32 mean_reest -
 *	int32 var_reest -
 *
 * Global Inputs: 
 * 	None
 *
 * Return Values: 
 *	S3_SUCCESS - The values were written to files successfully.
 *	S3_ERROR   - The values were not completely written to files.
 * 
 * Global Outputs: 
 *	None
 * 
 * Errors: 
 * 
 *********************************************************************/
int32
accum_dump(const char *out_dir,
	   model_inventory_t *inv,
	   int32 mixw_reest,
	   int32 tmat_reest,
	   int32 mean_reest,
	   int32 var_reest,
	   int32 pass2var,
	   int32 var_is_full,
	   int ckpt)  	    /* checkpoint dump flag */
{
    char fn[MAXPATHLEN+1];
    gauden_t *g;

    /* run over the accumulators and report anything exceptional */
    if (!ckpt)
	accum_stat(inv, mixw_reest);

    g = inv->gauden;

    mk_bkp(mixw_reest, tmat_reest, mean_reest, var_reest, out_dir);

    if (mixw_reest) {
	sprintf(fn, "%s/mixw_counts", out_dir);

	if (s3mixw_write(fn,
			 inv->mixw_acc,
			 inv->n_mixw,
			 g->n_feat,
			 g->n_density) != S3_SUCCESS) {

	    revert_bkp(mixw_reest,
		       tmat_reest,
		       mean_reest,
		       var_reest,
		       out_dir);

	    return S3_ERROR;
	}
    }
    else {
	if (!ckpt)
	    E_INFO("Mixture weights not reestimated.  "
		   "No %s/mixw_counts produced.\n",
		   out_dir);
    }

    if (tmat_reest) {
	sprintf(fn, "%s/tmat_counts", out_dir);
	if (s3tmat_write(fn,
			 inv->tmat_acc,
			 inv->n_tmat,
			 inv->n_state_pm) != S3_SUCCESS) {

	    revert_bkp(mixw_reest,
		       tmat_reest,
		       mean_reest,
		       var_reest,
		       out_dir);

	    return S3_ERROR;
	}
    }
    else {
	if (!ckpt)
	    E_INFO("Transition matrices not reestimated.  "
		   "No %s/tmat_counts produced.\n",
		   out_dir);
    }
    
    if (mean_reest || var_reest) {
	int32 rv;

	sprintf(fn, "%s/gauden_counts", out_dir);
	if (var_is_full)
	    rv = s3gaucnt_write_full(fn,
				(mean_reest ? g->macc : NULL),
				(var_reest ? g->fullvacc : NULL),
				pass2var,
				g->dnom,
				g->n_mgau,
				g->n_feat,
				g->n_density,
				g->veclen);
	else
	    rv = s3gaucnt_write(fn,
				(mean_reest ? g->macc : NULL),
				(var_reest ? g->vacc : NULL),
				pass2var,
				g->dnom,
				g->n_mgau,
				g->n_feat,
				g->n_density,
				g->veclen);
	if (rv != S3_SUCCESS) {
	    revert_bkp(mixw_reest,
		       tmat_reest,
		       mean_reest,
		       var_reest,
		       out_dir);

	    return S3_ERROR;
	}
    }
    else {
	if (!ckpt)
	    E_INFO("means and variances not reestimated.  "
		   "No %s/gauden_counts produced.\n",
		   out_dir);
    }
    
    return commit(mixw_reest,
		  tmat_reest,
		  mean_reest,
		  var_reest,
		  ckpt,
		  out_dir);
}

int 
accum_viterbi(uint32 *vit_sseq,
	      uint32 n_vit_sseq,
	      state_t *state,
	      vector_t **obs,
	      uint32 n_obs,
	      model_inventory_t *inv,
	      float64 ****den,
	      uint32 ****den_idx,
	      int32 mixw_reest,
	      int32 tmat_reest,
	      int32 mean_reest,
	      int32 var_reest)
{
	return S3_SUCCESS;
}

int
mmi_accum_gauden(float32 ***denacc,
		 uint32 *lcl2gbl,
		 uint32 n_lcl2gbl,
		 vector_t *frame,
		 uint32 ***den_idx,
		 gauden_t *g,
		 int32 mean_reest,
		 int32 var_reest,
		 float64 arc_gamma,
		 feat_t *fcb)
{
  uint32 g_i, i, j, k, kk, l;
  
  vector_t ***macc = g->l_macc;     /* local to utt */
  vector_t m;
  
  vector_t ***vacc = g->l_vacc;
  vector_t v = NULL;
  
  float32 ***dnom = g->l_dnom;
  
  float32 obs_cnt;

  /* for each density family found in the utterance */
  for (i = 0; i < n_lcl2gbl; i++) {

    g_i = lcl2gbl[i];
    
    /* for each feature */
    for (j = 0; j < gauden_n_feat(g); j++) {
      vector_t feat = frame[j];
      
      /* for each density in the mixture density */
      for (kk = 0; kk < gauden_n_top(g); kk++) {
	
	k = den_idx[i][j][kk];                       /* i.e. density k is one of the n_top densities */
	
	obs_cnt = denacc[i][j][k] * exp(arc_gamma);  /* observation count for density (k) at this time frame
							given the model */
	
	/* don't bother adding a bunch of essentially zero values */
	if (obs_cnt <= MIN_POS_FLOAT32)
	  continue;
	
	m = macc[i][j][k];                          /* the vector accumulator for mean (i,j,k) */
	
	if (var_reest) {
	  v = vacc[i][j][k];                        /* the vector accumulator for variance (i,j,k) */
	}
	
	for (l = 0; l < g->veclen[j]; l++) {
	  if (mean_reest) {
	    m[l] += obs_cnt * feat[l];
	  }
	    
	  if (var_reest) {
	    v[l] += obs_cnt * feat[l] * feat[l];
	  }
	}
	/* accumulate observation count for all densities */
	dnom[i][j][k] += obs_cnt;
      }
    }
  }
  
  return S3_SUCCESS;
}

int32
accum_mmie_dump(const char *out_dir,
		const char *lat_ext,
		model_inventory_t *inv,
		int32 mean_reest,
		int32 var_reest)      /* checkpoint dump flag */
{
  char fn[MAXPATHLEN+1];
  gauden_t *g;
  
  /* run over the accumulators and report anything exceptional */
  accum_stat(inv, FALSE);
  
  g = inv->gauden;
  
  mk_bkp(FALSE, FALSE, mean_reest, var_reest, out_dir);
  
  if (mean_reest || var_reest) {
    int32 rv;
    
    sprintf(fn, "%s/%s_gauden_counts", out_dir, lat_ext);
    
    rv = s3gaucnt_write(fn,
			(mean_reest ? g->macc : NULL),
			(var_reest ? g->vacc : NULL),
			FALSE,
			g->dnom,
			g->n_mgau,
			g->n_feat,
			g->n_density,
			g->veclen);
    if (rv != S3_SUCCESS) {
      revert_bkp(FALSE,
		 FALSE,
		 mean_reest,
		 var_reest,
		 out_dir);
      
      return S3_ERROR;
    }
  }
  else {
    E_INFO("means and variances not reestimated.  "
	   "No %s/%s_gauden_counts produced.\n",
	   out_dir, lat_ext);
  }
  
  return commit(FALSE,
		FALSE,
		mean_reest,
		var_reest,
		FALSE,
		out_dir);
}
