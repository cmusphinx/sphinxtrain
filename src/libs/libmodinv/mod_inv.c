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
 * File: mod_inv.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include <s3/model_inventory.h>
#include <s3/ckd_alloc.h>
#include <s3/s2_param.h>
#include <s3/common.h>

#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3regmat_io.h>
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
		    uint32 n_top)
{
    vector_t ***mean;
    vector_t ***var;
    uint32 n_mgau, i;
    uint32 n_feat, j;
    uint32 n_density, k;
    const uint32 *veclen, *vl;
    gauden_t *g;
    
    if (s3gau_read(cmd_ln_access("-meanfn"),
		   &mean,
		   &n_mgau,
		   &n_feat,
		   &n_density,
		   &veclen) != S3_SUCCESS) {
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
	E_WARN("Model inventory n_density not set; setting to mean file value, %u.\n",
	       n_density);

	minv->n_density = n_density;
    }

    if (s3gau_read(varfn,
		   &var,
		   &i,
		   &j,
		   &k,
		   &vl) != S3_SUCCESS) {
	return S3_ERROR;
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
	if (vl[i] != veclen[i]) {
	    E_FATAL("Vector length of feature %u is %u in mean file but %u in variance file\n",
		    i, veclen[i], vl[i]);
	}
    }

    ckd_free((void *)vl);

    g = minv->gauden;

    /* configure the gauden structure */

    gauden_set_n_mgau(g, n_mgau);
    gauden_set_feat(g, n_feat, veclen);
    gauden_set_n_density(g, n_density);

    gauden_set_mean(g, mean);
    gauden_set_var(g, var);

    gauden_set_min_var(varfloor);
    gauden_floor_variance(g);

    if (n_top > n_density) {
	E_WARN("n_top %u > n_density %u.  n_top <- %u\n",
	       n_top, n_density, n_density);
	gauden_set_n_top(g, n_density);
    }
    else {
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

    if (s3mixw_read(cmd_ln_access("-mixwfn"),
		    &mixw,
		    &n_mixw,
		    &n_feat,
		    &n_density) != S3_SUCCESS) {
	return S3_ERROR;
    }
    minv->mixw = mixw;
    minv->n_mixw = n_mixw;

    if (n_mixw != mdef->n_tied_state) {
	E_WARN("# of tied states in mdef file, %u != # of mixing weight sets, %u, in %s\n",
	       mdef->n_tied_state, n_mixw, fn);
    }

    if ((minv->n_feat > 0) && (n_feat != minv->n_feat)) {
	E_FATAL("# of features in mixw file, %u, is inconsistent w/ prior setting, %u\n",
		n_feat, minv->n_feat);
    }
    else if (minv->n_feat == 0) {
	E_WARN("Model inventory n_feat not set; setting to value in mixw file, %u.\n",
	       n_feat);

	minv->n_feat = n_feat;
    }

    if ((minv->n_density > 0) && (n_density != minv->n_density)) {
	E_WARN("# of densities/mixture, %u in %s != prior settting, %u\n",
	       n_density, fn, minv->n_density);
    }
    else if (minv->n_density == 0) {
	E_WARN("Model inventory n_density not set; setting to value in mixw file, %u.\n",
	       n_density);

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
	    E_INFO("Norm failed for %d mixw:", n_err_norm);
	    for (i = 0; i < n_err_norm; i++) {
		fprintf(stderr, " %u", err_norm[i]);
	    }
	    fprintf(stderr, "\n");
	    fflush(stderr);

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
		    int mllr_mult,
		    int mllr_add)
{
    char fn[MAXPATHLEN+1];
    uint32 n_mixw;
    uint32 n_feat;
    uint32 n_density;

    uint32 n_tmat;
    uint32 n_state_pm;

    uint32 n_cb;
    const uint32 *veclen, *rd_veclen;

    uint32 mult, add;

    int ret = S3_SUCCESS;

    uint32 i;
    uint32 n_class;
    int32 pass2var;
    
    veclen = feat_vecsize();

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
    if (mean_reest || var_reest || mllr_mult || mllr_add) {
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
	ckd_free((void *)rd_veclen);
    }

    if (mllr_mult || mllr_add) {
	if (s3regmatcnt_read(fn,
			     &(minv->gauden->regr_acc),
			     &(minv->gauden->regl_acc),
			     &n_class,
			     &n_feat,
			     &rd_veclen,
			     &mult,
			     &add) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	
	if (n_feat != minv->n_feat) {
	    E_ERROR("Checkpointed n_feat=%u inconsistent w/ trainer config=%u\n",
		    n_feat, minv->n_feat);
	    ret = S3_ERROR;
	}

	for (i = 0; i < n_feat; i++) {
	    if (veclen[i] != rd_veclen[i]) {
		E_ERROR("Checkpointed veclen[%u]=%u inconsistent w/ trainer config veclen[%u]=%u\n",
			i, rd_veclen[i], i, veclen[i]);
		ret = S3_ERROR;
	    }
	}
	ckd_free((void *)rd_veclen);

	if ((mllr_mult && !mult) ||
	    (!mllr_mult && mult)) {
	    E_ERROR("Checkpointed mllr_mult=%u inconsistent with trainer config=%u\n",
		    mult, mllr_mult);
	    ret = S3_ERROR;
	}
	if ((mllr_add && !add) ||
	    (!mllr_add && add)) {
	    E_ERROR("Checkpointed mllr_add=%u inconsistent with trainer config=%u\n",
		    add, mllr_add);
	    ret = S3_ERROR;
	}
    }

    return ret;
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
 * Revision 1.7  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.6  1996/07/29  16:46:51  eht
 * Put reading and initialization of model parameter code here
 *
 * Revision 1.5  1995/10/18  11:23:10  eht
 * Changed the commenting conventions
 *
 * Revision 1.4  1995/10/09  15:11:53  eht
 * Changed interface to ckd_alloc to remove need for __FILE__, __LINE__ arguments
 *
 * Revision 1.3  1995/07/07  17:58:47  eht
 * Get rid of tying_init() since DAG is no longer used
 *
 * Revision 1.2  1995/06/02  16:48:06  eht
 * model inventory functions
 *
 * Revision 1.1  1995/02/06  14:26:45  eht
 * Initial revision
 *
 *
 */
