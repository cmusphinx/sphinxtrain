/* -*- c-basic-offset: 4 -*- */
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
 * File: main.c
 * 
 * Description: 
 *	This routine uses a source model definition file, source
 *	mixing weight file and destination model definition file
 *	to initialize a destination mixing weight file.
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/
#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>
#include <s3/was_added.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/ts2cb.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3gau_io.h>

/* Some SPHINX-II compatibility definitions */

#include <stdio.h>
#include <math.h>
#if (!defined(_WIN32) || defined(__CYGWIN__))
#include <sys/param.h>
#endif
#include <assert.h>
#include <string.h>

static int init_mixw(void);

static int
initialize(int argc,
	   char *argv[])
{

    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    return S3_SUCCESS;
}

static int
init_uniform(float32 ***dest_mixw,
	     model_def_entry_t *dest,
	     uint32 n_feat,
	     uint32 n_gau)
{
    float32 uniform = 1.0f / (float32)n_gau;
    unsigned int s, i, j;
    uint32 d_m;

    for (s = 0; s < dest->n_state; s++) {
	d_m = dest->state[s];
	for (i = 0; i < n_feat; i++) {
	    for (j = 0; j < n_gau; j++) {
		if (d_m != TYING_NON_EMITTING)
		    dest_mixw[d_m][i][j] = uniform;
	    }
	}
    }

    return S3_ERROR;
}

static pair_t **mixw_dest_list = NULL;
static pair_t **cb_dest_list = NULL;
static pair_t **tmat_dest_list = NULL;

static int
init_model(float32 ***dest_mixw,
	   vector_t ***dest_mean,
	   vector_t ***dest_var,
	   vector_t ****dest_fullvar,
	   float32 ***dest_tmat,
	   model_def_entry_t *dest,
	   uint32 *dest_cb_map,
	   acmod_set_t *dest_acmod_set,

	   float32 ***src_mixw,
	   vector_t ***src_mean,
	   vector_t ***src_var,
	   vector_t ****src_fullvar,
	   float32 ***src_tmat,
	   model_def_entry_t *src,
	   uint32 *src_cb_map,
	   acmod_set_t *src_acmod_set,
	   
	   uint32 n_feat,
	   uint32 n_gau,
	   uint32 n_state_pm,
	   const uint32 *veclen)
{
    unsigned int s, i, j, k, l, ll;
    unsigned int s_m, s_mg;
    unsigned int d_m, d_mg;
    uint32 s_tmat, d_tmat;

    printf("%10s <- %-10s: ",
	   acmod_set_id2name(dest_acmod_set, dest->p),
	   acmod_set_id2name(src_acmod_set, src->p));

    fflush(stdout);

    s_tmat = src->tmat;
    d_tmat = dest->tmat;

    if (!was_added(&tmat_dest_list[d_tmat], s_tmat)) {
	printf("[tm %5u += %5u]\n", d_tmat, s_tmat);
	for (i = 0; i < n_state_pm-1; i++) {
	    for (j = 0; j < n_state_pm; j++) {
		dest_tmat[d_tmat][i][j] += src_tmat[s_tmat][i][j];
	    }
	}
    }

    for (s = 0; s < src->n_state; s++) {
	s_m = src->state[s];
	d_m = dest->state[s];

	if ((s_m == TYING_NON_EMITTING) &&
	    (d_m == TYING_NON_EMITTING))
	    continue;

	if ((s_m != TYING_NON_EMITTING) &&
	    (d_m != TYING_NON_EMITTING)) {

	    if (!was_added(&mixw_dest_list[d_m], s_m)) {
		printf("[mx %5u(%1u) += %5u] ", d_m, s, s_m);

		for (j = 0; j < n_feat; j++) {
		    for (k = 0; k < n_gau; k++) {
			dest_mixw[d_m][j][k] += src_mixw[s_m][j][k];
		    }
		}
	    }

	    s_mg = src_cb_map[s_m];
	    d_mg = dest_cb_map[d_m];
	    if (!was_added(&cb_dest_list[d_mg], s_mg)) {
		printf("[mg %5u(%1u) <- %5u] ", d_mg, s, s_mg);
		
		for (j = 0; j < n_feat; j++) {
		    for (k = 0; k < n_gau; k++) {
			for (l = 0; l < veclen[j]; l++) {
			    dest_mean[d_mg][j][k][l] = src_mean[s_mg][j][k][l];
			    if (dest_var)
				dest_var[d_mg][j][k][l] = src_var[s_mg][j][k][l];
			    else if (dest_fullvar) {
				for (ll = 0; ll < veclen[j]; ++ll) {
				    dest_fullvar[d_mg][j][k][l][ll]
					= src_fullvar[s_mg][j][k][l][ll];
				}
			    }
			}
		    }
		}
	    }
	}
	else {
	    E_ERROR("Source is %semitting and destination is %semitting\n",
		    (s_m != TYING_NON_EMITTING ? "" : "non-"),
		    (d_m != TYING_NON_EMITTING ? "" : "non-"));
	}
    }

    printf("\n");

    return S3_SUCCESS;
}

