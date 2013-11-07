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
#include "cnt_st_seg.h"
#include "cnt_phn_seg.h"
#include "agg_st_seg.h"
#include "agg_phn_seg.h"
#include "agg_all_seg.h"

#include <s3/segdmp.h>

#include <s3/lexicon.h>
#include <s3/model_def_io.h>
#include <s3/ts2cb.h>
#include <s3/s3ts2cb_io.h>
#include <s3/s3cb2mllr_io.h>
#include <s3/corpus.h>
#include <s3/s3.h>

#include <string.h>

#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/feat.h>


int
initialize(lexicon_t **out_lex,
	   model_def_t **out_mdef,
	   feat_t **out_feat,
	   uint32 **out_ts2cb,
	   int32 **out_cb2mllr,
	   segdmp_type_t *out_dmp_type)
{
    model_def_t *mdef;
    const char *dictfn;
    const char *fdictfn;
    feat_t *feat;
    lexicon_t *lex;
    const char *fn;
    uint32 tmp;
    uint32 n_ts;
    uint32 n_cb;
    uint32 n_map;
    
    if (cmd_ln_str("-moddeffn")) {
	if (model_def_read(&mdef,
			   cmd_ln_str("-moddeffn")) != S3_SUCCESS) {
	    E_ERROR_SYSTEM("Unable to open model def file");
	    return S3_ERROR;
	}
    } else {
	mdef = NULL;
    }
	
    fn = cmd_ln_str("-ts2cbfn");
    
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
	else if (strcmp(fn, PTM_LABEL) == 0) {
	    *out_ts2cb = ptm_ts2cb(mdef);
	    n_ts = mdef->n_tied_state;
	    n_cb = mdef->acmod_set->n_ci;
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
    
    dictfn = cmd_ln_str("-dictfn");
    lex = NULL;
    if (dictfn) {
	E_INFO("Reading dictionary %s.\n", dictfn);

	lex = lexicon_read(NULL,	/* no lexicon to start */
			   dictfn,
			   mdef->acmod_set);
	if (lex == NULL)
	    return S3_ERROR;
    }
    
    fdictfn = cmd_ln_str("-fdictfn");
    if (fdictfn) {
	E_INFO("Reading filler dictionary %s.\n", fdictfn);
	
	(void)lexicon_read(lex,	/* add filler words to content lexicon */
			   fdictfn,
			   mdef->acmod_set);
    }

    *out_lex = lex;

    if (cmd_ln_str("-lsnfn"))
	corpus_set_lsn_filename(cmd_ln_str("-lsnfn"));
    else if (cmd_ln_str("-sentdir") && cmd_ln_str("-sentext")) {
	corpus_set_sent_dir(cmd_ln_str("-sentdir"));
	corpus_set_sent_ext(cmd_ln_str("-sentext"));
    }
    else {
	E_INFO("No lexical transcripts provided\n");
    }

    if (cmd_ln_str("-segdir") != NULL) {
	corpus_set_seg_dir(cmd_ln_str("-segdir"));
    }
    if (cmd_ln_str("-segext") != NULL) {
	corpus_set_seg_ext(cmd_ln_str("-segext"));
    }


    if (cmd_ln_str("-cepdir") != NULL) {
	corpus_set_mfcc_dir(cmd_ln_str("-cepdir"));
    }
    if (cmd_ln_str("-cepext") != NULL) {
	corpus_set_mfcc_ext(cmd_ln_str("-cepext"));
    }


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



    if (cmd_ln_str("-ctlfn")) {
	corpus_set_ctl_filename(cmd_ln_str("-ctlfn"));
        if (cmd_ln_int32("-nskip") && cmd_ln_int32("-runlen")) {
	    corpus_set_interval(cmd_ln_int32("-nskip"),
			        cmd_ln_int32("-runlen"));
    	} else if (cmd_ln_int32("-part") && cmd_ln_int32("-npart")) {
	    corpus_set_partition(cmd_ln_int32("-part"),
			         cmd_ln_int32("-npart"));
	}
    }

    if (cmd_ln_str("-mllrctlfn")) {
	corpus_set_mllr_filename(cmd_ln_str("-mllrctlfn"));

	fn = cmd_ln_str("-cb2mllrfn");
	if (fn == NULL) {
	    E_FATAL("Specify -cb2mllrfn\n");
	}

	if (cmd_ln_str("-mllrdir")) {
	    corpus_set_mllr_dir(cmd_ln_str("-mllrdir"));
	}
	
	if (strcmp(fn, ".1cls.") == 0) {
	    *out_cb2mllr = (int32 *)ckd_calloc(n_cb, sizeof(int32));
	    n_map = n_cb;
	}
	else if (s3cb2mllr_read(cmd_ln_str("-cb2mllrfn"),
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

    E_INFO("Will produce feature dump\n");

    return S3_SUCCESS;
}

static uint32 *
cnt_st(model_def_t *mdef, lexicon_t *lex)
{
    uint32 i;
    uint32 *cnt;
    FILE *cnt_fp;

    cnt_fp = fopen(cmd_ln_str("-cntfn"), "r");
    if (cnt_fp == NULL) {
	E_INFO("Count file %s not found; creating.\n",
	       cmd_ln_str("-cntfn"));
	cnt = cnt_st_seg(mdef, lex);
	E_INFO("Writing %s.\n",
	       cmd_ln_str("-cntfn"));
	
	cnt_fp = fopen(cmd_ln_str("-cntfn"), "w");
	for (i = 0; i < mdef->n_tied_state; i++) {
	    fprintf(cnt_fp, "%u\n", cnt[i]);
	}
	fclose(cnt_fp);
	exit(0);
    }
    else {
	E_INFO("Reading %s\n",
	       cmd_ln_str("-cntfn"));
	cnt = ckd_calloc(mdef->n_tied_state, sizeof(uint32));
	for (i = 0; i < mdef->n_tied_state; i++) {
	    if (fscanf(cnt_fp, "%u", &cnt[i]) != 1) {
		E_FATAL_SYSTEM("Error reading count file %s",
			       cmd_ln_str("-cntfn"));
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

    cnt_fp = fopen(cmd_ln_str("-cntfn"), "r");
    if (cnt_fp == NULL) {
	E_INFO("Count file %s not found; creating.\n",
	       cmd_ln_str("-cntfn"));
	cnt_phn_seg(mdef, lex, &n_seg, &n_frame);
	E_INFO("Writing %s.\n",
	       cmd_ln_str("-cntfn"));
	
	cnt_fp = fopen(cmd_ln_str("-cntfn"), "w");
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
	       cmd_ln_str("-cntfn"));

	n_acmod = acmod_set_n_acmod(mdef->acmod_set);

	n_seg = (uint32 *)ckd_calloc(n_acmod, sizeof(uint32));
	n_frame = (uint32 **)ckd_calloc(n_acmod, sizeof(uint32 *));

	for (i = 0; i < n_acmod; i++) {
	    if (fscanf(cnt_fp, "%u", &n_seg[i]) != 1) {
		E_FATAL_SYSTEM("Error reading count file %s",
			       cmd_ln_str("-cntfn"));
	    }

	    if (n_seg[i] != 0) {
		n_frame[i] = (uint32 *)ckd_calloc(n_seg[i], sizeof(uint32));

		for (j = 0; j < n_seg[i]; j++) {
		    if (fscanf(cnt_fp, "%u", &(n_frame[i][j])) != 1) {
			E_FATAL_SYSTEM("Error reading count file %s",
				       cmd_ln_str("-cntfn"));
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
    feat_t *feat;
    const char *segtype;
    segdmp_type_t dmp_type;
    uint32 *n_seg;
    uint32 **n_frame;
    uint32 *ts2cb;
    int32 *cb2mllr;
    /*eov*/

    parse_cmd_ln(argc, argv);

    if (initialize(&lex, &mdef, &feat, &ts2cb, &cb2mllr, &dmp_type) != S3_SUCCESS) {
	exit(1);
    }

    segtype = cmd_ln_str("-segtype");

    if (strcmp(segtype, "all") == 0) {
	E_INFO("Writing frames to one file\n");

	if (agg_all_seg(feat,
			dmp_type,
			cmd_ln_str("-segdmpfn"),
			cmd_ln_int32("-stride")) != S3_SUCCESS) {
	    exit(1);
	}
    }
    else if (strcmp(segtype, "st") == 0) {
	segdmp_set_bufsz(cmd_ln_int32("-cachesz"));
	
	if (segdmp_open_write(cmd_ln_str_list("-segdmpdirs"),
			      cmd_ln_str("-segdmpfn"),
			      cmd_ln_str("-segidxfn"),
			      mdef->n_tied_state,
			      cnt_st(mdef, lex),
			      NULL,
			      dmp_type,
			      feat_dimension1(feat),
			      feat_stream_lengths(feat),
			      feat_dimension(feat)) != S3_SUCCESS) {
	    E_FATAL("Unable to initialize segment dump\n");
	}
	
	if (agg_st_seg(mdef, lex, feat, ts2cb, cb2mllr, dmp_type) != S3_SUCCESS) {
	    exit(1);
	}
	
	segdmp_close();
    }
    else if (strcmp(segtype, "phn") == 0) {
	segdmp_set_bufsz(cmd_ln_int32("-cachesz"));
	
	cnt_phn(mdef, lex, &n_seg, &n_frame);

	if (segdmp_open_write(cmd_ln_str_list("-segdmpdirs"),
			      cmd_ln_str("-segdmpfn"),
			      cmd_ln_str("-segidxfn"),
			      acmod_set_n_acmod(mdef->acmod_set),
			      n_seg,
			      n_frame,
			      dmp_type,
			      feat_dimension1(feat),
			      feat_stream_lengths(feat),
			      feat_dimension(feat)) != S3_SUCCESS) {
	    E_FATAL("Unable to initialize segment dump\n");
	}
	
	if (agg_phn_seg(lex, mdef->acmod_set, feat, dmp_type) != S3_SUCCESS) {
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
