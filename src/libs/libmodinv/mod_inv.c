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
 * File: mod_inv.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include <s3/model_inventory.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/common.h>

#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3gau_io.h>
#include <s3/model_def.h>

#include <sys_compat/file.h>

#include <stdio.h>

model_inventory_t *
mod_inv_new()
{
    model_inventory_t *new_mi = ckd_calloc(1, sizeof(model_inventory_t));

    new_mi->gauden = gauden_alloc();

    return new_mi;
}

void
mod_inv_free(model_inventory_t *minv)
{
    /* Free mixing weight related stuff */
    if (minv->mixw) {
	ckd_free_3d((void ***)minv->mixw);
    }
    minv->mixw = NULL;

    if (minv->mixw_acc) {
	ckd_free_3d((void ***)minv->mixw_acc);
    }
    minv->mixw_acc = NULL;

    if (minv->l_mixw_acc) {
	ckd_free_3d((void ***)minv->l_mixw_acc);
    }
    minv->l_mixw_acc = NULL;

    if (minv->mixw_inverse)
	ckd_free((void *)minv->mixw_inverse);
    minv->mixw_inverse = NULL;

    /* Free density related stuff */
    gauden_free(minv->gauden);
    minv->gauden = NULL;

    if (minv->cb_inverse)
	ckd_free((void *)minv->cb_inverse);
    minv->cb_inverse = NULL;

    /* Free transition matrix related stuff */
    if (minv->tmat) {
	ckd_free_3d((void ***)minv->tmat);
    }
    minv->tmat = NULL;

    if (minv->tmat_acc) {
	ckd_free_3d((void ***)minv->tmat_acc);
    }
    minv->tmat_acc = NULL;

    if (minv->l_tmat_acc) {
	ckd_free_2d((void **)minv->l_tmat_acc);
    }
    minv->l_tmat_acc = NULL;

    /* Free the top-level inventory structure */
    ckd_free(minv);
}

void
mod_inv_set_n_feat(model_inventory_t *minv,
		   uint32 n_feat)
{
    minv->n_feat = n_feat;
}

void
mod_inv_set_n_density(model_inventory_t *minv,
		      uint32 n_density)
{
    minv->n_density = n_density;
}

int32
mod_inv_alloc_gauden_acc(model_inventory_t *minv)
{
    /* allocate accumulators for mean/variance reestimation sums */

    if (gauden_alloc_acc(minv->gauden) != S3_SUCCESS)
	return S3_ERROR;

    return S3_SUCCESS;
}

