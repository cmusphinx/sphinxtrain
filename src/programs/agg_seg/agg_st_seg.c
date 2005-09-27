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
#include <s3/feat.h>
#include <s3/ck_seg.h>
#include <s3/mk_sseq.h>
#include <s3/mk_phone_seq.h>
#include <s3/ckd_alloc.h>

static void
xfrm_feat(float32 ***ainv,
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
		o[i] += ainv[s][i][j] * f[s][j];
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
    uint32 n_frame;
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
	   uint32 *ts2cb,
	   uint32 *cb2mllr,
	   segdmp_type_t type)
{
    uint32 seq_no;
    vector_t *mfcc = NULL;
    uint32 mfc_veclen;
    uint32 n_frame;
    vector_t **feat = NULL;
    uint32 *sseq = NULL;
    uint32 i, j;
    uint32 t;
    int32 sv_feat = FALSE;
    int32 sv_mfcc = FALSE;
    int32 sv_vq = FALSE;
    uint32 n_stream=0;
    uint32 n_stream_tmp;
    const uint32 *veclen = 0;
    const uint32 *veclen_tmp;
    uint32 n_mllr_cls;
    float32 ****a = NULL;
    float32 ****ainv = NULL;
    float32 ***b = NULL;
    uint32 mcls;
    /*eov*/

    if (type == SEGDMP_TYPE_FEAT) {
	sv_feat = TRUE;
	n_stream = feat_n_stream();
	veclen = feat_vecsize();
    }
    else if (type == SEGDMP_TYPE_MFCC) {
	sv_mfcc = TRUE;
    }
    else if (type == SEGDMP_TYPE_VQ) {
	sv_vq = TRUE;
    }

    if (sv_vq) {
	E_FATAL("VQ aggregation of states not supported\n");
    }

    for (seq_no = corpus_get_begin(); corpus_next_utt(); seq_no++) {
	if (!(seq_no % 250)) {
	    fprintf(stderr, " [%u]", seq_no);
	    fflush(stderr);
	}
	
	if (sv_feat || sv_mfcc) {
	    if (mfcc) {
		free(mfcc[0]);
		ckd_free(mfcc);

		mfcc = NULL;
	    }

	    /* get the MFCC data for the utterance */
	    if (corpus_get_mfcc(&mfcc, &n_frame, &mfc_veclen) < 0) {
		E_FATAL("Can't read input features\n");
	    }
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
	    feat_free(feat);

	    continue;
	}
	
	if (sv_feat) {
	    if (feat) {
		feat_free(feat);
		feat = NULL;
	    }

	    if (n_frame < 9) {
	      E_WARN("utt %s too short\n", corpus_utt());
	      if (mfcc) {
		ckd_free(mfcc[0]);
		ckd_free(mfcc);
	      }
	      continue;
	    }

	    feat = feat_compute(mfcc, &n_frame);

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

		/* Free the prior A^(-1) if any */
		if (ainv) {
		    for (i = 0; i < n_mllr_cls; i++) {
			for (j = 0; j < n_stream; j++) {
			    ckd_free_2d((void **)ainv[i][j]);
			}
		    }
		    ckd_free_2d((void **)ainv);
		}

		/* Compute A^(-1) for the current transform */
		ainv = (float32 ****)ckd_calloc_2d(n_mllr_cls, n_stream, sizeof(float32 **));
		for (i = 0; i < n_mllr_cls; i++) {
		    for (j = 0; j < n_stream; j++) {
			ainv[i][j] = (float32 **)ckd_calloc_2d(veclen[j], veclen[j], sizeof(float32));

			invert(ainv[i][j], a[i][j], veclen[j]);
		    }
		}
	    }

	    for (t = 0; t < n_frame; t++) {
		if (corpus_has_xfrm()) {
		    /* determine the MLLR class for the frame */
		    mcls = cb2mllr[ts2cb[sseq[t]]];

		    /* Transform the feature space using the inverse MLLR transform */
		    xfrm_feat(ainv[mcls], b[mcls], feat[t], n_stream, veclen);
		}
		segdmp_add_feat(sseq[t], &feat[t], 1);
	    }
	}
	else if (sv_mfcc) {
	    for (t = 0; t < n_frame; t++) {
		segdmp_add_mfcc(sseq[t], &mfcc[t], 1, mfc_veclen);
	    }
	}	    
    }
    
    if (mfcc) {
	free(mfcc[0]);
	ckd_free(mfcc);
	
	mfcc = NULL;
    }

    if (feat) {
	feat_free(feat);
	feat = NULL;
    }

    if (sseq) {
	ckd_free((void *)sseq);
	sseq = NULL;
    }

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.5  2005/09/27  02:02:47  arthchan2003
 * Check whether utterance is too short in init_gau, bw and agg_seg.
 * 
 * Revision 1.4  2004/07/21 18:30:32  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
