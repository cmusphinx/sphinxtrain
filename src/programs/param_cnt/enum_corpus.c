/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
#include "mk_sseq.h"

/* BHIKSHA FIX */
#include "phone_cnt.h"
/* END BHIKSHA FIX */

#include <s3/ck_seg.h>
#include <s3/prim_type.h>
#include <s3/corpus.h>
#include <s3/mk_wordlist.h>
#include <s3/mk_phone_list.h>
#include <s3/vector.h>
#include <s3/feat.h>
#include <s3/ckd_alloc.h>
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
    uint32 n_frame;
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

/* BHIKSHA FIX; IF ONLY PHONE COUNTS WANTED DONT NEED STATE SEGS */
        if (cnt_fn != phone_cnt){
	    if (corpus_get_seg(&seg, &n_frame) != S3_SUCCESS) {
	        E_FATAL("Unable to read Viterbi state segmentation for %s\n",
		    corpus_utt_brief_name());
            }
	}
/* END BHIKSHA FIX */
	    
	word = mk_wordlist(trans, &n_word);
	phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);
	if (phone == NULL) {
	    E_WARN("Unable to produce phone sequence; skipping utt %s\n", corpus_utt());
	    continue;
	}

/* BHIKSHA FIX; THIS STEP ONLY IF WE NEED STATE COUNTS */
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

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:32  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/07  08:40:21  eht
 * Initial revision
 * 
 * Revision 1.1  1996/03/25  15:21:20  eht
 * Initial revision
 *
 *
 */