int32
mod_inv_read_gauden(model_inventory_t *minv,
		    const char *meanfn,
		    const char *varfn,
		    float32 varfloor,
		    uint32 n_top,
		    int32 var_is_full)
{
    vector_t ***mean;
    vector_t ***var = NULL, ****fullvar = NULL;
    uint32 n_mgau, i;
    uint32 n_feat, j;
    uint32 n_density, k;
    uint32 *v1, *v2;
    gauden_t *g;
    
    if (s3gau_read(meanfn,
		   &mean,
		   &n_mgau,
		   &n_feat,
		   &n_density,
		   &v1) != S3_SUCCESS) {
	return S3_ERROR;
    }

    if ((minv->n_feat > 0) && (n_feat != minv->n_feat)) {
	E_FATAL("# of features in mean file %u is inconsistent w/ inventory, %u\n",
		n_feat, minv->n_feat);
    }
    else if (minv->n_feat == 0) {
	E_WARN("Model inventory n_feat not set; setting to mean file value, %u.\n",
	       n_feat);
	
	minv->n_feat = n_feat;
    }

    if ((minv->n_density > 0) && (n_density != minv->n_density)) {
	E_FATAL("mean file contains a different # of Gaussian densities, %u, per mixture than previously defined, %u\n", n_density, minv->n_density);
    }
    else if (minv->n_density == 0) {
	/* This warning is bogus. */
        /* E_WARN("Model inventory n_density not set; setting to mean file value, %u.\n",
	   n_density); */

	minv->n_density = n_density;
    }

    if (var_is_full) {
	if (s3gau_read_maybe_full(varfn,
		    		  &fullvar,
		    		  &i,
		    		  &j,
		    		  &k,
		    		  &v2, FALSE) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }
    else {
	if (s3gau_read(varfn,
		       &var,
		       &i,
		       &j,
		       &k,
		       &v2) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }

    if (j != minv->n_feat) {
	E_FATAL("# of features in mean file %u is incompatible w/ prior setting, %u\n",
		j, minv->n_feat);
    }
    
    if (k != minv->n_density) {
	E_FATAL("variance file contains a different # of Gaussian densities, %u, per mixture than prior setting, %u\n", minv->n_density);
    }

    if (i != n_mgau) {
	E_FATAL("mean and variance file have an inconsistent # mgau, %u and %u resp.\n",
		n_mgau, i);
    }
	
    for (i = 0; i < n_feat; i++) {
	if (v1[i] != v2[i]) {
	    E_FATAL("Vector length of feature %u is %u in mean file but %u in variance file\n",
		    i, v1[i], v2[i]);
	}
    }


    g = minv->gauden;

    /* configure the gauden structure */

    gauden_set_n_mgau(g, n_mgau);
    gauden_set_feat(g, n_feat, v1);
    gauden_set_n_density(g, n_density);

    ckd_free(v1);
    ckd_free(v2);

    gauden_set_mean(g, mean);
    if (var_is_full)
	gauden_set_fullvar(g, fullvar);
    else
	gauden_set_var(g, var);

    gauden_set_min_var(varfloor);
    gauden_floor_variance(g);

    if (n_top > n_density) {
	E_WARN("topn %u is greater than n_density %u, using the latter\n",
	       n_top, n_density);
	gauden_set_n_top(g, n_density);
    } else if (var_is_full && n_top != n_density) {
	E_WARN("topn %u is not supported for full covariance using the n_density %u\n",
	       n_top, n_density);
	gauden_set_n_top(g, n_density);
    } else {
	gauden_set_n_top(g, n_top);
    }

    return S3_SUCCESS;
}

int32
mod_inv_alloc_tmat_acc(model_inventory_t *minv)
{
    minv->tmat_acc = (float32 ***)ckd_calloc_3d(minv->n_tmat,
						minv->n_state_pm-1,
						minv->n_state_pm,
						sizeof(float32));

    return S3_SUCCESS;
}

int32
mod_inv_read_tmat(model_inventory_t *minv,
		  const char *fn,
		  float32 floor)
{
    float32 ***tmat;
    uint32 n_tmat;
    uint32 n_state_pm;
    uint32 i, j;

    if (s3tmat_read(fn,
		    &tmat,
		    &n_tmat,
		    &n_state_pm) != S3_SUCCESS) {
	return S3_ERROR;
    }

    if (floor != 0) {
	E_INFO("inserting tprob floor %e and renormalizing\n",
	       floor);
    
	for (i = 0; i < n_tmat; i++) {
	    for (j = 0; j < n_state_pm-1; j++) {
		vector_normalize(tmat[i][j], n_state_pm);
		vector_nz_floor(tmat[i][j], n_state_pm, floor);
		vector_normalize(tmat[i][j], n_state_pm);
	    }
	}
    }

    minv->tmat = tmat;
    minv->n_tmat = n_tmat;
    minv->n_state_pm = n_state_pm;

    return S3_SUCCESS;
}

int32
mod_inv_alloc_mixw_acc(model_inventory_t *minv)
{
    minv->mixw_acc = (float32 ***)ckd_calloc_3d(minv->n_mixw,
						minv->n_feat,
						minv->n_density,
						sizeof(float32));

    return S3_SUCCESS;
}

int32
mod_inv_read_mixw(model_inventory_t *minv,
		  const model_def_t *mdef,
		  const char *fn,
		  float32 floor)
{
    float32 ***mixw;
    uint32 n_mixw, i;
    uint32 n_feat, j;
    uint32 n_density;

    if (s3mixw_read(fn,
		    &mixw,
		    &n_mixw,
		    &n_feat,
		    &n_density) != S3_SUCCESS) {
	return S3_ERROR;
    }
    minv->mixw = mixw;
    minv->n_mixw = n_mixw;

    if (n_mixw != mdef->n_tied_state) {
	E_WARN("Number of tied states in mdef file %u differs from the number of mixtures %u in %s, do files belong to the same model?\n",
	       mdef->n_tied_state, n_mixw, fn);
    }

    if ((minv->n_feat > 0) && (n_feat != minv->n_feat)) {
	E_FATAL("Number of feature streams in mixture_weights file %u differs from the configured value %u, check the command line options\n",
		n_feat, minv->n_feat);
    }
    else if (minv->n_feat == 0) {
	E_WARN("Number of feature streams in the model inventory is not set; setting to the value in mixture_weights file, %u.\n",
	       n_feat);

	minv->n_feat = n_feat;
    }

    if ((minv->n_density > 0) && (n_density != minv->n_density)) {
	E_WARN("Number of densities per mixture, %u in %s differs from the configured value %u\n, check the command line options",
	       n_density, fn, minv->n_density);
    }
    else if (minv->n_density == 0) {
	/* This warning is bogus. */
        /* E_WARN("Model inventory n_density not set; setting to value in mixw file, %u.\n",
	   n_density); */

	minv->n_density = n_density;
    }

    if (floor != 0) {
	uint32 *err_norm = NULL;
	uint32 n_err_norm = 0;
	uint32 err = FALSE;

	for (i = 0; i < n_mixw; i++) {
	    for (j = 0; j < n_feat; j++) {
		if (vector_normalize(mixw[i][j], n_density) != S3_SUCCESS) {
		    err = TRUE;
		}
		vector_floor(mixw[i][j], n_density, floor);
		vector_normalize(mixw[i][j], n_density);
	    }

	    if (err) {
		if (err_norm) {
		    err_norm[n_err_norm++] = i;
		}
		else {
		    /* first use; allocate it */
		    err_norm = ckd_calloc(n_mixw, sizeof(uint32));
		    err_norm[n_err_norm++] = i;
		}

		err = FALSE;
	    }
	}

	if (n_err_norm > 0) {
	    E_INFO("Failed to norm for %d mixw:", n_err_norm);
	    for (i = 0; i < n_err_norm; i++) {
		E_INFOCONT(" %u", err_norm[i]);
	    }
	    E_INFOCONT("\n");

	    ckd_free(err_norm);
	}
    }

    return S3_SUCCESS;
}

int
mod_inv_restore_acc(model_inventory_t *minv,
		    const char *accumdir,
		    int mixw_reest,
		    int mean_reest,
		    int var_reest,
		    int tmat_reest,
		    const uint32 *veclen)
{
    char fn[MAXPATHLEN+1];
    uint32 n_mixw;
    uint32 n_feat;
    uint32 n_density;

    uint32 n_tmat;
    uint32 n_state_pm;

    uint32 n_cb;
    uint32 *rd_veclen;

    int ret = S3_SUCCESS;

    uint32 i;
    int32 pass2var;
    
    if (mixw_reest) {
	ckd_free_3d((void ***)minv->mixw_acc);
	minv->mixw_acc = NULL;

	sprintf(fn, "%s/mixw_counts", accumdir);
	if (s3mixw_read(fn,
			&minv->mixw_acc,
			&n_mixw,
			&n_feat,
			&n_density) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	if (n_mixw != minv->n_mixw) {
	    E_ERROR("Checkpointed n_mixw=%u inconsistent w/ trainer config=%u\n",
		    n_mixw, minv->n_mixw);
	    ret = S3_ERROR;
	}
	if (n_feat != minv->n_feat) {
	    E_ERROR("Checkpointed n_feat=%u inconsistent w/ trainer config=%u\n",
		    n_feat, minv->n_feat);
	    ret = S3_ERROR;

	}
	if (n_density != minv->n_density) {
	    E_ERROR("Checkpointed n_density=%u inconsistent w/ trainer config=%u\n",
		    n_density, minv->n_density);
	    ret = S3_ERROR;

	}
    }
    if (tmat_reest) {
	ckd_free_3d((void ***)minv->tmat_acc);
	minv->tmat_acc = NULL;

	sprintf(fn, "%s/tmat_counts", accumdir);
	if (s3tmat_read(fn,
			&minv->tmat_acc,
			&n_tmat,
			&n_state_pm) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	if (n_tmat != minv->n_tmat) {
	    E_ERROR("Checkpointed n_tmat=%u inconsistent w/ trainer config=%u\n",
		    n_tmat, minv->n_tmat);
	}
	if (n_state_pm != minv->n_state_pm) {
	    E_ERROR("Checkpointed n_state_pm=%u inconsistent w/ trainer config=%u\n",
		    n_state_pm, minv->n_state_pm);
	    ret = S3_ERROR;
	}
    }
    if (mean_reest || var_reest) {
	gauden_free_acc(minv->gauden);

	sprintf(fn, "%s/gauden_counts", accumdir);

	if (s3gaucnt_read(fn,
			  &(minv->gauden->macc),
			  &(minv->gauden->vacc),
			  &pass2var,
			  &(minv->gauden->dnom),
			  &n_cb,
			  &n_feat,
			  &n_density,
			  &rd_veclen) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	if (n_cb != minv->gauden->n_mgau) {
	    E_ERROR("Checkpointed n_cb=%u inconsistent w/ trainer config=%u\n",
		    n_cb, minv->gauden->n_mgau);
	    ret = S3_ERROR;
	}
	if (n_feat != minv->n_feat) {
	    E_ERROR("Checkpointed n_feat=%u inconsistent w/ trainer config=%u\n",
		    n_feat, minv->n_feat);
	    ret = S3_ERROR;
	}
	if (n_density != minv->n_density) {
	    E_ERROR("Checkpointed n_density=%u inconsistent w/ trainer config=%u\n",
		    n_density, minv->n_density);
	    ret = S3_ERROR;
	}
	for (i = 0; i < n_feat; i++) {
	    if (veclen[i] != rd_veclen[i]) {
		E_ERROR("Checkpointed veclen[%u]=%u inconsistent w/ trainer config veclen[%u]=%u\n",
			i, rd_veclen[i], i, veclen[i]);
		ret = S3_ERROR;
	    }
	}
	ckd_free(rd_veclen);
    }

    return ret;
}

