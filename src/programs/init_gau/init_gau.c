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
 * File: init_gau.c
 * 
 * Description: 
 * 	Initialize one Gaussian mixtures for each context independent
 *	phone state.
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include "init_gau.h"
#include "accum.h"

#include <s3/mk_sseq.h>
#include <s3/ck_seg.h>
#include <s3/corpus.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>
#include <s3/gauden.h>
#include <s3/s3gau_io.h>
#include <s3/vector.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <sphinxbase/prim_type.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/feat.h>


int
init_gau(lexicon_t *lex,
	 model_def_t *mdef,
	 feat_t *feat)
{
    char *trans = NULL;
    char *fn;
    acmod_set_t *acmod_set;

    vector_t *mfcc = NULL;
    int32 n_frame;
    int32 feat_n_frame;
    int32 tmp;
    
    uint16 *seg = NULL;
    uint32 *sseq = NULL;
    uint32 *ci_sseq = NULL;

    uint32 tick_cnt = 0;

    char **word = NULL;
    uint32 n_word;

    acmod_id_t *phone = NULL;
    uint32 n_phone;
    char *btw_mark = NULL;

    vector_t ***mean_acc = NULL;
    vector_t ***mean = NULL;
    vector_t ***var_acc = NULL;
    vector_t ****fullvar_acc = NULL;
    float32 ***dnom = NULL;

    const uint32 *veclen;

    uint32 n_ts;

    uint32 *r_veclen;
    uint32 r_n_ts;
    uint32 r_n_feat;
    uint32 r_n_density;

    mfcc_t ***f = NULL;

    const char *meanfn;

    uint32 ceplen = cmd_ln_int32("-ceplen");
    int32 var_is_full = cmd_ln_int32("-fullvar");

    if (mdef) {
	acmod_set = mdef->acmod_set;
	n_ts = mdef->n_tied_state;
    }
    else {
	acmod_set = NULL;
	n_ts = 1;		/* Global mean/var */
    }

    meanfn = cmd_ln_str("-meanfn");

    veclen = (uint32 *)feat_stream_lengths(feat);
    
    if (meanfn == NULL) {
	E_INFO("Computing %ux%ux1 mean estimates\n", n_ts, feat_dimension1(feat));
    
	mean_acc = gauden_alloc_param(n_ts,
				      feat_dimension1(feat),
				      1,
				      veclen);
	var_acc = NULL;
    }
    else {
	assert(meanfn != NULL);

	E_INFO("Computing %ux%ux1 variance estimates\n", n_ts, feat_dimension1(feat));


	if (s3gau_read(meanfn,
		       &mean,
		       &r_n_ts,
		       &r_n_feat,
		       &r_n_density,
		       &r_veclen) != S3_SUCCESS) {
	    E_FATAL_SYSTEM("Unable to open %s for reading\n", meanfn);
	}
	ckd_free(r_veclen);

	mean_acc = NULL;
	if (var_is_full)
		fullvar_acc = gauden_alloc_param_full(n_ts,
						      feat_dimension1(feat),
						      1,
						      veclen);
	else
		var_acc =  gauden_alloc_param(n_ts,
					      feat_dimension1(feat),
					      1,
					      veclen);
    }

    dnom = (float32 ***)ckd_calloc_3d(n_ts, feat_dimension1(feat), 1, sizeof(float32));

    while (corpus_next_utt()) {
	if (mfcc) {
	    free(mfcc[0]);
	    ckd_free(mfcc);
	    mfcc = NULL;
	}
	if (trans) {
	    free(trans);
	    trans = NULL;
	}
	if (seg) {
	    free(seg);
	    seg = NULL;
	}
	if (word) {
	    ckd_free(word);
	    word = NULL;
	}
	if (phone) {
	    ckd_free(phone);
	    phone = NULL;
	}
	if (btw_mark) {
	    ckd_free(btw_mark);
	    btw_mark = NULL;
	}
	if (ci_sseq) {
	    ckd_free(ci_sseq);
	    ci_sseq = NULL;
	}
	if (sseq) {
	    ckd_free(sseq);
	    sseq = NULL;
	}
	if (f) {
	    feat_array_free(f);
	    f = NULL;
	}
	
	if ((++tick_cnt % 100) == 0) {
	    printf("[%u] ", tick_cnt);
	    fflush(stdout);
	}

	if (mdef) {
	    if (corpus_get_sent(&trans) != S3_SUCCESS) {
		E_FATAL("Unable to read word transcript for %s\n", corpus_utt_brief_name());
	    }

	    if (corpus_get_seg(&seg, &n_frame) != S3_SUCCESS) {
		E_FATAL("Unable to read Viterbi state segmentation for %s\n",
			corpus_utt_brief_name());
	    }
	    
    	    n_word = str2words(trans, NULL, 0);
    	    word = ckd_calloc(n_word, sizeof(char*));
	    str2words(trans, word, n_word);
	    phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);

	    /* check to see whether the word transcript and dictionary entries
	       agree with the state segmentation */
	    if (ck_seg(acmod_set, phone, n_phone, seg, n_frame, corpus_utt()) != S3_SUCCESS) {
		continue;
	    }

	    /* make a tied state id sequence from the state segmentation and the
	       phone list */
	    ci_sseq = mk_sseq(seg, n_frame, phone, n_phone, mdef);

	    if (cvt2triphone(acmod_set, phone, btw_mark, n_phone) != S3_SUCCESS) {
		continue;
	    }

	    /* make a tied state id sequence from the state segmentation and the
	       phone list */
	    sseq = mk_sseq(seg, n_frame, phone, n_phone, mdef);
	}

        if (corpus_get_generic_featurevec(&mfcc, &tmp, ceplen) < 0) {
	        E_FATAL("Can't read input features\n");
	}

	if (mdef == NULL) n_frame = tmp;

	if (tmp != n_frame) {
	    E_FATAL("# frames in MFCC file inconsistent w/ seg file for utt %s.\n",
		    corpus_utt_brief_name());
	}

	feat_n_frame = n_frame;

	/* FIXME: This number is bogus, it ought to depend on the type of feature used. */
	if (n_frame < 9) {
	    E_WARN("utt %s too short\n", corpus_utt());
	    if (mfcc) {
		ckd_free(mfcc[0]);
		ckd_free(mfcc);
		mfcc = NULL;
	    }
	    continue;
	}
	
	f = feat_array_alloc(feat, feat_n_frame + feat_window_size(feat));
	feat_s2mfc2feat_live(feat, mfcc, &feat_n_frame, TRUE, TRUE, f);

	if (feat_n_frame != n_frame) {
	    E_FATAL("# frames compute != # frames of state seg\n");
	}

	if (mean_acc) {
	    /* accumulate mean sums since no estimate given */
	    accum_state_mean(mean_acc, dnom, f, feat_dimension1(feat), veclen, sseq, ci_sseq, n_frame);
	}
	else if (var_acc) {
	    /* accumulate var sums since mean estimate exists */
	    accum_state_var(var_acc, mean, dnom, f, feat_dimension1(feat), veclen, sseq, ci_sseq, n_frame);
	}
	else if (fullvar_acc) {
	    /* accumulate var sums since mean estimate exists */
	    accum_state_fullvar(fullvar_acc, mean, dnom, f, feat_dimension1(feat), veclen, sseq, ci_sseq, n_frame);
	}
    }
    
    fn = ckd_calloc(strlen(cmd_ln_str("-accumdir")) + strlen("/gauden_counts") + 1, 1);
    sprintf(fn, "%s/gauden_counts", cmd_ln_str("-accumdir"));
    
    if (var_is_full) {
	if (s3gaucnt_write_full(fn, mean_acc, fullvar_acc, TRUE /* 2-pass variance */, dnom,
				n_ts, feat_dimension1(feat), 1, veclen) != 0) {
	}
    }
    else {
	if (s3gaucnt_write(fn, mean_acc, var_acc, TRUE /* 2-pass variance */, dnom,
			   n_ts, feat_dimension1(feat), 1, veclen) != 0) {
	}
    }

    /* free the per utterance data structures from the last utt */
    if (mfcc) {
	free(mfcc[0]);
	ckd_free(mfcc);
	mfcc = NULL;
    }
    if (trans) {
	free(trans);
	trans = NULL;
    }
    if (seg) {
	free(seg);
	seg = NULL;
    }
    if (word) {
	ckd_free(word);
	word = NULL;
    }
    if (phone) {
	ckd_free(phone);
	phone = NULL;
    }
    if (btw_mark) {
	ckd_free(btw_mark);
	btw_mark = NULL;
    }
    if (ci_sseq) {
	ckd_free(ci_sseq);
	ci_sseq = NULL;
    }
    if (sseq) {
	ckd_free(sseq);
	sseq = NULL;
    }
    if (f) {
	feat_array_free(f);
	f = NULL;
    }

    if (mean_acc) {
	gauden_free_param(mean_acc);
    }
    if (var_acc) {
	gauden_free_param(var_acc);
    }
    if (fullvar_acc) {
	gauden_free_param_full(fullvar_acc);
    }
    ckd_free_3d((void ***)dnom);
    ckd_free(fn);

    return S3_SUCCESS;
}

