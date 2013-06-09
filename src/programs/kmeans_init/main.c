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
 * 
 * Author: 
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/prim_type.h>
#include <sphinxbase/feat.h>
#include <sphinxbase/err.h>
#include <sphinxbase/profile.h>

#include <s3/lexicon.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3io.h>
#include <s3/mk_phone_seq.h>
#include <s3/ts2cb.h>
#include <s3/mk_sseq.h>
#include <s3/gauden.h>
#include <s3/corpus.h>
#include <s3/segdmp.h>
#include <s3/s3.h>
#include <s3/vector.h>

#include <sys_compat/file.h>
#include <sys_compat/misc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static uint32 stride = 1;
static uint32 l_ts = -1;
static uint32 l_strm = -1;
static float32 *obuf = NULL;

static uint32 multiclass;
static long   data_offset;

static uint32 *n_o2d = NULL;
static uint32 *i_o2d = NULL;
static uint32 **o2d = NULL;

static uint32 n_tot_frame = 0;

static FILE *dmp_fp = NULL;
static uint32 dmp_swp = -1;

static ptmr_t all_timer;
static ptmr_t km_timer;
static ptmr_t var_timer;
static ptmr_t em_timer;

uint32
setup_d2o_map(model_def_t *d_mdef,
	      model_def_t *o_mdef)
{
    model_def_entry_t *o_defn, *d_defn;
    uint32 d_ts;
    uint32 o_ts;
    uint32 *mapped;
    uint32 i, j, k, d;
    const char *nm;
    int did_warn = FALSE;

    if (d_mdef->n_tied_state < o_mdef-> n_tied_state) {
	E_FATAL("more tied states in output than in dump mdef (%u vs %u)\n",
		o_mdef->n_tied_state, d_mdef->n_tied_state);
    }
    if (d_mdef->n_tied_ci_state != o_mdef->n_tied_ci_state) {
	E_FATAL("# tied ci state in output, %u not equal to # in dmp, %u\n",
		o_mdef->n_tied_ci_state, d_mdef->n_tied_ci_state);
    }

    n_o2d  =  (uint32 *)ckd_calloc(o_mdef->n_tied_state, sizeof(uint32));
    i_o2d  =  (uint32 *)ckd_calloc(o_mdef->n_tied_state, sizeof(uint32));
    o2d    = (uint32 **)ckd_calloc(o_mdef->n_tied_state, sizeof(uint32 *));
    mapped =  (uint32 *)ckd_calloc(d_mdef->n_tied_state, sizeof(uint32));

    for (i = 0; i < o_mdef->n_defn; i++) {
	nm = acmod_set_id2name(o_mdef->acmod_set, i);
	
	d = acmod_set_name2id(d_mdef->acmod_set, nm);

	if (d == NO_ID) {
	    if (!did_warn) {
		E_WARN("Some models in the output mdef not in the dump mdef\n");
		did_warn = TRUE;
	    }
	    continue;
	}

	o_defn = &o_mdef->defn[i];
	d_defn = &d_mdef->defn[d];

	for (j = 0; j < o_defn->n_state; j++) {
	    o_ts = o_defn->state[j];
	    d_ts = d_defn->state[j];
	    if ((o_ts != TYING_NO_ID) && (o_ts != TYING_NO_ID)) {
		if (mapped[d_ts] == FALSE) {
		    ++n_o2d[o_ts];
		    mapped[d_ts] = TRUE;
		}
	    }
	    else {
		if (!((o_ts == TYING_NO_ID) && (o_ts == TYING_NO_ID))) {
		    E_INFO("%s state is NULL but %s isn't.\n",
			   (o_ts == TYING_NO_ID ? "output" : "dump"),
			   (o_ts == TYING_NO_ID ? "dump" : "output"));
		}
	    }
	}
    }

    for (i = 0; i < o_mdef->n_tied_state; i++) {
	o2d[i] = (uint32 *)ckd_calloc(n_o2d[i], sizeof(uint32));
    }

    for (i = 0; i < o_mdef->n_defn; i++) {
	/* Figure out the index in the dump mdef
	   for the model in the output mdef */

	nm = acmod_set_id2name(o_mdef->acmod_set, i);
	
	d = acmod_set_name2id(d_mdef->acmod_set, nm);

	if (d == NO_ID) continue;


	o_defn = &o_mdef->defn[i];
	d_defn = &d_mdef->defn[d];

	for (j = 0; j < o_defn->n_state; j++) {
	    o_ts = o_defn->state[j];
	    d_ts = d_defn->state[j];
	    if ((o_ts != TYING_NO_ID) && (o_ts != TYING_NO_ID)) {
		for (k = 0; k < i_o2d[o_ts]; k++) {
		    if (o2d[o_ts][k] == d_ts)
			break;
		}
		if (k == i_o2d[o_ts]) {
		    o2d[o_ts][i_o2d[o_ts]++] = d_ts;
		}
	    }
	    else {
		if (!((o_ts == TYING_NO_ID) && (o_ts == TYING_NO_ID))) {
		    E_INFO("%s state is NULL but %s isn't.\n",
			   (o_ts == TYING_NO_ID ? "output" : "dump"),
			   (o_ts == TYING_NO_ID ? "dump" : "output"));
		}
	    }
	}
    }

    for (i = 0; i < o_mdef->n_tied_state; i++) {
	if (i_o2d[i] != n_o2d[i]) {
	    E_FATAL("%u != %u for %u\n",
		    i_o2d[i], n_o2d[i], i);
	}
    }

    for (i = 0; i < o_mdef->n_tied_state; i++) {
	i_o2d[i] = 0;
    }
    return S3_SUCCESS;
}

