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
#include <s3/prim_type.h>
#include <s3/corpus.h>
#include <s3/mk_wordlist.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>
#include <s3/gauden.h>
#include <s3/s3gau_io.h>
#include <s3/vector.h>
#include <s3/feat.h>
#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/s3.h>

/* ADDITION BY BHIKSHA, TO FACILITATE FEATURES OF ARBITRARY LENGTH, 7jan98 */
#include <s3/s2_param.h>
/* END CHANGES BY BHIKSHA */

#include <sys_compat/file.h>

#include <stdio.h>
#include <assert.h>


int
init_gau(lexicon_t *lex,
	 model_def_t *mdef)
{
    char *trans = NULL;
    acmod_set_t *acmod_set;

    vector_t *mfcc = NULL;
    uint32 n_frame;
    uint32 feat_n_frame;
    uint32 framelen;
    uint32 tmp;
    
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
    float32 ***dnom = NULL;

    const uint32 *veclen;

    uint32 n_ts;

    uint32 *r_veclen;
    uint32 r_n_ts;
    uint32 r_n_feat;
    uint32 r_n_density;

    vector_t **f = NULL;

    const char *meanfn;

    uint32 *del_b = NULL;
    uint32 *del_e = NULL;
    uint32 n_del = 0;

    char fn[MAXPATHLEN+1];

/* ADDITION BY BHIKSHA, TO CHECK FOR FEATURE LENGTH, 7 JAN 98 */
    uint32 ceplen;
/* END ADDITION BY BHIKSHA */

    if (mdef) {
	acmod_set = mdef->acmod_set;
	n_ts = mdef->n_tied_state;
    }
    else {
	acmod_set = NULL;
	n_ts = 1;		/* Global mean/var */
    }

    meanfn = (char *)cmd_ln_access("-meanfn");

    veclen = feat_vecsize();
    
    if (meanfn == NULL) {
	E_INFO("Computing %ux%ux1 mean estimates\n", n_ts, feat_n_stream());
    
	mean_acc = gauden_alloc_param(n_ts,
				      feat_n_stream(),
				      1,
				      veclen);
	var_acc = NULL;
    }
    else {
	assert(meanfn != NULL);

	E_INFO("Computing %ux%ux1 variance estimates\n", n_ts, feat_n_stream());


	if (s3gau_read(meanfn,
		       &mean,
		       &r_n_ts,
		       &r_n_feat,
		       &r_n_density,
		       &r_veclen) != S3_SUCCESS) {
	    E_FATAL_SYSTEM("Unable to open %s for reading\n", meanfn);
	}

	mean_acc = NULL;
	var_acc =  gauden_alloc_param(n_ts,
				      feat_n_stream(),
				      1,
				      veclen);
    }

    dnom = (float32 ***)ckd_calloc_3d(n_ts, feat_n_stream(), 1, sizeof(float32));

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
	    feat_free(f);
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
	    
	    word = mk_wordlist(trans, &n_word);
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



/* CHANGE BY BHIKSHA; IF INPUT VECLEN != 13, THEN DO NOT USE THE
   REGULAR corpus_get_mfcc() WHICH REQUIRES INPUT DATA TO BE 13 DIMENSIONAL
   CEPSTRA. USE, INSTEAD, THE HACKED VERSION corpus_get_generic_featurevec()
   WHICH TAKES FEATURES OF ARBITRARY LENGTH
   7 JAN 1998 */
	ceplen = *(int32 *)cmd_ln_access("-ceplen");
        if (ceplen == S2_CEP_VECLEN) {
	    if (corpus_get_mfcc(&mfcc, &tmp, &framelen) != S3_SUCCESS) {
	        E_FATAL("Unable to read MFCC data for %s\n", corpus_utt_brief_name());
	    }
	    assert(framelen == ceplen);
        }
        else {
	    if (corpus_get_generic_featurevec(&mfcc, &tmp, ceplen) < 0) {
	        E_FATAL("Can't read input features\n");
	    }
        }

/* END CHANGES BY BHIKSHA */

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


	f = feat_compute(mfcc, &feat_n_frame);

	if (feat_n_frame != n_frame) {
	    E_FATAL("# frames compute != # frames of state seg\n");
	}

	if (del_b) {
	    ckd_free(del_b);
	}
	if (del_e) {
	    ckd_free(del_e);
	}

	if (corpus_get_sildel(&del_b, &del_e, &n_del) != S3_SUCCESS) {
	    E_ERROR("Unable to get silence deletions for %s\n",
		    corpus_utt());
	}

	if (mean_acc) {
	    /* accumulate mean sums since no estimate given */
	    accum_state_mean(mean_acc, dnom, f, del_b, del_e, n_del, feat_n_stream(), veclen, sseq, ci_sseq, n_frame);
	}
	else if (var_acc) {
	    /* accumulate var sums since mean estimate exists */
	    accum_state_var(var_acc, mean, dnom, f, del_b, del_e, n_del, feat_n_stream(), veclen, sseq, ci_sseq, n_frame);
	}
    }

    sprintf(fn, "%s/gauden_counts", (const char *)cmd_ln_access("-accumdir"));
    
    if (s3gaucnt_write(fn, mean_acc, var_acc, TRUE /* 2-pass variance */, dnom,
		       n_ts, feat_n_stream(), 1, veclen) != 0) {
	exit(1);
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
	feat_free(f);
	f = NULL;
    }

    if (mean_acc) {
	gauden_free_param(mean_acc);
    }
    if (var_acc) {
	gauden_free_param(var_acc);
    }
    ckd_free_3d((void ***)dnom);

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.7  2005/11/10  19:31:14  dhdfu
 * also prevent a double free() when the utt is too short
 * 
 * Revision 1.6  2005/11/03 04:17:07  dhdfu
 * Make init_gau use the same (semi-arbitrary) lower bound on the size of
 * an utterance as bw does, with a comment that this is arbitrary.
 * Prevents segfaults.
 *
 * Revision 1.5  2005/09/27 02:02:47  arthchan2003
 * Check whether utterance is too short in init_gau, bw and agg_seg.
 *
 * Revision 1.4  2004/07/21 18:30:34  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.8  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.7  1996/08/06  14:13:56  eht
 * Included missing header files to define prototypes
 *
 * Revision 1.6  1996/04/02  17:04:53  eht
 * Include mk_sseq from libcommon rather than locally
 *
 * Revision 1.5  1996/03/25  15:43:07  eht
 * Deal w/ some memory leaks
 *
 * Revision 1.4  1996/02/02  17:33:32  eht
 * Added estimation of mean/var of CI states when only CD states are present.
 *
 * Revision 1.3  1996/01/30  17:11:23  eht
 * Check return status of read routines
 *
 * Revision 1.2  1995/12/14  20:01:53  eht
 * Yet another development version.  Many changes
 *
 * Revision 1.1  1995/12/01  20:55:40  eht
 * Initial revision
 *
 *
 */
