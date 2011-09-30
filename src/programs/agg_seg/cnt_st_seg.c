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
 * File: cnt_st_seg.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "cnt_st_seg.h"

#include <s3/lexicon.h>
#include <s3/corpus.h>
#include <s3/segdmp.h>
#include <s3/ck_seg.h>
#include <s3/mk_sseq.h>
#include <s3/mk_phone_seq.h>
#include <sphinxbase/ckd_alloc.h>


static uint32 *
get_next_sseq(model_def_t *mdef,
	      lexicon_t *lex,
	      uint32 *out_n_frame)
{
    char *trans;
    uint16 *seg;
    int32 n_frame;
    acmod_id_t *phone;
    uint32 n_phone;
    uint32 *sseq;
    
    corpus_get_sent(&trans);
    corpus_get_seg(&seg, &n_frame);

    mk_phone_seq(&phone, &n_phone, trans, mdef->acmod_set, lex);

    ck_seg(mdef->acmod_set, phone, n_phone, seg, n_frame, corpus_utt());

    sseq = mk_sseq(seg, n_frame, phone, n_phone, mdef);
    
    ckd_free(phone);
    ckd_free(seg);
    ckd_free(trans);

    *out_n_frame = n_frame;

    return sseq;
}


uint32 *
cnt_st_seg(model_def_t *mdef,
	   lexicon_t *lex)
{
    uint32 seq_no;
    uint32 n_frame;
    uint32 *sseq;
    uint32 *n_frame_per;
    uint32 i;

    E_INFO("Counting # occ. for %u tied states\n", mdef->n_tied_state);

    n_frame_per = ckd_calloc(mdef->n_tied_state, sizeof(uint32));

    for (seq_no = corpus_get_begin(); corpus_next_utt(); seq_no++) {
	if (!(seq_no % 250)) {
	    E_INFOCONT(" cnt[%u]", seq_no);
	}
	
	/* Read transcript and convert it into a senone sequence */
	sseq = get_next_sseq(mdef, lex, &n_frame);

	if (sseq == NULL) {
	    E_WARN("senone sequence not produced; skipping.\n");

	    continue;
	}

	for (i = 0; i < n_frame; i++) {
	    n_frame_per[sseq[i]]++;
	}

	ckd_free(sseq);
    }

    for (i = 0; i < mdef->n_tied_state; i++) {
	E_INFO("ts= %u cnt= %u\n", i, n_frame_per[i]);
    }

    return n_frame_per;
}