static uint32
setup_obs_1class(uint32 strm, uint32 n_frame, uint32 n_stream, uint32 *veclen, uint32 blksize)
{
    float32 *buf;
    vector_t *frm;
    uint32 i, l, o;
    uint32 n_sv_frame;
    uint32 ignore = 0;

    n_sv_frame = n_frame / stride;

    if (l_strm == strm) {
	E_INFO("No need to read data; using existing buffered data\n");
	
	return n_sv_frame;
    }

    n_tot_frame += n_sv_frame;

    l_strm = strm;

    E_INFO("alloc'ing %uMb obs buf\n",
	   n_sv_frame*veclen[strm]*sizeof(float32) / (1024 * 1024));

    if (obuf) {
	ckd_free(obuf);
	obuf = NULL;
    }
    obuf = ckd_calloc(n_sv_frame * veclen[strm], sizeof(float32));

    buf = (float32 *)ckd_calloc(blksize, sizeof(float32));
    frm = (vector_t *)ckd_calloc(n_stream, sizeof(float32 *));
    for (i = 0, l = 0; i < n_stream; i++) {
	frm[i] = &buf[l];
	l += veclen[i];
    }

    assert(l == blksize);
    assert(dmp_fp != NULL);

    if (fseek(dmp_fp, data_offset, SEEK_SET) < 0) {
	E_ERROR_SYSTEM("Can't seek to start of data\n");

	return 0;
    }

    for (i = 0, o = 0; i < n_frame; i++) {
	if (bio_fread(buf, sizeof(float32), blksize, dmp_fp, dmp_swp, &ignore) != blksize) {
	    E_ERROR_SYSTEM("Can't read dump file\n");

	    return 0;
	}
	    
	if ((i % stride) == 0) {
	    memcpy(&obuf[o],
		   (void *)&frm[strm][0],
		   sizeof(float32) * veclen[strm]);
	    o += veclen[strm];
	}
    }
    
    ckd_free(buf);
    ckd_free(frm);

    return n_sv_frame;
}

static uint32
setup_obs_multiclass(uint32 ts, uint32 strm, uint32 n_frame, uint32 veclen)
{
    uint32 i, o, k;
    uint32 n_i_frame;
    vector_t **feat;
    uint32 d_ts;
    uint32 n_sv_frame;

    n_sv_frame = n_frame / stride;

    if ((l_ts == ts) && (l_strm == strm)) {
	E_INFO("No need to read data; using existing buffered data\n");
	
	return n_sv_frame;
    }

    n_tot_frame += n_sv_frame;

    l_ts = ts;
    l_strm = strm;

    E_INFO("alloc'ing %uMb obs buf\n", n_sv_frame*veclen*sizeof(float32) / (1024 * 1024));

    if (obuf) {
	ckd_free(obuf);
	obuf = NULL;
    }
    obuf = ckd_calloc(n_sv_frame * veclen, sizeof(float32));

    if (stride == 1) {
	E_INFO("Reading all frames\n");
    }
    else {
	E_INFO("Reading 1 out of every %u frames from obs dmp file...\n", stride);
    }

    if (o2d) {
	E_INFO("o_ts == %u ->", ts);
	for (k = 0; k < n_o2d[ts]; k++) {
	    E_INFOCONT(" %d", o2d[ts][k]);
	}
	E_INFOCONT("\n");
	
	for (k = 0, o = 0; k < n_o2d[ts]; k++) {
	    d_ts = o2d[ts][k];

	    for (i = 0; segdmp_next_feat(d_ts, &feat, &n_i_frame); i++) {
		assert(n_i_frame == 1);

		if ((i % stride) == 0) {
		    memcpy(&obuf[o],
			   (void *)&feat[0][strm][0],
			   sizeof(float32) * veclen);
		    o += veclen;
		}
		ckd_free((void *)&feat[0][0][0]);
		ckd_free_2d((void **)feat);
	    }
	}
    }
    else {
	E_INFO("dmp mdef == output mdef\n");
	for (i = 0, o = 0; segdmp_next_feat(ts, &feat, &n_i_frame); i++) {
	    assert(n_i_frame == 1);

	    if ((i % stride) == 0) {
		memcpy(&obuf[o],
		       (void *)&feat[0][strm][0],
		       sizeof(float32) * veclen);
		o += veclen;
	    }
	    ckd_free((void *)&feat[0][0][0]);
	    ckd_free_2d((void **)feat);
	}
    }	

    if ((o / veclen) != n_sv_frame) {
	E_WARN("Expected %u frames, but read %u\n",
	       n_sv_frame, o / veclen);
    }

    E_INFO("done reading %u frames\n", n_sv_frame);

    return n_sv_frame;
}

static uint32 vlen;

uint32
setup_obs(uint32 ts, uint32 strm, uint32 n_frame, uint32 n_stream, uint32 *veclen, uint32 blksize)
{
    vlen = veclen[strm];
    if (multiclass) {
	return setup_obs_multiclass(ts, strm, n_frame, veclen[strm]);
    }
    else {
	return setup_obs_1class(strm, n_frame, n_stream, veclen, blksize);
    }
}

vector_t
get_obs(uint32 i)
{
    return &obuf[i*vlen];
}


