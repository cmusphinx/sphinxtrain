/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: agg_st_seg.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "agg_st_seg.h"

#include <s3/lexicon.h>
#include <s3/corpus.h>
#include <s3/segdmp.h>
#include <s3/mllr.h>
#include <s3/ck_seg.h>
#include <s3/mk_sseq.h>
#include <s3/mk_phone_seq.h>

#include <sphinxbase/matrix.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/feat.h>

static void
xfrm_feat(float32 ***a,
	  float32 **b,
	  float32 **f,
	  uint32 n_stream,
	  const uint32 *veclen)
{
    uint32 s, i, j;
    float32 *o;

    for (s = 0; s < n_stream; s++) {
	for (i = 0; i < veclen[s]; i++) {
	    f[s][i] -= b[s][i];
	}

	o = ckd_calloc(veclen[s], sizeof(float32));

	for (i = 0; i < veclen[s]; i++) {
	    for (j = 0; j < veclen[s]; j++) {
		o[i] += a[s][i][j] * f[s][j];
	    }
	}

	for (j = 0; j < veclen[s]; j++) {
	    f[s][j] = o[j];
	}

	ckd_free((void *)o);
    }
}


static uint32 *
get_sseq(model_def_t *mdef,
	 lexicon_t *lex,
	 uint32 n_frame_in)
{
    char *trans;
    uint16 *seg;
    int32 n_frame;
    acmod_id_t *phone;
    uint32 n_phone;
    uint32 *sseq;
    
    corpus_get_sent(&trans);
    corpus_get_seg(&seg, &n_frame);

    if (n_frame_in != n_frame) {
	E_WARN("# frames in feature stream, %u != # frames in seg file %u; skipping\n",
	       n_frame_in, n_frame);
	    
	ckd_free(seg);
	ckd_free(trans);

	return NULL;
    }

    mk_phone_seq(&phone, &n_phone, trans, mdef->acmod_set, lex);

    /*    ck_seg(mdef->acmod_set, phone, n_phone, seg, n_frame, corpus_utt()); */

    sseq = mk_sseq(seg, n_frame, phone, n_phone, mdef);
    
    ckd_free(phone);
    ckd_free(seg);
    ckd_free(trans);

    return sseq;
}


int
agg_st_seg(model_def_t *mdef,
	   lexicon_t *lex,
	   feat_t* fcb,
	   uint32 *ts2cb,
	   int32 *cb2mllr,
	   segdmp_type_t type)
{
    uint32 seq_no;
    vector_t *mfcc = NULL;
    int32 n_frame;
    vector_t **feat = NULL;
    uint32 *sseq = NULL;
    uint32 i, j;
    uint32 t;
    uint32 n_stream=0;
    uint32 n_stream_tmp;
    uint32 *veclen = NULL;
    uint32 *veclen_tmp;
    uint32 n_mllr_cls;
    float32 ****a = NULL;
    float32 ***b = NULL;
    uint32 mcls;
    int32 mfc_veclen = cmd_ln_int32("-ceplen");

    n_stream = feat_dimension1(fcb);
    veclen = feat_stream_lengths(fcb);


    for (seq_no = corpus_get_begin(); corpus_next_utt(); seq_no++) {
	if (!(seq_no % 250)) {
	    E_INFOCONT(" [%u]", seq_no);
	}
	
	    if (mfcc) {
		free(mfcc[0]);
		ckd_free(mfcc);

		mfcc = NULL;
	    }

	    if (corpus_get_generic_featurevec(&mfcc, &n_frame, mfc_veclen) < 0) {
	      E_FATAL("Can't read input features from %s\n", corpus_utt());
	    }


	if (sseq != NULL) {
	    ckd_free((void *)sseq);
	    sseq = NULL;
	}

	/* read transcript and convert it into a senone sequence */
	sseq = get_sseq(mdef, lex, n_frame);
	if (sseq == NULL) {
	    E_WARN("senone sequence not produced; skipping.\n");

	    free(mfcc[0]);
	    ckd_free((void *)mfcc);
	    feat_array_free(feat);

	    continue;
	}
	
	    if (feat) {
		feat_array_free(feat);
		feat = NULL;
	    }

	    if (n_frame < 9) {
	      E_WARN("utt %s too short\n", corpus_utt());
	      if (mfcc) {
		ckd_free(mfcc[0]);
		ckd_free(mfcc);
		mfcc = NULL;
	      }
	      continue;
	    }

	    feat = feat_array_alloc(fcb, n_frame + feat_window_size(fcb));
	    feat_s2mfc2feat_live(fcb, mfcc, &n_frame, TRUE, TRUE, feat);

	    if (corpus_has_xfrm()) {
		if (a) {
		    for (i = 0; i < n_mllr_cls; i++) {
			for (j = 0; j < n_stream; j++) {
			    ckd_free_2d((void **)a[i][j]);
			}
		    }
		    ckd_free_2d((void **)a);
		}
		if (b) {
		    for (i = 0; i < n_mllr_cls; i++) {
			for (j = 0; j < n_stream; j++) {
			    ckd_free((void *)b[i][j]);
			}
		    }
		}

		corpus_get_xfrm(&a, &b,
				&veclen_tmp,
				&n_mllr_cls,
				&n_stream_tmp);

		if (n_stream != n_stream_tmp) {
		    E_FATAL("Feature module # of streams, %u, is inconsistent w/ MLLR matrix, %u\n",
			    n_stream, n_stream_tmp);
		}

		for (i = 0; i < n_mllr_cls; i++) {
		    for (j = 0; j < n_stream; j++) {
			invert(a[i][j], a[i][j], veclen[j]);
		    }
		}
	    }

	    for (t = 0; t < n_frame; t++) {
		if (corpus_has_xfrm()) {
		    /* determine the MLLR class for the frame */
		    mcls = cb2mllr[ts2cb[sseq[t]]];

		    /* Transform the feature space using the inverse MLLR transform */
		    xfrm_feat(a[mcls], b[mcls], feat[t], n_stream, veclen);
		}
		segdmp_add_feat(sseq[t], &feat[t], 1);
	    }
    }
    
    if (mfcc) {
	free(mfcc[0]);
	ckd_free(mfcc);
	
	mfcc = NULL;
    }

    if (feat) {
	feat_array_free(feat);
	feat = NULL;
    }

    if (sseq) {
	ckd_free((void *)sseq);
	sseq = NULL;
    }

    return S3_SUCCESS;
}
