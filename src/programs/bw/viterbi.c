/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * File: viterbi.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include "viterbi.h"
#include "accum.h"

#include <s3/ckd_alloc.h>
#include <s3/profile.h>
#include <s3/remap.h>
#include <s3/err.h>

#include <math.h>
#include <assert.h>

int32
viterbi_update(float64 *out_log_prob,
	       vector_t **f,
	       uint32 *s_seq,
	       uint32 *t_seq,
	       uint32 *ms_seq,
	       uint32 n_obs,
	       model_inventory_t *inv,
	       int32 mixw_reest,
	       int32 tmat_reest,
	       int32 mean_reest,
	       int32 var_reest,
	       int32 pass2var)
{
    int ret = S3_SUCCESS;
    uint32 i, j, k, l, kk, last_ms;
    uint32 t;
    float64 **den;
    uint32 **den_idx;
    float64 out_lik;
    float64 cond_prob;
    float64 total_log_olik = 0;
    float64 xxx;
    float64 diff;
    model_def_t *mdef;
    float32 ***mixw;
    float32 ***l_tmat_acc;
    float32 ***l_mixw_acc;
    vector_t ***l_mean_acc;
    vector_t ***l_var_acc;
    vector_t ***mean;
    float32 ***l_dnom;
    gauden_t *g;
    uint32 tmat_id, p_tmat_id;
    uint32 p_ms, ms, s, l_s;
    float32 ***tmat;
    map_t *mixw_map;	/* structure to map global mixw id's to local utt id's */
    map_t *cb_map;	/* structure to map global outprob id's to outprob id's */
    uint32 cb;
    uint32 l_cb;

    mixw_map = remap_init(n_obs);	/* the largest it could be is one per obs */
    cb_map = remap_init(n_obs);	/* the largest it could be is one per obs */

    mdef = inv->mdef;
    mixw = inv->mixw;
    tmat = inv->tmat;
    g = inv->gauden;
    mean = g->mean;
    last_ms = inv->n_state_pm-1;
    
    den = (float64 **)ckd_calloc_2d(gauden_n_feat(g),
				    gauden_n_top(g),
				    sizeof(float64));
    den_idx = (uint32 **)ckd_calloc_2d(gauden_n_feat(g),
				       gauden_n_top(g),
				       sizeof(uint32));
    if (tmat_reest)
	l_tmat_acc = (float32 ***)ckd_calloc_3d(inv->n_tmat,
						inv->n_state_pm-1,
						inv->n_state_pm,
						sizeof(float32));
    else
	l_tmat_acc = NULL;

    if (mixw_reest)
	l_mixw_acc = (float32 ***)ckd_calloc_3d(n_obs,
						gauden_n_feat(g),
						gauden_n_density(g),
						sizeof(float32));
    else
	l_mixw_acc = NULL;

    gauden_alloc_l_acc(g, n_obs,
		       mean_reest, var_reest, FALSE, FALSE);

    l_dnom = g->l_dnom;

    if (mean_reest)
	l_mean_acc = g->l_macc;
    else
	l_mean_acc = NULL;
	
    if (var_reest)
	l_var_acc = g->l_vacc;
    else
	l_var_acc = NULL;
    
    s = s_seq[0];
    cb = mdef->cb[s];
    t = 0;

    l_s = remap(mixw_map, s);
    l_cb = remap(cb_map, cb);

    gauden_compute(den, den_idx, f[t], g, mdef->cb[s]);
    out_lik = gauden_mixture(den, den_idx, mixw[s], g);
    if (out_lik == 0) {
	E_ERROR("outlik == 0\n");

	ret = S3_ERROR;

	goto free;
    }

    total_log_olik = log(out_lik);
	
    for (j = 0; j < gauden_n_feat(g); j++) {
	/* just deal w/ 1 feature stream for now */
	assert(gauden_n_feat(g) == 1);
	
	for (kk = 0; kk < gauden_n_top(g); kk++) {
	    k = den_idx[j][kk];
	    cond_prob = mixw[s][j][k] * den[j][kk] / out_lik;
	    
	    if (mixw_reest)
		l_mixw_acc[l_s][j][k] += cond_prob;
	    
	    for (l = 0; l < g->veclen[j]; l++) {
		xxx = f[t][j][l] * cond_prob;
		
		if (mean_reest)
		    l_mean_acc[l_cb][j][k][l] += xxx;
		if (var_reest) {
		    if (pass2var) {
			diff = f[t][j][l] - mean[cb][j][k][l];
			
			l_var_acc[l_cb][j][k][l] += cond_prob * diff * diff;
		    }
		    else {
			l_var_acc[l_cb][j][k][l] += xxx * f[t][j][l];
		    }
		}
		if (mean_reest || var_reest)
		    l_dnom[l_cb][j][k] += cond_prob;
	    }
	}
    }

    for (t = 1, p_ms = ms_seq[0], p_tmat_id = t_seq[0]; t < n_obs; t++) {
	ms = ms_seq[t];		/* model state (i.e. 0,1,...n_state_pm-1) */
	tmat_id = t_seq[t];	/* tied transition matrix id @ t */
	s = s_seq[t];		/* global tied state @ t */
	l_s = remap(mixw_map, s);	/* local (to utt) tied state */
	cb = mdef->cb[s];
	l_cb = remap(cb_map, cb);

	if ((p_tmat_id == tmat_id) && (p_ms <= ms)) {
	    /* i.e. intra model transition, need to update */

	    if (tmat_reest) {
		l_tmat_acc[tmat_id][p_ms][ms] += 1.0;
	    }
	    if (tmat[tmat_id][p_ms][ms] > 0.0)
		total_log_olik += log(tmat[tmat_id][p_ms][ms]);
	    
	    if (l_tmat_acc[tmat_id][last_ms-1][0] != 0) {
		E_ERROR("attn: %u %u %u > %u %u\n",
			tmat_id, last_ms-1, 0, p_ms, ms);
	    }
			    
	    /* Begin(HACK) (assumes a particular class of model topologies) */
	    if ((ms == last_ms-1) || (ms == last_ms-2)) {
		/* Current model state a pred. to a non-emitting state.
		 * Need to transition there too. */
		
		/* deal w/ non-emitting final state */
		if (tmat_reest &&
		    tmat[tmat_id][ms][last_ms] > 0.0) {
		    l_tmat_acc[tmat_id][ms][last_ms] += 1.0;
		}

		if (l_tmat_acc[tmat_id][last_ms-1][0] != 0) {
		    E_ERROR("end attn: %u %u %u > %u %u\n",
			    tmat_id, last_ms-1, 0, p_ms, ms);
		}
			    

		if (tmat[tmat_id][ms][last_ms] > 0.0)
		    total_log_olik += log(tmat[tmat_id][ms][last_ms]);
	    }
	    /* End(HACK) */
	}

	gauden_compute(den, den_idx, f[t], g, cb);
	out_lik = gauden_mixture(den, den_idx, mixw[s], g);
	if (out_lik == 0) {
	    E_ERROR("outlik == 0\n");
	    ret = S3_ERROR;

	    goto free;
	}

	total_log_olik += log(out_lik);

	for (j = 0; j < gauden_n_feat(g); j++) {
	    /* just deal w/ 1 feature stream for now */
	    assert(gauden_n_feat(g) == 1);
	    

	    for (kk = 0; kk < gauden_n_top(g); kk++) {
		k = den_idx[j][kk];

		cond_prob = mixw[s][j][k] * den[j][kk] / out_lik;

		if (mixw_reest)
		    l_mixw_acc[l_s][j][k] += cond_prob;
		
		for (l = 0; l < g->veclen[j]; l++) {
		    xxx = f[t][j][l] * cond_prob;
		    
		    if (mean_reest)
			l_mean_acc[l_cb][j][k][l] += xxx;
		    if (var_reest) {
			if (pass2var) {
			    diff = f[t][j][l] - mean[cb][j][k][l];
			    
			    l_var_acc[l_cb][j][k][l] += cond_prob * diff * diff;
			}
			else {
			    l_var_acc[l_cb][j][k][l] += xxx * f[t][j][l];
			}
		    }
		    if (mean_reest || var_reest)
			l_dnom[l_cb][j][k] += cond_prob;
		}
	    }
	}
	p_ms = ms;
	p_tmat_id = tmat_id;
    }

    if (mixw_reest) {
	if (inv->mixw_inverse)
	    ckd_free(inv->mixw_inverse);
	inv->mixw_inverse = remap_inverse(mixw_map, &inv->n_mixw_inverse);
	inv->l_mixw_acc = l_mixw_acc;
	accum_global_mixw(inv, g);
    }

    if (mean_reest || var_reest) {
	inv->cb_inverse = remap_inverse(cb_map, &inv->n_cb_inverse);
    }

    if (mean_reest)
	accum_global_gauden(g->macc, g->l_macc, g,
			    inv->cb_inverse, inv->n_cb_inverse);

    if (var_reest)
	accum_global_gauden(g->vacc, g->l_vacc, g,
			    inv->cb_inverse, inv->n_cb_inverse);
	
    if (mean_reest || var_reest) {
	accum_global_gauden_dnom(g->dnom, g->l_dnom, g,
				 inv->cb_inverse, inv->n_cb_inverse);
    }

    if (tmat_reest) {
	for (i = 0; i < inv->n_tmat; i++) {
	    for (j = 0; j < inv->n_state_pm-1; j++) {
		for (k = 0; k < inv->n_state_pm; k++) {
		    inv->tmat_acc[i][j][k] += l_tmat_acc[i][j][k];
		    if ((j > k) && (inv->tmat_acc[i][j][k] != 0)) {
			E_ERROR("not upper-triangular: [%u %u %u] non-zero\n",
				i, j, k);
		    }
		}
	    }
	}
    }


free:
    if (l_mixw_acc)
	ckd_free_3d((void ***)l_mixw_acc);
    if (l_tmat_acc)
	ckd_free_3d((void ***)l_tmat_acc);

    ckd_free_2d((void **)den_idx);
    ckd_free_2d((void **)den);

    remap_free(mixw_map);
    remap_free(cb_map);

    *out_log_prob = total_log_olik;

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
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/07/29  16:20:55  eht
 * Initial revision
 *
 *
 */