int
main_initialize(int argc,
		char *argv[],
		lexicon_t **out_lex,
		model_def_t **out_omdef,
		model_def_t **out_dmdef,
		feat_t** out_feat)
{
    model_def_t *dmdef = NULL;
    model_def_t *omdef = NULL;
    lexicon_t *lex = NULL;
    feat_t *feat;
    const char *fn;
    uint32 n_ts;
    uint32 n_cb;
    const char *ts2cbfn;

    parse_cmd_ln(argc, argv);

    feat = 
        feat_init(cmd_ln_str("-feat"),
                  cmn_type_from_str(cmd_ln_str("-cmn")),
                  cmd_ln_boolean("-varnorm"),
                  agc_type_from_str(cmd_ln_str("-agc")),
                  1, cmd_ln_int32("-ceplen"));


    if (cmd_ln_str("-lda")) {
        E_INFO("Reading linear feature transformation from %s\n",
               cmd_ln_str("-lda"));
        if (feat_read_lda(feat,
                          cmd_ln_str("-lda"),
                          cmd_ln_int32("-ldadim")) < 0)
            return -1;
    }

    if (cmd_ln_str("-svspec")) {
        int32 **subvecs;
        E_INFO("Using subvector specification %s\n", 
               cmd_ln_str("-svspec"));
        if ((subvecs = parse_subvecs(cmd_ln_str("-svspec"))) == NULL)
            return -1;
        if ((feat_set_subvecs(feat, subvecs)) < 0)
            return -1;
    }

    if (cmd_ln_exists("-agcthresh")
        && 0 != strcmp(cmd_ln_str("-agc"), "none")) {
        agc_set_threshold(feat->agc_struct,
                          cmd_ln_float32("-agcthresh"));
    }

    if (feat->cmn_struct
        && cmd_ln_exists("-cmninit")) {
        char *c, *cc, *vallist;
        int32 nvals;

        vallist = ckd_salloc(cmd_ln_str("-cmninit"));
        c = vallist;
        nvals = 0;
        while (nvals < feat->cmn_struct->veclen
               && (cc = strchr(c, ',')) != NULL) {
            *cc = '\0';
            feat->cmn_struct->cmn_mean[nvals] = FLOAT2MFCC(atof(c));
            c = cc + 1;
            ++nvals;
        }
        if (nvals < feat->cmn_struct->veclen && *c != '\0') {
            feat->cmn_struct->cmn_mean[nvals] = FLOAT2MFCC(atof(c));
        }
        ckd_free(vallist);
    }
    *out_feat = feat;


    if (cmd_ln_str("-omoddeffn")) {
	E_INFO("Reading output model definitions: %s\n", cmd_ln_str("-omoddeffn"));
	
	/* Read in the model definitions.  Defines the set of
	   CI phones and context dependent phones.  Defines the
	   transition matrix tying and state level tying. */
	if (model_def_read(&omdef,
			   cmd_ln_str("-omoddeffn")) != S3_SUCCESS) {
	    return S3_ERROR;
	}

	if (cmd_ln_str("-dmoddeffn")) {
	    E_INFO("Reading dump model definitions: %s\n", cmd_ln_str("-dmoddeffn"));
	
	    if (model_def_read(&dmdef,
			       cmd_ln_str("-dmoddeffn")) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	    setup_d2o_map(dmdef, omdef);
	}
	else {
	    E_INFO("Assuming dump and output model definitions are identical\n");
	}

	ts2cbfn = cmd_ln_str("-ts2cbfn");
	if (ts2cbfn) {
	    if (strcmp(SEMI_LABEL, ts2cbfn) == 0) {
		omdef->cb = semi_ts2cb(omdef->n_tied_state);
		n_ts = omdef->n_tied_state;
		n_cb = 1;
	    }
	    else if (strcmp(CONT_LABEL, ts2cbfn) == 0) {
		omdef->cb = cont_ts2cb(omdef->n_tied_state);
		n_ts = omdef->n_tied_state;
		n_cb = omdef->n_tied_state;
	    }
	    else if (strcmp(PTM_LABEL, ts2cbfn) == 0) {
		omdef->cb = ptm_ts2cb(omdef);
		n_ts = omdef->n_tied_state;
		n_cb = omdef->acmod_set->n_ci;
	    }
	    else if (s3ts2cb_read(cmd_ln_str("-ts2cbfn"),
				  &omdef->cb,
				  &n_ts,
				  &n_cb) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	    
	    if (omdef->n_tied_state != n_ts) {
		E_FATAL("Model definition file n_tied_state = %u, but %u mappings in ts2cb\n",
			omdef->n_tied_state, n_ts);
	    }
	}
    }
    else {
	E_INFO("No mdef files.  Assuming 1-class init\n");
    }

    *out_omdef = omdef;
    *out_dmdef = dmdef;

    fn = cmd_ln_str("-dictfn");
    if (fn) {
	E_INFO("Reading main dictionary: %s\n", fn);
	     

	lex = lexicon_read(NULL,
			   fn,
			   omdef->acmod_set);
	if (lex == NULL)
	    return S3_ERROR;
    }
    
    fn = cmd_ln_str("-fdictfn");
    if (fn) {
	E_INFO("Reading filler dictionary: %s\n", fn);
	(void)lexicon_read(lex,
			   fn,
			   omdef->acmod_set);
    }
    
    *out_lex = lex;
    
    stride = cmd_ln_int32("-stride");

    return S3_SUCCESS;
}

#include <s3/kmeans.h>


static float32
random_kmeans(uint32 n_trial,
	      uint32 n_obs,
	      uint32 veclen,
	      vector_t *bst_mean,
	      uint32 n_mean,
	      float32 min_ratio,
	      uint32 max_iter,
	      codew_t **out_label)
{
    uint32 t, k, kk;
    float32 rr;
    uint32 cc;
    codew_t *label = NULL, *b_label = NULL;
    vector_t *tmp_mean;
    float64 sqerr, b_sqerr = MAX_POS_FLOAT64;
    vector_t c;
    uint32 n_aborts;

    tmp_mean = (vector_t *)ckd_calloc_2d(n_mean, veclen, sizeof(float32));

    E_INFO("Initializing means using random k-means\n");

    for (t = 0; t < n_trial; t++) {
	E_INFO("Trial %u: %u means\n", t, n_mean);

	n_aborts = 100;		/* # of aborts to allow */
	do {
	    /* pick a (pseudo-)random set of initial means from the corpus */
	    for (k = 0; k < n_mean; k++) {
		rr = drand48();	/* random numbers in the interval [0, 1) */
		cc = rr * n_obs;
		assert((cc >= 0) && (cc < n_obs));
		c = get_obs(cc);
		for (kk = 0; kk < veclen; kk++) {
		    tmp_mean[k][kk] = c[kk];
		}
	    }

	    if (n_mean > 1) {
		sqerr = k_means_trineq(tmp_mean, n_mean,
				       n_obs,
				       veclen,
				       min_ratio,
				       max_iter,
				       &label);
	    }
	    else {
		sqerr = k_means(tmp_mean, n_mean,
				n_obs,
				veclen,
				min_ratio,
				max_iter,
				&label);
	    }

	    if (sqerr < 0) {
		E_INFO("\t-> Aborting k-means, bad initialization\n");
		--n_aborts;
	    }
	} while ((sqerr < 0) && (n_aborts > 0));
	    

	if (sqerr < b_sqerr) {
	    b_sqerr = sqerr;

	    E_INFO("\tbest-so-far sqerr = %e\n", b_sqerr);
	    if (b_label)
		ckd_free(b_label);
	    b_label = label;
	    for (k = 0; k < n_mean; k++) {
		for (kk = 0; kk < veclen; kk++) {
		    bst_mean[k][kk] = tmp_mean[k][kk];
		}
	    }
	}
	else {
	    if (label) {
		ckd_free(label);
		label = NULL;
	    }
	}
    }

    *out_label = b_label;

    ckd_free_2d((void **)tmp_mean);

    return b_sqerr;
}

float64
find_farthest_neigh(uint32 *obs_subset,
		    uint32 n_obs_subset,
		    uint32 veclen,
		    vector_t neigh_a,
		    vector_t neigh_b)
{
    uint32 i, j;
    uint32 i_corp, j_corp;
    uint32 i_corp_max=0, j_corp_max=0;
    float64 d;
    float64 d_max = 0;
    float64 diff;
    uint32 l;
    vector_t c_i, c_j;

    d = 0;

    for (i = 0; i < n_obs_subset-1; i++) {
	for (j = i+1; j < n_obs_subset; j++) {
	    i_corp = obs_subset[i];
	    j_corp = obs_subset[j];

	    c_i = get_obs(i_corp);
	    c_j = get_obs(j_corp);
	    for (l = 0, d = 0; l < veclen; l++) {
		diff = c_i[l] - c_j[l];
		
		d += diff * diff;
	    }

	    if (d > d_max) {
		d_max = d;
		i_corp_max = i_corp;
		j_corp_max = j_corp;
	    }
	}
    }

    c_i = get_obs(i_corp_max);
    c_j = get_obs(j_corp_max);
    for (l = 0; l < veclen; l++) {
	neigh_a[l] = c_i[l];
	neigh_b[l] = c_j[l];
    }

    return d_max;
}

static uint32 **
cw_obs(codew_t *label,
       uint32 n_cw,
       uint32 n_obs,
       uint32 *occ_cnt)
{
    uint32 **ret;
    uint32 *free;
    uint32 k, i;

    ret = ckd_calloc(n_cw, sizeof(uint32 *));

    for (k = 0; k < n_cw; k++) {
	occ_cnt[k] = 0;
    }
    for (i = 0; i < n_obs; i++) {
	occ_cnt[label[i]]++;
    }

    for (k = 0; k < n_cw; k++) {
	ret[k] = ckd_calloc(occ_cnt[k], sizeof(uint32));
    }

    free = ckd_calloc(n_cw, sizeof(uint32));

    for (i = 0; i < n_obs; i++) {
	k = label[i];

	ret[k][free[k]++] = i;
    }

    for (k = 0; k < n_cw; k++) {
	assert(free[k] == occ_cnt[k]);
    }

    ckd_free((void *)free);

    return ret;
}

static float64
furthest_neighbor_kmeans(uint32 n_obs,
			 uint32 veclen,
			 vector_t *mean,
			 uint32 n_mean,
			 float32 min_ratio,
			 uint32 max_iter)
{
    uint32 **obs_of;
    uint32 *occ_cnt;
    codew_t *lbl;
    uint32 k_max=0, occ_max;
    uint32 n_mean_cur;
    vector_t *extr_mean;
    uint32 k, l;
    float64 sqerr;

    lbl = ckd_calloc(n_obs, sizeof(codew_t));
    occ_cnt = ckd_calloc(n_mean, sizeof(uint32));
    n_mean_cur = 1;

    extr_mean = (float32 **)ckd_calloc_2d(2, veclen, sizeof(float32));

    do {
	E_INFO("n_mean == %u\n", n_mean_cur);

	obs_of = cw_obs(lbl, n_mean_cur, n_obs, occ_cnt);

	occ_max = 0;

	for (k = 0; k < n_mean_cur; k++) {
	    if (occ_cnt[k] > occ_max) {
		occ_max = occ_cnt[k];
		k_max = k;
	    }
	}
	
	/* set the initial values of the new means by extreme means */

	E_INFO("d_max == %e\n",
	       find_farthest_neigh(obs_of[k_max], occ_cnt[k_max], veclen,
				   extr_mean[0], extr_mean[1]));
	
	
	sqerr = k_means_subset(extr_mean, 2,
			       obs_of[k_max], occ_cnt[k_max],
			       veclen,
			       min_ratio,
			       max_iter,
			       NULL);
	
	for (l = 0; l < veclen; l++) {
	    mean[k_max][l] = extr_mean[0][l];
	    mean[n_mean_cur][l] = extr_mean[1][l];
	}

	++n_mean_cur;

	ckd_free(lbl);

	sqerr = k_means(mean, n_mean_cur, n_obs,
			veclen,
			min_ratio,
			max_iter,
			&lbl);

	E_INFO("\tsquerr == %e\n", sqerr);
    } while (n_mean_cur < n_mean);

    return sqerr;
}

float64
cluster(int32 ts,
	uint32 n_stream,
	uint32 n_in_frame,
	uint32 *veclen,
	uint32 blksize,
	vector_t **mean,
	uint32 n_density,
	codew_t **out_label)
{
    float64 sum_sqerr, sqerr=0;
    uint32 s, n_frame;
    const char *meth;
    
    *out_label = NULL;

    k_means_set_get_obs(&get_obs);

    for (s = 0, sum_sqerr = 0; s < n_stream; s++, sum_sqerr += sqerr) {
	meth = cmd_ln_str("-method");

	n_frame = setup_obs(ts, s, n_in_frame, n_stream, veclen, blksize);

	if (strcmp(meth, "rkm") == 0) {
	    sqerr = random_kmeans(cmd_ln_int32("-ntrial"),
				  n_frame,
				  veclen[s],
				  mean[s],
				  n_density,
				  cmd_ln_float32("-minratio"),
				  cmd_ln_int32("-maxiter"),
				  out_label);
	    if (sqerr < 0) {
		E_ERROR("Too few observations for kmeans\n");
		
		return -1.0;
	    }
	}
	else if (strcmp(meth, "fnkm") == 0) {
	    sqerr = furthest_neighbor_kmeans(n_frame,
					     veclen[s],
					     mean[s],
					     n_density,
					     cmd_ln_float32("-minratio"),
					     cmd_ln_int32("-maxiter"));
	}
	else {
	    E_ERROR("I don't know how to do method '%s'.  Sorry.\n", meth);
	}
    }

    return sum_sqerr;
}

int
init_mixw(float32 **mixw,
	  vector_t **mean,
	  uint32 n_density,
	  const uint32 *veclen,
	  uint32 n_frame,
	  uint32 n_stream,
	  codew_t *label)
{
    uint32 s;
    uint32 *occ_cnt;
    uint32 i;

    occ_cnt = ckd_calloc(n_density, sizeof(uint32));

    for (s = 0; s < n_stream; s++) {
	for (i = 0; i < n_frame; i++) {
	    occ_cnt[label[i]]++;
	}

	for (i = 0; i < n_density; i++) {
	    mixw[s][i] = (float32)occ_cnt[i] / (float32)n_frame;
	}
    }
    return 0;
}

int
variances(uint32 ts,
	  vector_t **mean,
	  vector_t **var,
	  uint32 n_density,

	  uint32 n_stream,
	  uint32 *veclen,
	  uint32 blksize,
	  
	  uint32 n_in_frame,

	  codew_t *label)
{
    uint32 *n_obs;
    float64 term;
    uint32 s, i, l, k, n_frame;
    vector_t c;
    
    E_INFO("Initializing variances\n");
    for (s = 0; s < n_stream; s++) {
	n_obs = ckd_calloc(n_density, sizeof(uint32));

	n_frame = setup_obs(ts, s, n_in_frame, n_stream, veclen, blksize);

	for (i = 0; i < n_frame; i++) {
	    k = label[i];	/* best codeword */
	    n_obs[k]++;

	    c = get_obs(i);

	    for (l = 0; l < veclen[s]; l++) {
		term = c[l] - mean[s][k][l];
		term *= term;
		var[s][k][l] += term;
	    }
	}

	for (k = 0; k < n_density; k++) {
	    term = 1.0 / (float64)n_obs[k];
	    for (l = 0; l < veclen[s]; l++) {
		var[s][k][l] *= term;
	    }
	}

	ckd_free(n_obs);
    }
    return 0;
}

int
full_variances(uint32 ts,
	  vector_t **mean,
	  vector_t ***var,
	  uint32 n_density,

	  uint32 n_stream,
	  uint32 *veclen,
	  uint32 blksize,
	  
	  uint32 n_in_frame,

	  codew_t *label)
{
    uint32 *n_obs;
    float64 term;
    uint32 s, i, l, m, k, n_frame;
    vector_t c;
    
    E_INFO("Initializing full covariances\n");
    for (s = 0; s < n_stream; s++) {
	n_obs = ckd_calloc(n_density, sizeof(uint32));

	n_frame = setup_obs(ts, s, n_in_frame, n_stream, veclen, blksize);

	for (i = 0; i < n_frame; i++) {
	    k = label[i];	/* best codeword */
	    n_obs[k]++;

	    c = get_obs(i);

	    for (l = 0; l < veclen[s]; l++) {
		for (m = 0; m < veclen[s]; m++) {
		    var[s][k][l][m] +=
			(c[l] - mean[s][k][l])
			* (c[m] - mean[s][k][m]);
		}
	    }
	}

	for (k = 0; k < n_density; k++) {
	    term = 1.0 / (float64)n_obs[k];
	    for (l = 0; l < veclen[s]; l++) {
		for (m = 0; m < veclen[s]; m++) {
		    var[s][k][l][m] *= term;
		}
	    }
	}

	ckd_free(n_obs);
    }
    return 0;
}

static vector_t **
alloc_gau_acc(uint32 n_stream,
	      uint32 n_density,
	      const uint32 *veclen,
	      uint32 blksize)
{
    vector_t **param;
    float32 *buf;
    uint32 j, k, l;

    param = (vector_t **) ckd_calloc_2d(n_stream, n_density,
					sizeof(vector_t));
    
    buf = ckd_calloc(n_density * blksize, sizeof(float32));

    for (j = 0, l = 0; j < n_stream; j++) {
	for (k = 0; k < n_density; k++) {
	    param[j][k] = &buf[l];

	    l += veclen[j];
	}
    }

    return param;
}

float64
reest_sum(uint32 ts,
	  vector_t **mean,
	  vector_t **var,
	  float32 **mixw,
	  uint32 n_density,
	  uint32 n_stream,
	  uint32 n_in_obs,
	  uint32 *veclen,
	  uint32 blksize,
	  uint32 n_iter,
	  uint32 twopassvar,
	  uint32 vartiethr)
{
    uint32 o, i, j, k, l;
    float32  *mixw_acc;
    float32  *cb_acc;
    vector_t  **mean_acc_xx;
    vector_t  **var_acc_xx;
    vector_t  *mean_acc;
    vector_t  *var_acc;
    float64 ol, ttt, diff, log_tot_ol = 0, p_log_tot_ol = 0;
    float64 **norm, *den;
    float64 log_a_den=0;
    float32 mixw_norm;

    vector_t obs;
    uint32 n_obs;

    vector_t ***n_mean_xx = NULL;
    vector_t *n_mean = NULL;

    float64 avg_lik=0, p_avg_lik=0;
    uint32 tievar = FALSE;

    E_INFO("EM reestimation of mixw/means/vars\n");
    
    if (twopassvar) {
	n_mean_xx = gauden_alloc_param(1, 1, n_density, veclen);
	n_mean = n_mean_xx[0][0];
    }
    
    /* allocate mixing weight accumulators */
    mixw_acc = (float32 *)ckd_calloc(n_density, sizeof(float32));

    cb_acc = (float32 *)ckd_calloc(n_density, sizeof(float32));
    mean_acc_xx = (vector_t **)alloc_gau_acc(1, n_density, veclen, blksize);
    mean_acc = mean_acc_xx[0];
    var_acc_xx = (vector_t **)alloc_gau_acc(1, n_density, veclen, blksize);
    var_acc = var_acc_xx[0];

    den = (float64 *)ckd_calloc(n_density, sizeof(float64));
    norm = (float64 **)ckd_calloc_2d(n_stream, n_density, sizeof(float64));

    for (j = 0; j < n_stream; j++) {
	n_obs = setup_obs(ts, j, n_in_obs, n_stream, veclen, blksize);

	if (n_obs < vartiethr) tievar = TRUE;

	for (i = 0; i < n_iter; i++) {
	    p_log_tot_ol = log_tot_ol;
	    log_tot_ol = 0;

	    for (k = 0; k < n_density; k++) {
		/* floor variances */
		for (l = 0; l < veclen[j]; l++)
		    if (var[j][k][l] < 1e-4) var[j][k][l] = 1e-4; 

		/* compute normalization factors for Gaussian
		   densities */
		norm[j][k] = diag_norm(var[j][k], veclen[j]);

		/* precompute 1/(2sigma^2) terms */
		diag_eval_precomp(var[j][k], veclen[j]);
	    }

	    if (twopassvar) {
		/* do a pass over the corpus to compute reestimated means */
		for (o = 0; o < n_obs; o++) {
		    float64 mx;

		    obs = get_obs(o);
		
		    mx = MIN_NEG_FLOAT64;

		    for (k = 0; k < n_density; k++) {
			/* really log(den) for the moment */
			den[k] = log_diag_eval(obs, norm[j][k], mean[j][k], var[j][k], veclen[j]);
			if (mx < den[k]) mx = den[k];
		    }
		    for (k = 0, ol = 0; k < n_density; k++) {
			den[k] = exp(log_a_den - mx);
			ol += mixw[j][k] * den[k];
		    }

		    for (k = 0; k < n_density; k++) {
			ttt = mixw[j][k] * den[k] / ol;
		    
			cb_acc[k] += ttt;
			for (l = 0; l < veclen[j]; l++) {
			    mean_acc[k][l] += obs[l] * ttt;
			}
		    }
		}
	    
		cb_acc[0] = 1.0 / cb_acc[0];
		for (k = 1; k < n_density; k++) {
		    cb_acc[k] = 1.0 / cb_acc[k];
		}

		/* compute the reestimated mean value to be used in next pass */
		for (k = 0; k < n_density; k++) {
		    for (l = 0; l < veclen[j]; l++) {
			n_mean[k][l] = mean_acc[k][l] * cb_acc[k];
			mean_acc[k][l] = 0;
		    }
		    cb_acc[k] = 0;
		}
	    }
	    else {
		n_mean = mean[j];
	    }
		
	    for (o = 0; o < n_obs; o++) {
		float64 mx;

		/* Do a pass over the data to accumulate reestimation sums
		 * for the remaining parameters (including means
		 * if not a 2-pass config) */

		/* Get the next observation */
		obs = get_obs(o);

		mx = MIN_NEG_FLOAT64;

		/* Compute the mixture density value given the
		 * observation and the model parameters */
		for (k = 0; k < n_density; k++) {
		    /* really log(den) for the moment */
		    den[k] = log_diag_eval(obs, norm[j][k], mean[j][k], var[j][k], veclen[j]);
		    if (mx < den[k]) mx = den[k];
		}
		for (k = 0, ol = 0; k < n_density; k++) {
		    den[k] = exp(den[k] - mx);
		    ol += mixw[j][k] * den[k];
		}

		log_tot_ol += log(ol) + mx;

		/* Compute the reestimation sum terms for each
		 * of the component densities */
		for (k = 0; k < n_density; k++) {
		    ttt = mixw[j][k] * den[k] / ol;

		    mixw_acc[k] += ttt;

		    cb_acc[k] += ttt;
		    
		    for (l = 0; l < veclen[j]; l++) {
			/* if not doing two-pass variance computation
			 * n_mean <- mean above. */
			diff = obs[l] - n_mean[k][l];
			
			if (!twopassvar) {
			    mean_acc[k][l] += ttt * obs[l];
			}

			var_acc[k][l] += ttt * diff * diff;
		    }
		}
	    }

	    avg_lik = exp(log_tot_ol / n_obs);

	    if (p_log_tot_ol != 0)
		p_avg_lik = exp(p_log_tot_ol / n_obs);
	    else
		p_avg_lik = 0.5 * avg_lik;

	    E_INFO("EM stream %u: [%u] avg_lik %e conv_ratio %e\n",
		   j, i, avg_lik, (avg_lik - p_avg_lik) / p_avg_lik);
	    
	    /* normalize after iteration */

	    if (tievar) {
		/* create a sum over all densities in entry 0 */
		for (k = 1; k < n_density; k++) {
		    for (l = 0; l < veclen[j]; l++) {
			var[j][0][l] += var[j][k][l];
		    }
		    cb_acc[0] += cb_acc[k];
		}
		/* copy entry 0 back to remaining entries */
		for (k = 1; k < n_density; k++) {
		    for (l = 0; l < veclen[j]; l++) {
			var[j][k][l] = var[j][0][l];
		    }
		    cb_acc[k] = cb_acc[0];
		}		
	    }
		
	    for (k = 0, mixw_norm = 0; k < n_density; k++) {
		/* norm for per density expectations */
		cb_acc[k] = 1.0 / cb_acc[k];

		mixw_norm += mixw_acc[k];
	    }
	    mixw_norm = 1.0 / mixw_norm;

	    if (!twopassvar) {
		for (k = 0; k < n_density; k++) {
		    mixw[j][k] = mixw_acc[k] * mixw_norm;
		    mixw_acc[k] = 0;

		    for (l = 0; l < veclen[j]; l++) {
			mean[j][k][l] = mean_acc[k][l] * cb_acc[k];
			mean_acc[k][l] = 0;
			var[j][k][l]  = var_acc[k][l] * cb_acc[k];
			var_acc[k][l] = 0;
		    }
		    cb_acc[k] = 0;
		}
	    }
	    else {
		for (k = 0; k < n_density; k++) {
		    mixw[j][k] = mixw_acc[k] * mixw_norm;
		    mixw_acc[k] = 0;

		    for (l = 0; l < veclen[j]; l++) {
			/* already computed in first pass */
			mean[j][k][l] = n_mean[k][l];
			var[j][k][l]  = var_acc[k][l] * cb_acc[k];
			var_acc[k][l] = 0;
		    }
		    cb_acc[k] = 0;
		}
	    }
	}	/* end of EM iteration loop */

	E_INFO("EM stream %u: [final] n_obs %u avg_lik %e conv_ratio %e\n",
	       j, n_obs, avg_lik, (avg_lik - p_avg_lik) / p_avg_lik);

    }	/* end of feature stream loop */

    ckd_free((void *)mixw_acc);
    ckd_free((void *)cb_acc);
    ckd_free((void *)&mean_acc_xx[0][0][0]);
    ckd_free_2d((void **)mean_acc_xx);
    ckd_free((void *)&var_acc_xx[0][0][0]);
    ckd_free_2d((void **)var_acc_xx);
    if (n_mean_xx) {
	ckd_free((void *)&n_mean_xx[0][0][0]);
	ckd_free_2d((void **)n_mean);
    }

    ckd_free_2d((void **)norm);
    ckd_free((void *)den);

    return log_tot_ol;
}

static int
init_state(const char *obsdmp,
	   const char *obsidx,
	   uint32 n_density,
	   uint32 n_stream,
	   uint32 *veclen,
	   uint32 blksize,
	   int reest,
	   const char *mixwfn,
	   const char *meanfn,
	   const char *varfn,
	   uint32 ts_off,
	   uint32 ts_cnt,
	   uint32 n_ts,
	   uint32 n_d_ts)
{
    vector_t ***mean;
    vector_t ***var = NULL;
    vector_t ****fullvar = NULL;
    float32  ***mixw = NULL;
    uint32 n_frame;
    uint32 ignore = 0;
    codew_t *label;
    uint32 n_corpus = 0;
    float64 sqerr;
    float64 tot_sqerr;
    segdmp_type_t t;
    uint32 i, j, ts, n;
    int32 full_covar;

    full_covar = cmd_ln_int32("-fullvar");
    /* fully-continuous for now */
    mean = gauden_alloc_param(ts_cnt, n_stream, n_density, veclen);
    if (full_covar)
	    fullvar  = gauden_alloc_param_full(ts_cnt, n_stream, n_density, veclen);
    else
	    var  = gauden_alloc_param(ts_cnt, n_stream, n_density, veclen);
    if (mixwfn)
	mixw = (float32 ***)ckd_calloc_3d(ts_cnt,
					  n_stream,
					  n_density,
					  sizeof(float32));

    if (cmd_ln_str("-segidxfn")) {
	E_INFO("Multi-class dump\n");
	if (segdmp_open_read(cmd_ln_str_list("-segdmpdirs"),
			     cmd_ln_str("-segdmpfn"),
			     cmd_ln_str("-segidxfn"),
			     &n,
			     &t, 
			     n_stream, veclen, blksize) != S3_SUCCESS) {
	    E_FATAL("Unable to open dumps\n");
	}

	if (n != n_d_ts) {
	    E_FATAL("Expected %u tied-states in dump, but apparently %u\n",
		    n_d_ts, n);
	}
	if (t != SEGDMP_TYPE_FEAT) {
	    E_FATAL("Expected feature dump, but instead saw %u\n", t);
	}
	
	multiclass = TRUE;
    }
    else {
	E_INFO("1-class dump file\n");
	
	multiclass = FALSE;
	
	dmp_fp = s3open(cmd_ln_str("-segdmpfn"), "rb",
			&dmp_swp);
	if (dmp_fp == NULL) {
	    E_ERROR_SYSTEM("Unable to open dump file %s for reading\n",
			   cmd_ln_str("-segdmpfn"));

	    return S3_ERROR;
	}

	if (bio_fread(&n_frame, sizeof(uint32), 1, dmp_fp, dmp_swp, &ignore) != 1) {
	    E_ERROR_SYSTEM("Unable to open dump file %s for reading\n",
			   cmd_ln_str("-segdmpfn"));

	    return S3_ERROR;
	}

	data_offset = ftell(dmp_fp);
    }

    tot_sqerr = 0;
    for (i = 0; i < ts_cnt; i++) {
	ts = ts_off + i;

	/* stride not accounted for yet */
	if (o2d == NULL) {
	    if (multiclass)
		n_frame = segdmp_n_seg(ts);
	}
	else {
	    for (j = 0, n_frame = 0; j < n_o2d[ts]; j++) {
		n_frame += segdmp_n_seg(o2d[ts][j]);
	    }
	}
    
	E_INFO("Corpus %u: sz==%u frames%s\n",
	       ts, n_frame,
	       (n_frame > cmd_ln_int32("-vartiethr") ? "" : " tied var"));

	if (n_frame == 0) {
	    continue;
	}


	E_INFO("Convergence ratios are abs(cur - prior) / abs(prior)\n");
	/* Do some variety of k-means clustering */
	ptmr_start(&km_timer);
	sqerr = cluster(ts, n_stream, n_frame, veclen, blksize, mean[i], n_density, &label);
	ptmr_stop(&km_timer);

	if (sqerr < 0) {
	    E_ERROR("Unable to do k-means for state %u; skipping...\n", ts);

	    continue;
	}

	/* Given the k-means and assuming equal prior liklihoods
	 * compute the variances */
	ptmr_start(&var_timer);
	if (full_covar)
		full_variances(ts, mean[i], fullvar[i], n_density, n_stream, veclen, blksize,
			       n_frame, label);
	else
		variances(ts, mean[i], var[i], n_density, n_stream, veclen, blksize, n_frame, label);
	ptmr_stop(&var_timer);

	if (mixwfn) {
	    /* initialize the mixing weights by counting # of occurrances
	     * of the top codeword over the corpus and normalizing */
	    init_mixw(mixw[i], mean[i], n_density, veclen, n_frame, n_stream, label);

	    ckd_free(label);

	    if (reest == TRUE && full_covar)
		E_ERROR("EM re-estimation is not yet supported for full covariances\n");
	    else if (reest == TRUE) {
		ptmr_start(&em_timer);
		/* Do iterations of EM to estimate the mixture densities */
		reest_sum(ts, mean[i], var[i], mixw[i], n_density, n_stream,
			  n_frame, veclen, blksize,
			  cmd_ln_int32("-niter"),
			  FALSE,
			  cmd_ln_int32("-vartiethr"));
		ptmr_stop(&em_timer);
	    }
	}
	
	++n_corpus;
	tot_sqerr += sqerr;
	    
	E_INFO("sqerr [%u] == %e\n", ts, sqerr);
    }

    if (n_corpus > 0) {
	E_INFO("sqerr = %e tot %e rms\n", tot_sqerr, sqrt(tot_sqerr/n_corpus));
    }

    if (!multiclass)
	s3close(dmp_fp);
    
    if (meanfn) {
	if (s3gau_write(meanfn,
			(const vector_t ***)mean,
			ts_cnt,
			n_stream,
			n_density,
			veclen) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }
    else {
	E_INFO("No mean file given; none written\n");
    }
		    
    if (varfn) {
	if (full_covar) {
	    if (s3gau_write_full(varfn,
				 (const vector_t ****)fullvar,
				 ts_cnt,
				 n_stream,
				 n_density,
				 veclen) != S3_SUCCESS)
		return S3_ERROR;
	}
	else {
	    if (s3gau_write(varfn,
				 (const vector_t ***)var,
				 ts_cnt,
				 n_stream,
				 n_density,
				 veclen) != S3_SUCCESS)
		return S3_ERROR;
	}
    }
    else {
	E_INFO("No variance file given; none written\n");
    }

    if (mixwfn) {
	if (s3mixw_write(mixwfn,
			 mixw,
			 ts_cnt,
			 n_stream,
			 n_density) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }
    else {
	E_INFO("No mixing weight file given; none written\n");
    }

    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    lexicon_t *lex;
    model_def_t *omdef;
    model_def_t *dmdef;
    feat_t *feat;
    uint32 n_stream, blksize;
    uint32 *veclen;
    uint32 ts_off;
    uint32 ts_cnt;
    FILE *fp;

    if (main_initialize(argc, argv, &lex, &omdef, &dmdef, &feat) != S3_SUCCESS) {
	return -1;
    }

    n_stream = feat_dimension1(feat);
    veclen = feat_stream_lengths(feat);
    blksize = feat_dimension(feat);

    if (strcmp(cmd_ln_str("-gthobj"), "state") == 0) {
	ts_off = cmd_ln_int32("-tsoff");

	if (cmd_ln_str("-tscnt") == NULL) {
	    ts_cnt = omdef->n_tied_state - ts_off;
 	}
	else {
	    ts_cnt = cmd_ln_int32("-tscnt");
	}

	if (ts_off + ts_cnt > omdef->n_tied_state) {
	    E_FATAL("Too many tied states specified\n");
	}

	n_tot_frame = 0;

	ptmr_reset(&all_timer);
	ptmr_reset(&km_timer);
	ptmr_reset(&var_timer);
	ptmr_reset(&em_timer);
	ptmr_start(&all_timer);

	if (init_state(cmd_ln_str("-segdmpfn"),
		       cmd_ln_str("-segidxfn"),
		       cmd_ln_int32("-ndensity"),
		       n_stream,
		       veclen,
		       blksize,
		       cmd_ln_int32("-reest"),
		       cmd_ln_str("-mixwfn"),
		       cmd_ln_str("-meanfn"),
		       cmd_ln_str("-varfn"),
		       ts_off,
		       ts_cnt,
		       omdef->n_tied_state,
		       (dmdef != NULL ? dmdef->n_tied_state : omdef->n_tied_state))
		       != S3_SUCCESS) {
	    E_ERROR("Unable to train [%u %u]\n", ts_off, ts_off+ts_cnt-1);
	}
	ptmr_stop(&all_timer);

	if (n_tot_frame > 0) {
	    E_INFO("TOTALS:");
    	    E_INFOCONT(" km %4.3fx %4.3e", 
	    	km_timer.t_cpu / (n_tot_frame * 0.01),
		(km_timer.t_cpu > 0 ?
		 km_timer.t_elapsed / km_timer.t_cpu : 0.0));
    	    E_INFOCONT(" var %4.3fx %4.3e", 
		var_timer.t_cpu / (n_tot_frame * 0.01),
		(var_timer.t_cpu > 0 ?
		 var_timer.t_elapsed / var_timer.t_cpu : 0.0));
	    E_INFOCONT(" em %4.3fx %4.3e", 
		em_timer.t_cpu / (n_tot_frame * 0.01),
		(em_timer.t_cpu > 0 ?
		 em_timer.t_elapsed / em_timer.t_cpu : 0.0));
    	    E_INFOCONT(" all %4.3fx %4.3e", 
	    	all_timer.t_cpu / (n_tot_frame * 0.01),
		(all_timer.t_cpu > 0 ?
		 all_timer.t_elapsed / all_timer.t_cpu : 0.0));
	    E_INFOCONT("\n");
	}
	
	if (cmd_ln_str("-tsrngfn") != NULL) {
	    fp = fopen(cmd_ln_str("-tsrngfn"),
		       "w");
	    if (fp == NULL) {
		E_FATAL_SYSTEM("Unable to open %s for reading",
			       cmd_ln_str("-tsrngfn"));
	    }
	    
	    fprintf(fp, "%d %d\n", ts_off, ts_cnt);
	}
	else if (ts_cnt != omdef->n_tied_state) {
	    E_WARN("Subset of tied states specified, but no -tsrngfn arg");
	}
    }
    else if (strcmp(cmd_ln_str("-gthobj"), "single") == 0) {
	n_tot_frame = 0;

	ptmr_reset(&all_timer);
	ptmr_reset(&km_timer);
	ptmr_reset(&var_timer);
	ptmr_reset(&em_timer);

	ptmr_start(&all_timer);
	
	if (init_state(cmd_ln_str("-segdmpfn"),
		       NULL,	/* No index -> single class dump file */
		       cmd_ln_int32("-ndensity"),
		       n_stream,
		       veclen,
		       blksize,
		       cmd_ln_int32("-reest"),
		       cmd_ln_str("-mixwfn"),
		       cmd_ln_str("-meanfn"),
		       cmd_ln_str("-varfn"),
		       0,
		       1,
		       1,
		       1) != S3_SUCCESS) {
	    E_ERROR("Unable to train\n");
	}
	ptmr_stop(&all_timer);

	if (n_tot_frame > 0) {
	    E_INFO("TOTALS:");
    	    E_INFOCONT(" km %4.3fx %4.3e", 
		km_timer.t_cpu / (n_tot_frame * 0.01),
		(km_timer.t_cpu > 0 ?
		 km_timer.t_elapsed / km_timer.t_cpu : 0.0));
	    E_INFOCONT(" var %4.3fx %4.3e", 
		var_timer.t_cpu / (n_tot_frame * 0.01),
		(var_timer.t_cpu > 0 ?
		 var_timer.t_elapsed / var_timer.t_cpu : 0.0));
	    E_INFOCONT(" em %4.3fx %4.3e", 
		em_timer.t_cpu / (n_tot_frame * 0.01),
		(em_timer.t_cpu > 0 ?
		 em_timer.t_elapsed / em_timer.t_cpu : 0.0));
	    E_INFOCONT(" all %4.3fx %4.3e", 
    		all_timer.t_cpu / (n_tot_frame * 0.01),
		(all_timer.t_cpu > 0 ?
		 all_timer.t_elapsed / all_timer.t_cpu : 0.0));
	    E_INFOCONT("\n");
	}
    }

    return 0;
}

