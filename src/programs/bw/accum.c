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
#include <s3/s3regmat_io.h>
#include <s3/corpus.h>

#include <s3/model_inventory.h>
#include <s3/model_def.h>

#include <s3/ckd_alloc.h>
#include <s3/s2_param.h>
#include <s3/feat.h>
#include <s3/s3.h>

#include <s3/state_seq.h>

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

static float32 *
xfrm_feat(float32 **ainv,
	  float32 *b,
	  float32 *f,
	  uint32 len)
{
    uint32 i, k;
    float32 *o;

    o = ckd_calloc(len, sizeof(float32));

    for (i = 0; i < len; i++) {
	for (k = 0; k < len; k++) {
	    o[i] += ainv[i][k] * (f[k] - b[k]);
	}
    }

    return o;
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
	     float32 ****spkr_xfrm_ainv,
	     float32  ***spkr_xfrm_b,
	     uint32 ***den_idx,
	     gauden_t *g,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 pass2var,
	     float32 ***wacc)
{
    uint32 g_i, i, j, k, kk, l, mc=0;

    vector_t ***macc = g->l_macc;	/* local to utt */
    vector_t m;
    vector_t ***mean = g->mean;
    vector_t pm = NULL;

    vector_t ***vacc = g->l_vacc;
    vector_t v = NULL;

    float32 ***dnom = g->l_dnom;

    float32 diff;
    float32 obs_cnt;
    vector_t feat = NULL;
    vector_t xfeat = NULL;

    /* for each density family found in the utterance */
    for (i = 0; i < n_lcl2gbl; i++) {

	g_i = lcl2gbl[i];

	if (g->mllr_idx)
	    mc = g->mllr_idx[g_i];	/* set MLLR class, if any */

	/* for each feature */
	for (j = 0; j < gauden_n_feat(g); j++) {

	    feat = frame[j];

	    if (spkr_xfrm_ainv || spkr_xfrm_b) {
		/* If there is a speaker MLLR inverse transform
		   in effect, do it */
		if (xfeat) ckd_free((void *)xfeat);

		xfeat = xfrm_feat(spkr_xfrm_ainv[mc][j], spkr_xfrm_b[mc][j],
				  feat, g->veclen[j]);
	    }
	    else {
		/* i.e. do no transform */
		xfeat = feat;
	    }
	    
	    /* for each density in the mixture density */
	    for (kk = 0; kk < gauden_n_top(g); kk++) {
		
		k = den_idx[i][j][kk];	/* i.e. density k is one of the n_top densities */
		
		obs_cnt = denacc[i][j][k];	/* observation count for density (k) at this time frame
						   given the model */
		
		/* don't bother adding a bunch of
		   essentially zero values */
		if (obs_cnt <= MIN_POS_FLOAT32)
		    continue;

#ifdef ACCUM_VERBOSE
		printf("denacc[%u][%u][%3u] == %.3e\n", i, j, k, obs_cnt);
#endif

		m = macc[i][j][k];	/* the vector accumulator for mean (i,j,k) */

		if (pass2var) {
		    g_i = lcl2gbl[i];
		    pm = mean[g_i][j][k];	/* the mean (i, j, k) */
		}

		if (var_reest) {
		    v = vacc[i][j][k];	/* the vector accumulator for variance (i,j,k) */
		}

		for (l = 0; l < g->veclen[j]; l++) {
		    if (mean_reest) {
			/* Reest means based on transformed features; if any */

			m[l] += obs_cnt * xfeat[l];
		    }

		    if (var_reest) {
			/* Always reest vars on untransformed features for now */

			if (!pass2var)
			    v[l] += obs_cnt * feat[l] * feat[l];
			else {
			    diff = feat[l] - pm[l];
			    
			    diff *= diff;
			    v[l] += obs_cnt * diff;
			}
		    }
		}		    
		/* accumulate observation count for all densities */
		dnom[i][j][k] += obs_cnt;
	    }
	}
    }

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
             int32 mllr_mult,    /* MLLR: accumulate A of Ax + B */
             int32 mllr_add)     /* MLLR: accumulate B of Ax + B */
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
	accum_global_gauden(g->vacc, g->l_vacc, g,
			    inv->cb_inverse, inv->n_cb_inverse);
    }
    if (mean_reest || var_reest) {
	/* add local mean/variance denominator accumulators to global ones */
	accum_global_gauden_dnom(g->dnom, g->l_dnom, g,
				 inv->cb_inverse, inv->n_cb_inverse);
    }
    
    if (mllr_mult || mllr_add) {
        accum_global_regmat(g->regr_acc, g->l_regr_acc,
                            g->regl_acc, g->l_regl_acc,
                            g);
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

	    if (k == inv->gauden->n_density) {
		if (n_z == 0) {
		    E_WARN("The following seno never occur in the input data\n\t");
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

#include <s3/mixw_param_io.h>
#include <s3/tmat_param_io.h>

static int32
mk_bkp(int32 mixw_reest,
       int32 tmat_reest,
       int32 mean_reest,
       int32 var_reest,
       int32 mllr_mult,
       int32 mllr_add,
       const char *out_dir)
{
    char fn[MAXPATHLEN+1];
    char fn_bkp[MAXPATHLEN+1];
    FILE *fp;

    if (mixw_reest) {
	sprintf(fn, "%s/mixw_counts", out_dir);
	sprintf(fn_bkp, "%s/mixw_counts.bkp", out_dir);

	fp = fopen(fn, "r");
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

	fp = fopen(fn, "r");
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

	fp = fopen(fn, "r");
	if (fp != NULL) {
	    fclose(fp);
	    if (rename(fn, fn_bkp) < 0) {
		E_ERROR_SYSTEM("Couldn't backup %s\n", fn);
		return S3_ERROR;
	    }
	}
    }
    if (mllr_mult || mllr_add) {
	sprintf(fn, "%s/regmat_counts", out_dir);
	sprintf(fn_bkp, "%s/regmat_counts.bkp", out_dir);

	fp = fopen(fn, "r");
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
	   int32 mllr_mult,
	   int32 mllr_add,
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
    if (mllr_mult || mllr_add) {
	sprintf(fn, "%s/regmat_counts", out_dir);
	sprintf(fn_bkp, "%s/regmat_counts.bkp", out_dir);

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
       int32 mllr_mult,
       int32 mllr_add,
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
    if (mllr_mult || mllr_add) {
	sprintf(fn_bkp, "%s/regmat_counts.bkp", out_dir);

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
	   int32 mllr_mult, /* MLLR: dump A accumulators */
	   int32 mllr_add,  /* MLLR: dump B accumulators */
	   int ckpt)  	    /* checkpoint dump flag */
{
    char fn[MAXPATHLEN+1];
    gauden_t *g;

    /* run over the accumulators and report anything exceptional */
    if (!ckpt)
	accum_stat(inv, mixw_reest);

    g = inv->gauden;

    mk_bkp(mixw_reest, tmat_reest, mean_reest, var_reest, mllr_mult, mllr_add, out_dir);

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
		       mllr_mult,
		       mllr_add,
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
		       mllr_mult,
		       mllr_add,
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
	sprintf(fn, "%s/gauden_counts", out_dir);
	if (s3gaucnt_write(fn,
			   (mean_reest ? g->macc : NULL),
			   (var_reest ? g->vacc : NULL),
			   pass2var,
			   g->dnom,
			   g->n_mgau,
			   g->n_feat,
			   g->n_density,
			   g->veclen) != S3_SUCCESS) {

	    revert_bkp(mixw_reest,
		       tmat_reest,
		       mean_reest,
		       var_reest,
		       mllr_mult,
		       mllr_add,
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

    if (mllr_mult || mllr_add) {
        sprintf(fn, "%s/regmat_counts", out_dir);
        if (s3regmatcnt_write(fn,
			      g->regr_acc,
			      g->regl_acc,
			      g->n_mllr_class,
			      g->n_feat,
			      g->veclen,
			      mllr_mult,
			      mllr_add) != S3_SUCCESS) {
	    revert_bkp(mixw_reest,
		       tmat_reest,
		       mean_reest,
		       var_reest,
		       mllr_mult,
		       mllr_add,
		       out_dir);

            return S3_ERROR;
	}
    }
    else {
	if (!ckpt)
	    E_INFO("MLLR regmat not reestimated.  "
		   "No %s/regmat_counts produced.\n",
		   out_dir);
    }
    
    return commit(mixw_reest,
		  tmat_reest,
		  mean_reest,
		  var_reest,
		  mllr_mult,
		  mllr_add,
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
/*****************************************************************************
 *
 * ADDITION BY BIXA AND VIPUL: July 10, 1996
 * (Basic code by Eric)
 *
 * Function: accum_regmat
 *
 * Description:
 *
 * Function Inputs:
 *      float32 ***denacc -
 *              contains probability of observing the top N codewords
 *              for the given time.  Major index for the array ranges over
 *              all codewords, but only the top N codewords have non-zero
 *              values.
 *
 *      vector_t *frame -
 *              All feature vectors for the current time.
 *
 *      uint32 ***den_idx -
 *              Top N density indices for all feature streams for time t.
 *
 *      gauden_t *g -
 *              Pointer to a structure containing the initial mean/var
 *              estimates for the iteration and mean/var reestimation
 *              numerators and denominators.
 *
 * Global Inputs:
 *      None
 *
 * Return Values:
 *      S3_SUCCESS - if accumulation was successful.
 *
 *      Currently this is the only return value, but may,
 *      want error conditions in the future.
 *
 * Global Outputs:
 *      None
 *
 * Errors:
 *      None
 *
 *********************************************************************/
int
accum_regmat(float32 ***denacc,
	     model_inventory_t *inv,
	     vector_t *frame,
	     uint32 ***den_idx)
{
    uint32 g_i, i, j, k, kk, l, p, q, veclen, mc;
    static uint32 p_veclen = 0;

    vector_t ***mean = NULL;
    vector_t m;
    static float32 **mprod = NULL;

    vector_t ***var = NULL;
    vector_t v;

    float32 *****regl = NULL;
    float32 ****regr = NULL;

    float32 xxx,yyy;	/* the tata of the late 90's */
    float32 obs_cnt;
    vector_t feat;

    uint32 *lcl2gbl;
    uint32 n_lcl2gbl;
    gauden_t *g;
    model_def_t *mdef;

    if (denacc == NULL) {
        E_FATAL ("denacc = NULL; Regression matrix accumulator update failed\n");
    }

    g = inv->gauden;
    mean = g->mean;
    var = g->var;
    regl = g->l_regl_acc;
    regr = g->l_regr_acc;

    lcl2gbl = inv->cb_inverse;
    n_lcl2gbl = inv->n_cb_inverse;

    mdef = inv->mdef;

    /* for each density family found in the frame */
    for (i = 0; i < n_lcl2gbl; i++) {
	g_i = lcl2gbl[i];

	mc = g->mllr_idx[g_i];

	/* for each feature */
	for (j = 0; j < gauden_n_feat(g); j++) {

	    feat = frame[j];	/* The jth independent feature stream */
	    veclen = g->veclen[j];
	    
	    if (p_veclen != veclen) {
		if (mprod) {
		    ckd_free(mprod);
		    mprod = NULL;
		}
		mprod = (float32 **)ckd_calloc_2d(veclen, veclen, sizeof(float32));

		p_veclen = veclen;
	    }
	    else {
		memset((void *)&mprod[0][0], 0, veclen*veclen*sizeof(float32));
	    }
		
	    /* for each density in the mixture density */
	    for (kk = 0; kk < gauden_n_top(g); kk++) {
		
		k = den_idx[i][j][kk];	/* i.e. density k is one of the n_top densities */

		obs_cnt = denacc[i][j][k];	/* observation count for density (k) at this time frame given the model */

		/* don't bother adding a bunch of essentially zero values */
		if (obs_cnt <= MIN_IEEE_NORM_POS_FLOAT32)
		    continue;

#ifdef ACCUM_VERBOSE
		printf("denacc[%u][%u][%3u] == %.3e\n", i, j, k, obs_cnt);
#endif

		m = mean[g_i][j][k];	/* the mean (i,j,k) */
		v = var[g_i][j][k];		/* 1 / (2 * var (i,j,k)) */

		/* factor out (partial) outer product of means */
		for (p = 0; p < veclen; p++) {
		    for (q = p; q < veclen; q++) {
			mprod[p][q] = m[p] * m[q];
		    }
		}

		for (l = 0; l < veclen; l++) {
		    xxx = obs_cnt * v[l];
		    for (p = 0; p < veclen; p++) {
			for (q = p; q < veclen; q++) {
			    /* since regl matrices are symmetric, compute
			       only the upper triangular matrix + diagonal */
			    regl[mc][j][l][p][q] += xxx * mprod[p][q];
			}
			yyy = xxx * m[p];
			regl[mc][j][l][p][veclen] += yyy;
			regr[mc][j][l][p] += yyy * feat[l];
		    }
		    regl[mc][j][l][veclen][veclen] += xxx;
		    regr[mc][j][l][veclen] += xxx * feat[l];
		}
	    }
	}
    }
/*********DDDDD BIXA & VIPUL, DEBUGGER ********/
/*    printf("ACHTUNG: regr[0][0][0][39] = %f feat = %f\n",regr[0][0][0][39],feat[0]); */
/******************** End debug **************/

    return S3_SUCCESS;
}


/*
 * ADDED BY BIXA AND VIPUL: July 10 1996
 * (Basic code by Eric)
 * 
 * Accumulates local MLLR regression matrix accumulators
 * into a global regression matrix accumulator
 */
void
accum_global_regmat(float32 ****regr_acc,
                    float32 ****l_regr_acc,
                    float32 *****regl_acc,
                    float32 *****l_regl_acc,
                    gauden_t *g)
{
    uint32 f, i, j, k, l, m;

    for (m = 0; m < g->n_mllr_class; m++) {
	for (f = 0; f < g->n_feat; f++) {
	    l = g->veclen[f];
	    for (i = 0; i < l; i++) {
		for (j = 0; j <= l; j++) {

		    regr_acc[m][f][i][j] += l_regr_acc[m][f][i][j];

		    for (k = 0; k < j; k++) {
			/* accumulate the upper triangular part to the
			   lower triangular part */
			regl_acc[m][f][i][j][k] += l_regl_acc[m][f][i][k][j];
		    }
		    for (k = j; k <= l; k++) {
			/* Accumulate diagonal and upper triangular part as is */
			regl_acc[m][f][i][j][k] += l_regl_acc[m][f][i][j][k];
		    }
		}
	    }
	}
    }
    
/*********DDDDD BIXA & VIPUL, DEBUGGER ********/
/*    printf("ACHTUNG: regr[0][0][0][39] = %f\n",regr_acc[0][0][0][39]); */
/******************** End debug **************/
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
 * Revision 1.21  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.20  96/10/21  14:17:25  eht
 * Changes to compute diagonal and upper triangular portions of
 * symmetric MLLR regression accumulator matrices.
 * 
 * Some minor name changes
 * 
 * Revision 1.19  1996/08/27  19:14:51  eht
 * Bug fix of MLLR accumulation from Bixa and Vipul.
 *
 * Revision 1.18  1996/08/27  19:00:51  eht
 * Separate CD from CI for MLLR accumulation
 * c
 *
 * Revision 1.17  1996/07/29  16:07:15  eht
 * Changes to incorporation initial MLLR reestimation
 * Changes to fix bug in reest variances based on prior mean
 * Changes to remove accum_trans() and accum_non_emit_trans() functions
 *     because they are inappropriate.
 *
 * Revision 1.16  1996/03/26  13:46:46  eht
 * Deal w/ the case when there are many fewer densities referenced in an utterance
 * than total # of densities
 *
 * Revision 1.15  1996/03/04  16:00:31  eht
 * Fixed seg fault when not reestimating variances
 *
 * Revision 1.14  1996/01/26  18:23:49  eht
 * Deal w/ accumulating CI mixture Gaussian counts when only CD mixture Gaussians are present.
 *
 * Revision 1.13  1996/01/04  20:04:18  eht
 * Minor formatting change
 *
 * Revision 1.12  1995/12/14  19:49:47  eht
 * Added clear of gaussian density accumulator if error occurs.  O/W
 * the accumulated results of aborted utterance will be added to
 * the next utterance accumulators
 *
 * Revision 1.11  1995/12/14  19:27:08  eht
 * Fix seg faults when not reestimating certain classes of parameters.
 *
 * Revision 1.10  1995/11/30  20:49:48  eht
 * Modularized a bit more
 * Added changes to deal with non-allocation of accumulators when
 *     parameter reestimation is turned off (e.g. mixw reest is turned
 *     off for codebook adaptation).
 *
 * Revision 1.9  1995/10/17  14:02:26  eht
 * Changed so that would port to Windows NT
 *
 * Revision 1.8  1995/10/10  12:43:50  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.7  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.6  1995/08/29  21:09:50  eht
 * Yet more comment revisions
 *
 * Revision 1.5  1995/08/29  20:24:58  eht
 * Removed erroneous end comment characters and placed definition
 * of accum_stat() above references
 *
 * Revision 1.4  1995/08/29  20:20:35  eht
 * Add more comments
 *
 * Revision 1.3  1995/08/09  20:15:05  eht
 * Correct some comments
 *
 * Revision 1.2  1995/06/28  14:31:27  eht
 * Commented a bit more
 *
 * Revision 1.1  1995/06/02  20:41:22  eht
 * Initial revision
 *
 *
 */



