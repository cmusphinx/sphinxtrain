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
 * File: enum_corpus.c
 * 
 * Description: 
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include "enum_corpus.h"
#include "cnt_fn.h"
#include "phone_cnt.h"

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/prim_type.h>

#include <s3/mk_sseq.h>
#include <s3/ck_seg.h>
#include <s3/corpus.h>
#include <s3/mk_phone_list.h>
#include <s3/vector.h>
#include <s3/s3.h>

#include <stdio.h>

int
enum_corpus(lexicon_t *lex,
	    model_def_t *mdef,
	    uint32 *cnt,
	    cnt_fn_t cnt_fn)
{
    uint32 tick_cnt = 0;
    char *trans = NULL;
    uint16 *seg = NULL;
    int32 n_frame;
    char **word = NULL;
    uint32 n_word;
    acmod_id_t *phone = NULL;
    uint32 n_phone;
    char *btw_mark = NULL;

    while (corpus_next_utt()) {
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
	
	if ((++tick_cnt % 1000) == 0) {
	    fprintf(stderr, "[%u] ", tick_cnt);
	    fflush(stderr);
	}

	if (corpus_get_sent(&trans) != S3_SUCCESS) {
	    E_FATAL("Unable to read word transcript for %s\n", corpus_utt_brief_name());
	}

        if (cnt_fn != phone_cnt){
	    if (corpus_get_seg(&seg, &n_frame) != S3_SUCCESS) {
	        E_FATAL("Unable to read Viterbi state segmentation for %s\n",
		    corpus_utt_brief_name());
            }
	}
	    
	n_word = str2words(trans, NULL, 0);
	word = ckd_calloc(n_word, sizeof(char*));
	str2words(trans, word, n_word);

	phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);
	if (phone == NULL) {
	    E_WARN("Unable to produce phone sequence; skipping utt %s\n", corpus_utt());
	    continue;
	}

        if (cnt_fn != phone_cnt){
	    /* check to see whether the word transcript and dictionary entries
	       agree with the state segmentation */
	    if (ck_seg(mdef->acmod_set, phone, n_phone, seg, n_frame, corpus_utt()) != S3_SUCCESS) {
	        continue;
            }
	}

	(*cnt_fn)(cnt,				/* observation counts */
		  mdef,				/* model definitions */
		  seg, n_frame,			/* Viterbi state segmentation */
		  phone, btw_mark, n_phone);	/* list of phones */
    }

    /* free the per utterance data structures from the last utt */
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
    
    return S3_SUCCESS;
}
