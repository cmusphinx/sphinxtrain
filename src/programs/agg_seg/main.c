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
 *    Given a prior time segmentation of an acoustic model
 *    training corpus, aggregate the observations for each triphone
 *    together.  This allows the triphones to be trained individually
 *    rather than training the whole set at once.
 *
 *    Currently, the output files for this process conform to the
 *    SPHINX-II expected format.
 *	
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include "parse_cmd_ln.h"
#include "agg_phn_seg.h"
#include "cnt_st_seg.h"
#include "cnt_phn_seg.h"
#include "agg_st_seg.h"
#include "agg_phn_seg.h"
#include "agg_all_seg.h"

#include <s3/segdmp.h>
#include <s3/fp_cache.h>

#include <s3/lexicon.h>
#include <s3/model_def_io.h>
#include <s3/ts2cb.h>
#include <s3/s3ts2cb_io.h>
#include <s3/s3cb2mllr_io.h>
#include <s3/state_param_def_io.h>
#include <s3/cmd_ln.h>
#include <s3/ckd_alloc.h>
#include <s3/feat.h>
#include <s3/corpus.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <string.h>

int
initialize(lexicon_t **out_lex,
	   model_def_t **out_mdef,
	   uint32 **out_ts2cb,
	   int32 **out_cb2mllr,
	   segdmp_type_t *out_dmp_type)
{
    model_def_t *mdef;
    const char *dictfn;
    const char *fdictfn;
    lexicon_t *lex;
    const char *fn;
    uint32 tmp;
    uint32 n_ts;
    uint32 n_cb;
    uint32 n_map;
    /*eov*/
    
    if (cmd_ln_access("-moddeffn")) {
	if (model_def_read(&mdef,
			   cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	    E_WARN_SYSTEM("Unable to open model def file");
	    return S3_ERROR;
	}
    }
	
    fn = (const char *)cmd_ln_access("-ts2cbfn");
    
    if (fn) {
	if (strcmp(fn, SEMI_LABEL) == 0) {
	    /* semi-continuous */
	    *out_ts2cb = semi_ts2cb(mdef->n_tied_state);
	    n_ts = mdef->n_tied_state;
	    n_cb = 1;
	}
	else if (strcmp(fn, CONT_LABEL) == 0) {
	    /* continuous */
	    *out_ts2cb = cont_ts2cb(mdef->n_tied_state);
	    n_ts = mdef->n_tied_state;
	    n_cb = mdef->n_tied_state;
	}
	else if (s3ts2cb_read(fn,
			      out_ts2cb,
			      &n_ts,
			      &n_cb) != S3_SUCCESS) {
	    return S3_ERROR;
	}
    }

    *out_mdef = mdef;

    /* drop the mdef structure on the floor. Forget about it. */
    
    dictfn = (const char *)cmd_ln_access("-dictfn");
    lex = NULL;
    if (dictfn) {
	E_INFO("Reading lexicon %s.\n", dictfn);

	lex = lexicon_read(NULL,	/* no lexicon to start */
			   dictfn,
			   mdef->acmod_set);
	if (lex == NULL)
	    return S3_ERROR;
    }
    
    fdictfn = (const char *)cmd_ln_access("-fdictfn");
    if (fdictfn) {
	E_INFO("Reading filler lexicon %s.\n", fdictfn);
	
	(void)lexicon_read(lex,	/* add filler words to content lexicon */
			   fdictfn,
			   mdef->acmod_set);
    }

    *out_lex = lex;

    if (cmd_ln_access("-lsnfn"))
	corpus_set_lsn_filename(cmd_ln_access("-lsnfn"));
    else if (cmd_ln_access("-sentdir") && cmd_ln_access("-sentext")) {
	corpus_set_sent_dir(cmd_ln_access("-sentdir"));
	corpus_set_sent_ext(cmd_ln_access("-sentext"));
    }
    else {
	E_INFO("No lexical transcripts provided\n");
    }

    if (cmd_ln_access("-segdir") != NULL) {
	corpus_set_seg_dir(cmd_ln_access("-segdir"));
    }
    if (cmd_ln_access("-segext") != NULL) {
	corpus_set_seg_ext(cmd_ln_access("-segext"));
    }

    /* Conditionally do VQ code aggregation */
    if (cmd_ln_access("-ccodedir") != NULL) {
	corpus_set_ccode_dir(cmd_ln_access("-ccodedir"));
    }
    if (cmd_ln_access("-ccodeext") != NULL) {
	corpus_set_ccode_ext(cmd_ln_access("-ccodeext"));
    }

    if (cmd_ln_access("-dcodedir") != NULL) {
	corpus_set_dcode_dir(cmd_ln_access("-dcodedir"));
    }
    if (cmd_ln_access("-dcodeext") != NULL) {
	corpus_set_dcode_ext(cmd_ln_access("-dcodeext"));
    }

    if (cmd_ln_access("-pcodedir") != NULL) {
	corpus_set_pcode_dir(cmd_ln_access("-pcodedir"));
    }
    if (cmd_ln_access("-pcodeext") != NULL) {
	corpus_set_pcode_ext(cmd_ln_access("-pcodeext"));
    }

    if (cmd_ln_access("-ddcodedir") != NULL) {
	corpus_set_ddcode_dir(cmd_ln_access("-ddcodedir"));
    }
    if (cmd_ln_access("-ddcodeext") != NULL) {
	corpus_set_ddcode_ext(cmd_ln_access("-ddcodeext"));
    }

    if (cmd_ln_access("-cepdir") != NULL) {
	corpus_set_mfcc_dir(cmd_ln_access("-cepdir"));
    }
    if (cmd_ln_access("-cepext") != NULL) {
	corpus_set_mfcc_ext(cmd_ln_access("-cepext"));
    }

    if (cmd_ln_access("-feat") != NULL) {
	if (feat_set(cmd_ln_access("-feat")) != S3_SUCCESS) {
	    E_FATAL("Unable to use feature set %s\n",
		    cmd_ln_access("-feat"));
	}
	feat_set_in_veclen(*(int32 *)cmd_ln_access("-ceplen"));
    }

    if (cmd_ln_access("-ctlfn")) {
	corpus_set_ctl_filename(cmd_ln_access("-ctlfn"));
	if (cmd_ln_access("-nskip")) {
	    corpus_set_interval(*(int32 *)cmd_ln_access("-nskip"),
				*(int32 *)cmd_ln_access("-runlen"));
	}
    }

    if (cmd_ln_access("-nskip") && cmd_ln_access("-runlen")) {
	corpus_set_interval(*(uint32 *)cmd_ln_access("-nskip"),
			    *(uint32 *)cmd_ln_access("-runlen"));
    }

    if (cmd_ln_access("-mllrctlfn")) {
	corpus_set_mllr_filename(cmd_ln_access("-mllrctlfn"));

	fn = (const char *)cmd_ln_access("-cb2mllrfn");
	if (fn == NULL) {
	    E_FATAL("Specify -cb2mllrfn\n");
	}

	if (cmd_ln_access("-mllrdir")) {
	    corpus_set_mllr_dir(cmd_ln_access("-mllrdir"));
	}
	
	if (strcmp(fn, ".1cls.") == 0) {
	    *out_cb2mllr = (int32 *)ckd_calloc(n_cb, sizeof(int32));
	    n_map = n_cb;
	}
	else if (s3cb2mllr_read((const char *)cmd_ln_access("-cb2mllrfn"),
				out_cb2mllr,
				&n_map,			    
				&tmp) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	
	if (n_map != n_cb) {
	    E_FATAL("# mappings in cb2mllrfn, %u, != # codebooks, %u.\n",
		    n_map, n_cb);
	}
    }

    if (corpus_init() != S3_SUCCESS) {
	return S3_ERROR;
    }

    

    if (feat_id() != FEAT_ID_NONE) {
	/* feature module configured so need to make a feature dump */
	E_INFO("Will produce FEAT dump\n");
	*out_dmp_type = SEGDMP_TYPE_FEAT;
    }
    else if (corpus_provides_mfcc()) {
	/* feature module not configured and corpus is providing MFCC */
	E_INFO("Will produce MFCC dump\n");
	*out_dmp_type = SEGDMP_TYPE_MFCC;
    }
    else if (corpus_provides_ccode()) {
	/* feature module not configured and
	   corpus not providing MFCC but it is providing VQ */
	E_INFO("Will produce VQ dump\n");
	*out_dmp_type = SEGDMP_TYPE_VQ;
    }
    else {
	E_FATAL("Unknown segment dump type\n");
    }


    return S3_SUCCESS;
}

static uint32 *
cnt_st(model_def_t *mdef, lexicon_t *lex)
{
    uint32 i;
    uint32 *cnt;
    FILE *cnt_fp;

    cnt_fp = fopen((const char *)cmd_ln_access("-cntfn"), "r");
    if (cnt_fp == NULL) {
	E_INFO("Count file %s not found; creating.\n",
	       (const char *)cmd_ln_access("-cntfn"));
	cnt = cnt_st_seg(mdef, lex);
	E_INFO("Writing %s.\n",
	       (const char *)cmd_ln_access("-cntfn"));
	
	cnt_fp = fopen((const char *)cmd_ln_access("-cntfn"), "w");
	for (i = 0; i < mdef->n_tied_state; i++) {
	    fprintf(cnt_fp, "%u\n", cnt[i]);
	}
	fclose(cnt_fp);
	exit(0);
    }
    else {
	E_INFO("Reading %s\n",
	       (const char *)cmd_ln_access("-cntfn"));
	cnt = ckd_calloc(mdef->n_tied_state, sizeof(uint32));
	for (i = 0; i < mdef->n_tied_state; i++) {
	    if (fscanf(cnt_fp, "%u", &cnt[i]) != 1) {
		E_FATAL_SYSTEM("Error reading count file %s",
			       (const char *)cmd_ln_access("-cntfn"));
	    }
	}
	if (fscanf(cnt_fp, "%u", &i) != EOF) {
	    E_FATAL("Expected EOF on count file, but remaining data found\n");
	}

	fclose(cnt_fp);
    }

    return cnt;
}

static int
cnt_phn(model_def_t *mdef, lexicon_t *lex,
	uint32 **out_n_seg,
	uint32 ***out_n_frame)
{
    uint32 i, j;
    uint32 *n_seg;
    uint32 **n_frame;
    FILE *cnt_fp;
    uint32 n_acmod;

    cnt_fp = fopen((const char *)cmd_ln_access("-cntfn"), "r");
    if (cnt_fp == NULL) {
	E_INFO("Count file %s not found; creating.\n",
	       (const char *)cmd_ln_access("-cntfn"));
	cnt_phn_seg(mdef, lex, &n_seg, &n_frame);
	E_INFO("Writing %s.\n",
	       (const char *)cmd_ln_access("-cntfn"));
	
	cnt_fp = fopen((const char *)cmd_ln_access("-cntfn"), "w");
	for (i = 0; i < acmod_set_n_acmod(mdef->acmod_set); i++) {
	    fprintf(cnt_fp, "%u", n_seg[i]);
	    for (j = 0; j < n_seg[i]; j++) {
		fprintf(cnt_fp, " %u", n_frame[i][j]);
	    }
	    fprintf(cnt_fp, "\n");
	}
	fclose(cnt_fp);
	exit(0);
    }
    else {
	E_INFO("Reading %s\n",
	       (const char *)cmd_ln_access("-cntfn"));

	n_acmod = acmod_set_n_acmod(mdef->acmod_set);

	n_seg = (uint32 *)ckd_calloc(n_acmod, sizeof(uint32));
	n_frame = (uint32 **)ckd_calloc(n_acmod, sizeof(uint32 *));

	for (i = 0; i < n_acmod; i++) {
	    if (fscanf(cnt_fp, "%u", &n_seg[i]) != 1) {
		E_FATAL_SYSTEM("Error reading count file %s",
			       (const char *)cmd_ln_access("-cntfn"));
	    }

	    if (n_seg[i] != 0) {
		n_frame[i] = (uint32 *)ckd_calloc(n_seg[i], sizeof(uint32));

		for (j = 0; j < n_seg[i]; j++) {
		    if (fscanf(cnt_fp, "%u", &(n_frame[i][j])) != 1) {
			E_FATAL_SYSTEM("Error reading count file %s",
				       (const char *)cmd_ln_access("-cntfn"));
		    }
		}
	    }
	}

	if (fscanf(cnt_fp, "%u", &i) != EOF) {
	    E_FATAL("Expected EOF on count file, but remaining data found\n");
	}

	fclose(cnt_fp);
    }

    *out_n_seg = n_seg;
    *out_n_frame = n_frame;

    return S3_SUCCESS;
}

int main(int argc, char *argv[])
{
    lexicon_t *lex;
    model_def_t *mdef;
    const char *segtype;
    segdmp_type_t dmp_type;
    uint32 *n_seg;
    uint32 **n_frame;
    uint32 *ts2cb;
    int32 *cb2mllr;
    /*eov*/

    parse_cmd_ln(argc, argv);

    if (initialize(&lex, &mdef, &ts2cb, &cb2mllr, &dmp_type) != S3_SUCCESS) {
	exit(1);
    }

    segtype = (const char *)cmd_ln_access("-segtype");

    if (strcmp(segtype, "all") == 0) {
	E_INFO("Writing frames to one file\n");

	if (agg_all_seg(dmp_type,
			(const char *)cmd_ln_access("-segdmpfn"),
			*(uint32 *)cmd_ln_access("-stride")) != S3_SUCCESS) {
	    exit(1);
	}
    }
    else if (strcmp(segtype, "st") == 0) {
	segdmp_set_bufsz(*(uint32 *)cmd_ln_access("-cachesz"));
	
	if (segdmp_open_write((const char **)cmd_ln_access("-segdmpdirs"),
			      (const char *)cmd_ln_access("-segdmpfn"),
			      (const char *)cmd_ln_access("-segidxfn"),
			      mdef->n_tied_state,
			      cnt_st(mdef, lex),
			      NULL,
			      dmp_type) != S3_SUCCESS) {
	    E_FATAL("Unable to initialize segment dump\n");
	}
	
	if (agg_st_seg(mdef, lex, ts2cb, cb2mllr, dmp_type) != S3_SUCCESS) {
	    exit(1);
	}
	
	segdmp_close();
    }
    else if (strcmp(segtype, "phn") == 0) {
	segdmp_set_bufsz(*(uint32 *)cmd_ln_access("-cachesz"));
	
	cnt_phn(mdef, lex, &n_seg, &n_frame);

	if (segdmp_open_write((const char **)cmd_ln_access("-segdmpdirs"),
			      (const char *)cmd_ln_access("-segdmpfn"),
			      (const char *)cmd_ln_access("-segidxfn"),
			      acmod_set_n_acmod(mdef->acmod_set),
			      n_seg,
			      n_frame,
			      dmp_type) != S3_SUCCESS) {
	    E_FATAL("Unable to initialize segment dump\n");
	}
	
	if (agg_phn_seg(lex, mdef->acmod_set, dmp_type) != S3_SUCCESS) {
	    exit(1);
	}
	
	segdmp_close();
    }
    else if (strcmp(segtype, "wrd") == 0) {
	E_FATAL("Unimplememented -segtype wrd\n");
    }
    else if (strcmp(segtype, "utt") == 0) {
	E_FATAL("Unimplememented -segtype utt\n");
    }
    else {
	E_FATAL("Unhandled -segtype %s\n", segtype);
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.5  2006/03/27  03:30:14  dhdfu
 * Fix some minor signedness issues to keep the compiler happy
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
 * Revision 1.5  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.4  96/07/29  16:03:59  eht
 * - Implement -nskip and -runlen cmd ln args.
 * - Deal w/ new model def data structure and read routine.
 * 
 * Revision 1.3  1996/03/25  15:45:23  eht
 * Development version
 *
 * Revision 1.2  1996/03/04  16:01:06  eht
 * *** empty log message ***
 *
 *
 */
