/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: agg_phn_seg.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "agg_phn_seg.h"
#include "mk_seg.h"

#include <s3/segdmp.h>
#include <s3/corpus.h>
#include <s3/mk_wordlist.h>
#include <s3/mk_phone_list.h>
#include <s3/ck_seg.h>
#include <s3/cvt2triphone.h>
#include <s3/ckd_alloc.h>
#include <s3/feat.h>

#include <stdio.h>
#include <assert.h>


int
agg_phn_seg(lexicon_t *lex,
	    acmod_set_t *acmod_set,
	    segdmp_type_t type)
{
    uint16 *seg;
    unsigned char *ccode;
    unsigned char *dcode;
    unsigned char *pcode;
    unsigned char *ddcode;
    vector_t *mfcc;
    vector_t **feat;
    uint32 n_coeff;
    uint32 n_frame;
    uint32 tmp;
    uint32 tick_cnt;

    acmod_id_t *phone;
    uint32 *start;
    uint32 *len;
    uint32 n_phone;
    uint32 s;
    char *btw_mark;

    char *trans;
    char **word;
    uint32 n_word;

    uint32 sv_feat = FALSE;
    uint32 sv_vq = FALSE;
    uint32 sv_mfcc = FALSE;

    uint32 n_stream;
    const uint32 *veclen;

    tick_cnt = 0;

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


    while (corpus_next_utt()) {
	if ((++tick_cnt % 500) == 0) {
	    printf("[%u] ", tick_cnt);
	    fflush(stdout);
	}

	if (corpus_get_sent(&trans) != S3_SUCCESS) {
	    E_FATAL("Unable to read word transcript for %s\n", corpus_utt_brief_name());
	}

	if (corpus_get_seg(&seg, &n_frame) != S3_SUCCESS) {
	    E_FATAL("Unable to read Viterbi state segmentation for %s\n", corpus_utt_brief_name());
	}
	    
	word = mk_wordlist(trans, &n_word);
	phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);
	start = ckd_calloc(n_phone, sizeof(uint32));
	len = ckd_calloc(n_phone, sizeof(uint32));

	/* check to see whether the word transcript and dictionary entries
	   agree with the state segmentation */
	if (ck_seg(acmod_set, phone, n_phone, seg, n_frame, corpus_utt()) != S3_SUCCESS) {
	    free(trans);	/* alloc'ed using strdup, not ckd_*() */
	    free(seg);	/* alloc'ed using malloc in areadshort(), not ckd_*() */
	    ckd_free(word);
	    ckd_free(phone);
	    
	    E_ERROR("ck_seg failed");

	    continue;
	}

	if (cvt2triphone(acmod_set, phone, btw_mark, n_phone) != S3_SUCCESS) {
	    free(trans);	/* alloc'ed using strdup, not ckd_*() */
	    free(seg);		/* alloc'ed using malloc in areadshort(), not ckd_*() */
	    ckd_free(word);
	    ckd_free(phone);

	    E_ERROR("cvt2triphone failed");
	    
	    continue;
	}

	ckd_free(btw_mark);

	if (mk_seg(acmod_set,
		   seg,
		   n_frame,
		   phone,
		   start,
		   len,
		   n_phone) != S3_SUCCESS) {
	    free(trans);
	    free(seg);
	    ckd_free(word);
	    ckd_free(phone);

	    E_ERROR("mk_seg failed");
	    continue;
	}
	
	if (corpus_provides_ccode()) {
	    /* assume that if it provides ccode, it provides
	       the remaining types */
	    corpus_get_ccode(&ccode, &tmp);
	    assert(tmp == n_frame);
	    corpus_get_dcode(&dcode, &tmp);
	    assert(tmp == n_frame);
	    corpus_get_pcode(&pcode, &tmp);
	    assert(tmp == n_frame);
	    corpus_get_ddcode(&ddcode, &tmp);
	    assert(tmp == n_frame);

	    for (s = 0; s < n_phone; s++) {
		segdmp_add_vq(phone[s],
			      &ccode[start[s]],
			      &dcode[start[s]],
			      &pcode[start[s]],
			      &ddcode[start[s]],
			      len[s]);
	    }

	    free(ccode);
	    free(dcode);
	    free(pcode);
	    free(ddcode);
	}
	else if (corpus_provides_mfcc()) {
	    if (feat_id() != NO_ID) {
		corpus_get_mfcc(&mfcc, &n_frame, &n_coeff);
		feat_set_in_veclen(n_coeff);
		
		feat = feat_compute(mfcc, &n_frame);

		for (s = 0; s < n_phone; s++) {
		    segdmp_add_feat(phone[s],
				    &feat[start[s]],
				    len[s]);
		}

		feat_free(feat);
		free(&mfcc[0][0]);
		ckd_free(mfcc);
	    }
	    else {
		corpus_get_mfcc(&mfcc, &n_frame, &n_coeff);

		for (s = 0; s < n_phone; s++) {
		    segdmp_add_mfcc(phone[s],
				    &mfcc[start[s]],
				    len[s],
				    n_coeff);
		}

		free(&mfcc[0][0]);
		ckd_free(mfcc);
	    }
	}
	else {
	    E_FATAL("No data type specified\n");
	}

	free(trans);	/* alloc'ed using strdup, not ckd_*() */
	free(seg);	/* alloc'ed using malloc in areadshort(), not ckd_*() */
	ckd_free(word);
	ckd_free(phone);
	ckd_free(start);
	ckd_free(len);
    }

    return 0;
}



/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 * Revision 1.3  1996/07/29  16:02:57  eht
 * segdmp module gets n_stream and veclen parameters at initialization time.
 * No need to pass these as args.
 *
 * Revision 1.2  1996/03/25  15:45:23  eht
 * Development version
 *
 *
 */