static int
init_mixw()
{
    model_def_t *src_mdef;
    float32 ***src_mixw;
    vector_t ***src_mean;
    vector_t ***src_var = NULL;
    vector_t ****src_fullvar = NULL;
    float32 ***src_tmat;

    model_def_t *dest_mdef;
    float32 ***dest_mixw;
    vector_t ***dest_mean;
    vector_t ***dest_var = NULL;
    vector_t ****dest_fullvar = NULL;
    float32 ***dest_tmat;

    uint32 n_mixw_src;
    uint32 n_mixw_dest;
    uint32 n_feat;
    uint32 tmp_n_feat;
    uint32 n_gau;
    uint32 tmp_n_gau;
    uint32 n_cb_src;
    uint32 n_cb_dest;
    uint32 n_state_pm;
    uint32 n_tmat_src;
    uint32 n_tmat_dest;
    uint32 *veclen;
    uint32 *tmp_veclen;

    uint32 m;
    uint32 dest_m;
    uint32 dest_m_base;
    uint32 src_m;
    acmod_id_t src_m_base;
    const char *dest_m_name;
    const char *dest_m_base_name;

    uint32 i;
    uint32 n_ts;
    uint32 n_cb;

    const char *ts2cbfn;

    E_INFO("Reading src %s\n", cmd_ln_str("-src_moddeffn"));

    /* read in the source model definition file */
    if (model_def_read(&src_mdef,
		       cmd_ln_str("-src_moddeffn")) != S3_SUCCESS) {
	return S3_ERROR;
    }

	   
    ts2cbfn = cmd_ln_str("-src_ts2cbfn");
    if (strcmp(SEMI_LABEL, ts2cbfn) == 0) {
	E_INFO("Generating semi-continous ts2cb mapping\n");
	src_mdef->cb = semi_ts2cb(src_mdef->n_tied_state);
	n_ts = src_mdef->n_tied_state;
	n_cb = 1;
    }
    else if (strcmp(CONT_LABEL, ts2cbfn) == 0) {
	E_INFO("Generating continous ts2cb mapping\n");
	src_mdef->cb = cont_ts2cb(src_mdef->n_tied_state);
	n_ts = src_mdef->n_tied_state;
	n_cb = src_mdef->n_tied_state;
    }
    else if (strcmp(PTM_LABEL, ts2cbfn) == 0) {
	E_INFO("Generating phonetically tied ts2cb mapping\n");
	src_mdef->cb = ptm_ts2cb(src_mdef);
	n_ts = src_mdef->n_tied_state;
	n_cb = src_mdef->acmod_set->n_ci;
    }
    else {
	E_INFO("Reading src %s\n", cmd_ln_str("-src_ts2cbfn"));
	if (s3ts2cb_read(ts2cbfn,
		     &src_mdef->cb,
		     &n_ts,
		     &n_cb) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }

    E_INFO("Reading src %s\n", cmd_ln_str("-src_mixwfn"));

    /* read in the source mixing weight parameter file */
    if (s3mixw_read(cmd_ln_str("-src_mixwfn"),
		    &src_mixw, &n_mixw_src, &n_feat, &n_gau) != S3_SUCCESS) {
			
	return S3_ERROR;
    }

    E_INFO("Reading src %s\n",
	   cmd_ln_str("-src_tmatfn"));
    
    if (s3tmat_read(cmd_ln_str("-src_tmatfn"),
		    &src_tmat,
		    &n_tmat_src,
		    &n_state_pm) != S3_SUCCESS) {
	return S3_ERROR;
    }

    E_INFO("Reading src %s\n", cmd_ln_str("-src_meanfn"));

    if (s3gau_read(cmd_ln_str("-src_meanfn"),
		   &src_mean,
		   &n_cb_src,
		   &tmp_n_feat,
		   &tmp_n_gau,
		   &veclen) != S3_SUCCESS) {
	return S3_ERROR;
    }

    if (tmp_n_feat != n_feat) {
	E_FATAL("src mean n_feat (== %u) != prior value (== %u)\n",
		tmp_n_feat, n_feat);
    }
    if (tmp_n_gau != n_gau) {
	E_FATAL("src mean n_gau (== %u) != prior value (== %u)\n",
		tmp_n_gau, n_gau);
    }
    if (n_cb_src != n_cb) {
	E_FATAL("src mean n_cb (== %u) is inconsistent with ts2cb mapping %u. Most probably phoneset has duplicated phones\n",
		n_cb_src, n_cb);
    }

    E_INFO("Reading src %s\n", cmd_ln_str("-src_varfn"));
    if (cmd_ln_int32("-fullvar")) {
	if (s3gau_read_full(cmd_ln_str("-src_varfn"),
		       &src_fullvar,
		       &n_cb_src,
		       &tmp_n_feat,
		       &tmp_n_gau,
		       &tmp_veclen) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }
    else {
	if (s3gau_read(cmd_ln_str("-src_varfn"),
		       &src_var,
		       &n_cb_src,
		       &tmp_n_feat,
		       &tmp_n_gau,
		       &tmp_veclen) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }

    if (tmp_n_feat != n_feat) {
	E_FATAL("src var n_feat (== %u) != prior value (== %u)\n",
		tmp_n_feat, n_feat);
    }
    if (tmp_n_gau != n_gau) {
	E_FATAL("src var n_gau (== %u) != prior value (== %u)\n",
		tmp_n_gau, n_gau);
    }
    if (n_cb_src != n_cb) {
	E_FATAL("src var n_cb (== %u) inconsistent w/ ts2cb mapping %u\n",
		n_cb_src, n_cb);
    }

    if (n_mixw_src < src_mdef->n_tied_state) {
	E_FATAL("Too few source mixing weights, %u, for the # of tied states, %u\n",
		n_mixw_src, src_mdef->n_tied_state);
    }

    for (i = 0; i < n_feat; i++) {
	if (veclen[i] != tmp_veclen[i]) {
	    E_FATAL("src var veclen[%u] (== %u) != prior value (== %u)\n",
		    i, tmp_veclen[i], veclen[i]);
	}
    }
    ckd_free(tmp_veclen);

    E_INFO("Reading dest %s\n",
	   cmd_ln_str("-dest_moddeffn"));

    /* read in the destination model definition file */
    if (model_def_read(&dest_mdef,
		       cmd_ln_str("-dest_moddeffn")) < S3_SUCCESS) {
	return S3_ERROR;
    }

    ts2cbfn = cmd_ln_str("-dest_ts2cbfn");
    if (strcmp(SEMI_LABEL, ts2cbfn) == 0) {
	E_INFO("Generating semi-continous ts2cb mapping\n");
	dest_mdef->cb = semi_ts2cb(dest_mdef->n_tied_state);
	n_ts = dest_mdef->n_tied_state;
	n_cb = 1;
    }
    else if (strcmp(CONT_LABEL, ts2cbfn) == 0) {
	E_INFO("Generating continous ts2cb mapping\n");
	dest_mdef->cb = cont_ts2cb(dest_mdef->n_tied_state);
	n_ts = dest_mdef->n_tied_state;
	n_cb = dest_mdef->n_tied_state;
    }
    else if (strcmp(PTM_LABEL, ts2cbfn) == 0) {
	E_INFO("Generating phonetically tied ts2cb mapping\n");
	dest_mdef->cb = ptm_ts2cb(dest_mdef);
	n_ts = dest_mdef->n_tied_state;
	n_cb = dest_mdef->acmod_set->n_ci;
    }
    else {
	E_INFO("Reading dest %s\n",
	       cmd_ln_str("-dest_ts2cbfn"));


	if (s3ts2cb_read(ts2cbfn,
			 &dest_mdef->cb,
			 &n_ts,
			 &n_cb) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }

    E_INFO("Calculating initial model parameters\n");

    n_tmat_dest = dest_mdef->n_tied_tmat;
    tmat_dest_list = init_was_added(n_tmat_dest);
    
    E_INFO("Alloc %ux%ux%u dest tmat\n",
	   n_tmat_dest,
	   n_state_pm-1,
	   n_state_pm);

    dest_tmat = (float32 ***)ckd_calloc_3d(n_tmat_dest,
					   n_state_pm-1,
					   n_state_pm,
					   sizeof(float32));
    
    n_mixw_dest = dest_mdef->n_tied_state;
    mixw_dest_list = init_was_added(n_mixw_dest);

    E_INFO("Alloc %ux%ux%u dest mixw\n",
	   n_mixw_dest, n_feat, n_gau);
    dest_mixw = (float32 ***)ckd_calloc_3d(n_mixw_dest, n_feat, n_gau, sizeof(float32));

    for (i = 0, n_cb_dest = 0; i < n_mixw_dest; i++) {
	if (dest_mdef->cb[i] != -1 && dest_mdef->cb[i] > n_cb_dest) {
	    n_cb_dest = dest_mdef->cb[i];
	}
    }
    ++n_cb_dest;

    cb_dest_list = init_was_added(n_cb_dest);

    E_INFO("Alloc %ux%ux%u dest mean and var\n",
	   n_cb_dest, n_feat, n_gau);
    dest_mean = gauden_alloc_param(n_cb_dest, n_feat, n_gau, veclen);
    if (src_var)
	dest_var = gauden_alloc_param(n_cb_dest, n_feat, n_gau, veclen);
    else if (src_fullvar)
	dest_fullvar = gauden_alloc_param_full(n_cb_dest, n_feat, n_gau, veclen);
    
    for (dest_m = 0; dest_m < dest_mdef->n_defn; dest_m++) {
	dest_m_name = acmod_set_id2name(dest_mdef->acmod_set, dest_m);
	src_m = acmod_set_name2id(src_mdef->acmod_set, dest_m_name);
	if (src_m == NO_ACMOD) {
	    /* No corresponding phone model in the source set */

	    /* See if there is a source base phone corresponding to this destination model
	       base phone */
	    dest_m_base = acmod_set_base_phone(dest_mdef->acmod_set, dest_m);
	    dest_m_base_name = acmod_set_id2name(dest_mdef->acmod_set, dest_m_base);

	    src_m_base = acmod_set_name2id(src_mdef->acmod_set, dest_m_base_name);
	    if (src_m_base == NO_ACMOD) {
		/* No corresponding model or base model found.  Use uniform distribution */

		E_INFO("No source base phone %s found.  Initializing %s using uniform distribution\n",
		       dest_m_base_name, dest_m_name);
		
		if (src_tmat) {
		    E_INFO("Uniform initialization of tmat not supported\n");
		}
		init_uniform(dest_mixw, &dest_mdef->defn[dest_m], n_feat, n_gau);
	    }
	    else {
		/* No corresponding model, but a base model was found.  Use base distribution. */
		init_model(dest_mixw, dest_mean, dest_var, dest_fullvar, dest_tmat,
			   &dest_mdef->defn[dest_m], dest_mdef->cb, dest_mdef->acmod_set,
			   src_mixw, src_mean, src_var, src_fullvar, src_tmat,
			   &src_mdef->defn[src_m_base], src_mdef->cb, src_mdef->acmod_set,
			   n_feat, n_gau, n_state_pm, veclen);
	    }
	}
	else {
	    /* Found a corresponding model in the source set, so use source distributions to init
	       the destination */
	    init_model(dest_mixw, dest_mean, dest_var, dest_fullvar, dest_tmat,
		       &dest_mdef->defn[dest_m], dest_mdef->cb, dest_mdef->acmod_set,
		       src_mixw, src_mean, src_var, src_fullvar, src_tmat,
		       &src_mdef->defn[src_m], src_mdef->cb, src_mdef->acmod_set,
		       n_feat, n_gau, n_state_pm, veclen);
	}
    }

    for (m = 0; m < n_mixw_dest; m++) {
	if (mixw_dest_list[m] == NULL) {
	    E_WARN("Destination state %u has not been initialized!\n", m);
	}
    }

    for (m = 0; m < n_cb_dest; m++) {
	if (cb_dest_list[m] == NULL) {
	    E_WARN("Destination cb %u has not been initialized!\n", m);
	}
	else if (cb_dest_list[m]->next != NULL) {
	    E_WARN("dest cb %u has > 1 corresponding source cb\n", m);
	}
    }

    E_INFO("Writing dest %s\n",
	   cmd_ln_str("-dest_tmatfn"));
    
    if (s3tmat_write(cmd_ln_str("-dest_tmatfn"),
		     dest_tmat,
		     n_tmat_dest,
		     n_state_pm) != S3_SUCCESS) {
	return S3_ERROR;
    }
	   

    E_INFO("Writing dest %s\n",
	   cmd_ln_str("-dest_mixwfn"));

    if (s3mixw_write(cmd_ln_str("-dest_mixwfn"),
		     dest_mixw,
		     dest_mdef->n_tied_state, n_feat, n_gau) < S3_SUCCESS) {
	return S3_ERROR;
    }

    E_INFO("Writing dest %s\n",
	   cmd_ln_str("-dest_meanfn"));

    if (s3gau_write(cmd_ln_str("-dest_meanfn"),
		    (const vector_t ***)dest_mean,
		    n_cb_dest,
		    n_feat,
		    n_gau,
		    veclen) != S3_SUCCESS) {
	return S3_ERROR;
    }

    E_INFO("Writing dest %s\n",
	   cmd_ln_str("-dest_varfn"));
    if (cmd_ln_int32("-fullvar")) {
	if (s3gau_write_full(cmd_ln_str("-dest_varfn"),
			     (const vector_t ****)dest_fullvar,
			     n_cb_dest,
			     n_feat,
			     n_gau,
			     veclen) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }
    else {
	if (s3gau_write(cmd_ln_str("-dest_varfn"),
			(const vector_t ***)dest_var,
			n_cb_dest,
			n_feat,
			n_gau,
			veclen) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }
    ckd_free(veclen);
    if (src_mean)
        gauden_free_param(src_mean);
    if (src_var)
        gauden_free_param(src_var);
    if (dest_mean)
        gauden_free_param(dest_mean);
    if (dest_var)
        gauden_free_param(dest_var);
    if (src_mdef)
        model_def_free(src_mdef);
    if (dest_mdef)
        model_def_free(dest_mdef);
    if (src_mixw)
	ckd_free_3d(src_mixw);
    if (dest_mixw)
	ckd_free_3d(dest_mixw);
    if (src_tmat)
	ckd_free_3d(src_tmat);
    if (dest_tmat)
	ckd_free_3d(dest_tmat);
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("errors initializing.\n");
	return 1;
    }
    
    if (init_mixw() != S3_SUCCESS) {
	return 1;
    }

    return 0;
}
