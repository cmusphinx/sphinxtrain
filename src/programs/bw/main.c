/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 *	This is the top level routine for SPHINX-III Baum-Welch
 *	reestimation.
 * 
 * Author: 
 *	Eric Thayer (eht+@cmu.edu) 20-Jun-95
 * 
 *********************************************************************/

#include "train_cmd_ln.h"
#include "forward.h"
#include "viterbi.h"
#include "next_utt_states.h"
#include "baum_welch.h"
#include "accum.h"

/* The SPHINX-III common library */
#include <s3/common.h>
#include <s3/prefetch.h>
#include <s3/profile.h>
#include <s3/ckd_alloc.h>
#include <s3/get_host_name.h>
#include <s3/mk_wordlist.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>
#include <s3/mk_sseq.h>
#include <s3/mk_trans_seq.h>
#include <s3/silcomp.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/mllr.h>
#include <s3/mllr_io.h>
#include <s3/ts2cb.h>
#include <s3/s3cb2mllr_io.h>

#include <s3/feat.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>

#include <sys_compat/misc.h>
#include <sys_compat/time.h>
#include <sys_compat/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define DUMP_RETRY_PERIOD	3	/* If a count dump fails, retry every # of sec's */

/*********************************************************************
 *
 * Function: 
 *	main_initialize
 * 
 * Description: 
 *	Construct data structures and precompute values necessary
 *	for Baum-Welch reestimation.
 *
 * Function Inputs: 
 * 	- argc
 *		The number of command line arguments
 *	- argv
 *		Array of command line argument strings
 *	- out_inv
 *		The model inventory data structure created
 *		by this routine.  (see libmodinv/modinv.c)
 *	- lex
 *		A word -> phone dictionary for the training set.
 *
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	- S3_SUCCESS
 *		This value is returned when no error condition
 *		has been detected.
 *	- S3_ERROR
 *		This value is returned when an error condition
 *		has been detected.
 * 
 * Global Outputs: 
 *	None
 *
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 *
 *********************************************************************/

static int
main_initialize(int argc,
		char *argv[],
		model_inventory_t **out_inv,
		lexicon_t **out_lex,
		model_def_t **out_mdef,
		float32 *****out_sxfrm_ainv,	/* per-"speaker" MLLR transform (A^(-1) of Ax + B)*/
		float32 ****out_sxfrm_b)	/* per-"speaker" MLLR transform (B of Ax + B)*/
{
    model_inventory_t *inv;	/* the model inventory */
    lexicon_t *lex;		/* the lexicon to be returned to the caller */
    const char *filler_dict;	/* the file name of a noise word dictionary */
    model_def_t *mdef;
    uint32 n_map;
    uint32 n_ts;
    uint32 n_cb;
    uint32 n_mllr;
    int mixw_reest;
    int tmat_reest;
    int mean_reest;
    int var_reest;
    int sil_del;
    int mllr_mult;
    int mllr_add;
    int did_restore = FALSE;
    const char *fn;
    char* silence_str;
    float32 ****sxfrm_a = NULL;
    float32 ****sxfrm_ainv = NULL;
    float32 ***sxfrm_b = NULL;
    
    E_INFO("Compiled on %s at %s\n", __DATE__, __TIME__);

    /* define, parse and (partially) validate the command line */
    train_cmd_ln_parse(argc, argv);

    if (cmd_ln_access("-feat") != NULL) {
	feat_set(cmd_ln_access("-feat"));
    }
    else {
	E_FATAL("You need to set a feature extraction config using -feat\n");
    }

    /* create a new model inventory structure */
    *out_inv = inv = mod_inv_new();

    mod_inv_set_n_feat(inv, feat_n_stream());

    E_INFO("Reading %s\n", cmd_ln_access("-moddeffn"));
    
    /* Read in the model definitions.  Defines the set of
       CI phones and context dependent phones.  Defines the
       transition matrix tying and state level tying. */
    if (model_def_read(&mdef,
		       cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	return S3_ERROR;
    }

    *out_mdef = mdef;

    fn = cmd_ln_access("-ts2cbfn");
    if (fn == NULL) {
	E_FATAL("Specify -ts2cbfn\n");
    }
    if (strcmp(fn, SEMI_LABEL) == 0) {
	mdef->cb = semi_ts2cb(mdef->n_tied_state);
	n_ts = mdef->n_tied_state;
	n_cb = 1;
    }
    else if (strcmp(fn, CONT_LABEL) == 0) {
	mdef->cb = cont_ts2cb(mdef->n_tied_state);
	n_ts = mdef->n_tied_state;
	n_cb = mdef->n_tied_state;
    }
    else if (s3ts2cb_read(fn,
			  &mdef->cb,
			  &n_ts,
			  &n_cb) != S3_SUCCESS) {
	return S3_ERROR;
    }

    inv->acmod_set = mdef->acmod_set;
    inv->mdef = mdef;

    if (mod_inv_read_mixw(inv, mdef, cmd_ln_access("-mixwfn"),
			  *(float32 *)cmd_ln_access("-mwfloor")) != S3_SUCCESS)
	return S3_ERROR;
    
    if (n_ts != inv->n_mixw) {
	E_WARN("%u mappings from tied-state to cb, but %u tied-state in %s\n",
	       mdef->n_cb, inv->n_mixw, cmd_ln_access("-mixwfn"));
    }

    if (mod_inv_read_tmat(inv,
			  cmd_ln_access("-tmatfn"),
			  *(float32 *)cmd_ln_access("-tpfloor")) != S3_SUCCESS)
	return S3_ERROR;

    if (mod_inv_read_gauden(inv,
			    cmd_ln_access("-meanfn"),
			    cmd_ln_access("-varfn"),
			    *(float32 *)cmd_ln_access("-varfloor"),
			    *(int32 *)cmd_ln_access("-topn")) != S3_SUCCESS)
	return S3_ERROR;
    
    if (gauden_eval_precomp(inv->gauden) != S3_SUCCESS) {
	E_ERROR("Problems precomputing values used during Gaussian density evaluation\n");

	return S3_ERROR;
    }

    if (inv->gauden->n_mgau != n_cb) {
	printf("# of codebooks in mean/var files, %u, inconsistent with ts2cb mapping %u\n", inv->gauden->n_mgau, n_cb);
    }

    fn = cmd_ln_access("-cb2mllrfn");
    if (fn != NULL) {
	if (strcmp(fn, ".1cls.") == 0) {
	    inv->gauden->mllr_idx = ckd_calloc(inv->gauden->n_mgau, sizeof(int32));
	    n_mllr = 1;
	    n_map = inv->gauden->n_mgau;
	}
	else if (s3cb2mllr_read((const char *)cmd_ln_access("-cb2mllrfn"),
				&inv->gauden->mllr_idx,
				&n_map,
				&n_mllr) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	if (n_map != inv->gauden->n_mgau) {
	    E_FATAL("cb2mllr maps %u cb, but read %u cb from files\n",
		    n_map, inv->gauden->n_mgau);
	}

	inv->gauden->n_mllr_class = n_mllr;
    }
	

    mixw_reest = *(int32 *)cmd_ln_access("-mixwreest");
    mean_reest = *(int32 *)cmd_ln_access("-meanreest");
    var_reest  = *(int32 *)cmd_ln_access("-varreest");
    tmat_reest = *(int32 *)cmd_ln_access("-tmatreest");
    mllr_mult  = *(int32 *)cmd_ln_access("-mllrmult");
    mllr_add   = *(int32 *)cmd_ln_access("-mllradd");
    sil_del    = *(int32 *)cmd_ln_access("-sildel");

    E_INFO("Will %sreestimate mixing weights.\n",
	   (mixw_reest ? "" : "NOT "));
    E_INFO("Will %sreestimate means.\n",
	   (mean_reest ? "" : "NOT "));
    E_INFO("Will %sreestimate variances.\n",
	   (var_reest ? "" : "NOT "));
    E_INFO("Will %sreestimate MLLR multiplicative term.\n",
	   (mllr_mult ? "" : "NOT "));
    E_INFO("Will %sreestimate MLLR additive term.\n",
	   (mllr_add ? "" : "NOT "));
    E_INFO("WIll %soptionally delete silence in Baum Welch or Viterbi. \n",
	   (sil_del ? "" : "NOT "));

    if (*(int32 *)cmd_ln_access("-mixwreest")) {
        if (mod_inv_alloc_mixw_acc(inv) != S3_SUCCESS)
	    return S3_ERROR;
    }

    E_INFO("Will %sreestimate transition matrices\n",
	   (*(int32 *)cmd_ln_access("-tmatreest") ? "" : "NOT "));
    if (*(int32 *)cmd_ln_access("-tmatreest")) {
	if (mod_inv_alloc_tmat_acc(inv) != S3_SUCCESS)
	    return S3_ERROR;
    }

    if (*(int32 *)cmd_ln_access("-meanreest") ||
	*(int32 *)cmd_ln_access("-varreest") ||
	*(int32 *)cmd_ln_access("-mllrmult") ||
	*(int32 *)cmd_ln_access("-mllradd")) {
	if (mod_inv_alloc_gauden_acc(inv) != S3_SUCCESS)
	    return S3_ERROR;
    }

    E_INFO("Reading main lexicon: %s\n",
	   cmd_ln_access("-dictfn"));

    lex = lexicon_read(NULL,
		       cmd_ln_access("-dictfn"),
		       mdef->acmod_set);
    if (lex == NULL)
	return S3_ERROR;
    if (cmd_ln_int32("-ltsoov"))
	lex->lts_rules = (lts_t *)&cmu6_lts_rules;
    
    filler_dict = cmd_ln_access("-fdictfn");
    if (filler_dict) {
	E_INFO("Reading filler lexicon: %s\n",
	       filler_dict);
	(void)lexicon_read(lex,
			   filler_dict,
			   mdef->acmod_set);
    }

    *out_lex = lex;


    /*
     * Configure corpus module (controls sequencing/access of per utterance data)
     */

    /* set the data directory and extension for cepstrum files */
    corpus_set_mfcc_dir(cmd_ln_access("-cepdir"));
    corpus_set_mfcc_ext(cmd_ln_access("-cepext"));


    /* The parameter required for re-estimation routines*/
    silence_str = (char *)cmd_ln_access("-siltag");
    E_INFO("Silence Tag %s\n",silence_str);

    if (*(int32 *)cmd_ln_access("-viterbi")) {
	if (cmd_ln_access("-segdir") == NULL) {
	    E_FATAL("Specify -segdir if -viterbi is true\n");
	}
	corpus_set_seg_dir(cmd_ln_access("-segdir"));
	corpus_set_seg_ext(cmd_ln_access("-segext"));
    }

    if (cmd_ln_access("-lsnfn")) {
	/* use a LSN file which has all the transcripts */
	corpus_set_lsn_filename(cmd_ln_access("-lsnfn"));
    }
    else {
	/* set the data directory and extension for word transcript
	   files */
	corpus_set_sent_dir(cmd_ln_access("-sentdir"));
	corpus_set_sent_ext(cmd_ln_access("-sentext"));
    }

    if (cmd_ln_access("-ctlfn")) {
	corpus_set_ctl_filename(cmd_ln_access("-ctlfn"));
    }

    if (cmd_ln_access("-sildelfn")) {
	corpus_set_sildel_filename((const char *)cmd_ln_access("-sildelfn"));
    }

    if (cmd_ln_access("-accumdir")) {
	char fn[MAXPATHLEN+1];
	FILE *fp;

	sprintf(fn, "%s/ckpt", (const char *)cmd_ln_access("-accumdir"));
	
	fp = fopen(fn, "r");
	if (fp != NULL) {
	    fclose(fp);

	    E_INFO("RESTORING CHECKPOINTED COUNTS IN %s\n", cmd_ln_access("-accumdir"));
	    
	    if (mod_inv_restore_acc(inv,
				    (const char *)cmd_ln_access("-accumdir"),
				    mixw_reest,
				    mean_reest,
				    var_reest,
				    tmat_reest,
				    mllr_mult,
				    mllr_add) != S3_SUCCESS) {
		E_FATAL("Unable to restore checkpoint information\n");
	    }

	    if (corpus_ckpt_set_interval(fn) != S3_SUCCESS) {
		E_FATAL("Unable to restore corpus state information\n");
	    }
	    
	    E_INFO("Resuming at utt %u\n", corpus_get_begin());
	    did_restore = TRUE;
	}
    }

    if (!did_restore) {
	if (cmd_ln_access("-part") && cmd_ln_access("-npart")) {
	    corpus_set_partition(*(uint32 *)cmd_ln_access("-part"),
				 *(uint32 *)cmd_ln_access("-npart"));
	}
	else if (cmd_ln_access("-nskip") && cmd_ln_access("-runlen")) {
	    corpus_set_interval(*(uint32 *)cmd_ln_access("-nskip"),
				*(uint32 *)cmd_ln_access("-runlen"));
	}
    }

    /* BEWARE: this function call must be done after all the other corpus
       configuration */
    corpus_init();

    if (cmd_ln_access("-spkrxfrm")) {
	const uint32 *tmp_veclen, *feat_veclen;
	uint32 tmp_n_mllrcls;
	uint32 tmp_n_stream;
	uint32 m, j;

	if (read_reg_mat(cmd_ln_access("-spkrxfrm"),
			 &tmp_veclen,
			 &tmp_n_mllrcls,
			 &tmp_n_stream,
			 &sxfrm_a, &sxfrm_b) != S3_SUCCESS) {
	    E_FATAL("Unable to read %s\n", cmd_ln_access("-spkrxfrm"));
	}

	if (feat_n_stream() != tmp_n_stream) {
	    E_FATAL("# feature streams in -spkrxfrm %s != # feature streams configured on cmd ln\n");
	}
	
	feat_veclen = feat_vecsize();
	for (j = 0; j < tmp_n_stream; j++) {
	    if (feat_veclen[j] != tmp_veclen[j]) {
		E_FATAL("# components of stream %u in -spkrxfrm inconsistent w/ -feat config (%u != %u)\n",
			j, tmp_veclen[j], feat_veclen[j]);
	    }
	}
	
	/* Compute A^(-1) for speaker transform */
	sxfrm_ainv = (float32 ****)ckd_calloc_2d(tmp_n_mllrcls, tmp_n_stream, sizeof(float32 **));
	for (m = 0; m < tmp_n_mllrcls; m++) {
	    for (j = 0; j < tmp_n_stream; j++) {
		sxfrm_ainv[m][j] = (float32 **)ckd_calloc_2d(tmp_veclen[j], tmp_veclen[j], sizeof(float32));
		
		invert(sxfrm_ainv[m][j], sxfrm_a[m][j], tmp_veclen[j]);
	    }
	}

	/* Free A since it is not needed now */
	for (m = 0; m < tmp_n_mllrcls; m++) {
	    for (j = 0; j < tmp_n_stream; j++) {
		ckd_free_2d((void **)sxfrm_a[m][j]);
	    }
	}

	ckd_free_2d((void **)sxfrm_a);
	ckd_free((void *)tmp_veclen);
    }

    *out_sxfrm_ainv = sxfrm_ainv;

    *out_sxfrm_b = sxfrm_b;

    return S3_SUCCESS;
}

void
main_reestimate(model_inventory_t *inv,
		lexicon_t *lex,
		model_def_t *mdef,
		float32 ****spkr_xfrm_ainv,
		float32 ***spkr_xfrm_b,
		int32 viterbi)
{
    vector_t *mfcc;	/* utterance cepstra */	
    uint32 n_frame;	/* # of cepstrum frames  */
    uint32 svd_n_frame;	/* # of cepstrum frames  */
    uint32 mfc_veclen;	/* # of MFC coefficients per frame */
    vector_t **f;		/* independent feature streams derived
				 * from cepstra */
    state_t *state_seq;		/* sentence HMM state sequence for the
				   utterance */
    uint32 n_state = 0;	/* # of sentence HMM states */
    float64 total_log_lik;	/* total log liklihood over corpus */
    float64 log_lik;		/* log liklihood for an utterance */
    uint32 total_frames;	/* # of frames over the corpus */
    float64 a_beam;		/* alpha pruning beam */
    float64 b_beam;		/* beta pruning beam */
    float32 spthresh;		/* state posterior probability threshold */
    uint32 seq_no;	/* sequence # of utterance in corpus */
    uint32 mixw_reest;	/* if TRUE, reestimate mixing weights */
    uint32 tmat_reest;	/* if TRUE, reestimate transition probability matrices */
    uint32 mean_reest;	/* if TRUE, reestimate means */
    uint32 var_reest;	/* if TRUE, reestimate variances */
    uint32 mllr_mult;  /* if TRUE estimate multiplicative term of MLLR */
    uint32 mllr_add;   /* if TRUE estimate additive term of MLLR */
    uint32 sil_del;    /* if TRUE optionally delete silence at the end */
    char *trans;
    char* silence_str;
    uint32 in_veclen;
    timing_t *utt_timer = NULL;
    timing_t *upd_timer = NULL;
    timing_t *fwd_timer = NULL;
    timing_t *bwd_timer = NULL;
    timing_t *gau_timer = NULL;
    timing_t *rsts_timer = NULL;
    timing_t *rstf_timer = NULL;
    timing_t *rstu_timer = NULL;

    int32 profile;

    int32 pass2var;

    uint32 n_utt;

    uint16 *seg;
    uint32 n_seg;

    uint32 *del_sf;
    uint32 *del_ef;
    uint32 n_del;

    uint32 maxuttlen;
    uint32 n_frame_skipped = 0;

    uint32 ckpt_intv = 0;
    uint32 no_retries=0;

    uint32 outputfullpath=0;
    uint32 fullsuffixmatch=0;

    E_INFO("Reestimation: %s\n",
	   (viterbi ? "Viterbi" : "Baum-Welch"));
    
    profile = *(int32 *)cmd_ln_access("-timing");
    if (profile) {
        E_INFO("Generating profiling information consumes significant CPU resources.");
	E_INFO("If you are not interested in profiling, use -timing no");
    }
    outputfullpath = *(int32 *)cmd_ln_access("-outputfullpath");
    fullsuffixmatch = *(int32 *)cmd_ln_access("-fullsuffixmatch");

    corpus_set_full_suffix_match(fullsuffixmatch);

    if (profile) {
	utt_timer  = timing_new();
	upd_timer   = timing_new();
	fwd_timer = timing_new();
	bwd_timer = timing_new();
	gau_timer = timing_new();
	rsts_timer = timing_new();
	rstf_timer = timing_new();
	rstu_timer = timing_new();

	/* bind some timers to names so that lower level routines
	 * can get at them easily */
	timing_bind_name("fwd", fwd_timer);
	timing_bind_name("bwd", bwd_timer);
	timing_bind_name("gau", gau_timer);
	timing_bind_name("rsts", rsts_timer);
	timing_bind_name("rstf", rstf_timer);
	timing_bind_name("rstu", rstu_timer);
    }

    mixw_reest = *(int32 *)cmd_ln_access("-mixwreest");
    tmat_reest = *(int32 *)cmd_ln_access("-tmatreest");
    mean_reest = *(int32 *)cmd_ln_access("-meanreest");
    var_reest = *(int32 *)cmd_ln_access("-varreest");
    pass2var = *(int32 *)cmd_ln_access("-2passvar");
    mllr_mult = *(int32 *)cmd_ln_access("-mllrmult");
    mllr_add = *(int32 *)cmd_ln_access("-mllradd");
    sil_del    = *(int32 *)cmd_ln_access("-sildel");
    silence_str = (char *)cmd_ln_access("-siltag");

    if (cmd_ln_access("-ckptintv")) {
	ckpt_intv = *(int32 *)cmd_ln_access("-ckptintv");
    }

    if ((mllr_mult || mllr_add) && (inv->gauden->mllr_idx == NULL)) {
	E_FATAL("Specify MLLR class map using -cb2mllrfn\n");
    }

    if (cmd_ln_access("-ceplen") == NULL) {
	E_FATAL("Input vector length must be specified\n");
    }
	
    in_veclen = *(int32 *)cmd_ln_access("-ceplen");

    feat_set_in_veclen(in_veclen);

    if (cmd_ln_access("-accumdir") == NULL) {
	E_WARN("NO ACCUMDIR SET.  No counts will be written; assuming debug\n");
    }

    if (!mixw_reest && !tmat_reest && !mean_reest && !var_reest &&
	(!mllr_mult && !mllr_add)) {
	E_WARN("No reestimation specified!  None done.\n");
	
	return;
    }

    total_log_lik = 0;
    total_frames = 0;

    a_beam = *(float64 *)cmd_ln_access("-abeam");
    b_beam = *(float64 *)cmd_ln_access("-bbeam");
    spthresh = *(float32 *)cmd_ln_access("-spthresh");
    maxuttlen = *(int32 *)cmd_ln_access("-maxuttlen");

    /* Begin by skipping over some (possibly zero) # of utterances.
     * Continue to process utterances until there are no more (either EOF
     * or end of run). */

    seq_no = corpus_get_begin();

    printf("column defns\n");
    printf("\t<seq>\n");
    printf("\t<id>\n");
    printf("\t<n_frame_in>\n");
    printf("\t<n_frame_del>\n");
    printf("\t<n_state_shmm>\n");
    printf("\t<avg_states_alpha>\n");
    printf("\t<avg_states_beta>\n");
    printf("\t<avg_states_reest>\n");
    printf("\t<avg_posterior_prune>\n");
    printf("\t<frame_log_lik>\n");
    printf("\t<utt_log_lik>\n");
    printf("\t... timing info ... \n");

    n_utt = 0;
    while (corpus_next_utt()) {
	/* Zero timers before utt processing begins */
	if (utt_timer) {
	    timing_reset(utt_timer);
	}
	if (upd_timer) {
	    timing_reset(upd_timer);
	}
	if (fwd_timer) {
	    timing_reset(fwd_timer);
	}
	if (bwd_timer) {
	    timing_reset(bwd_timer);
	}
	if (gau_timer) {
	    timing_reset(gau_timer);
	}
	if (rsts_timer) {
	    timing_reset(rsts_timer);
	}
	if (rstf_timer) {
	    timing_reset(rstf_timer);
	}
	if (rstu_timer) {
	    timing_reset(rstu_timer);
	}

	if (utt_timer) {
	    timing_start(utt_timer);
	}

	printf("utt> %5u %25s", 
	       seq_no,
	       (outputfullpath ? corpus_utt_full_name() : corpus_utt()));

	/* get the MFCC data for the utterance */
/* CHANGE BY BHIKSHA; IF INPUT VECLEN != 13, THEN DO NOT USE THE
   REGULAR corpus_get_mfcc() WHICH REQUIRES INPUT DATA TO BE 13 DIMENSIONAL
   CEPSTRA. USE, INSTEAD, THE HACKED VERSION corpus_get_generic_featurevec()
   WHICH TAKES FEATURES OF ARBITRARY LENGTH
   7 JAN 1998 */
        if (in_veclen == S2_CEP_VECLEN) {
	    if (corpus_get_mfcc(&mfcc, &n_frame, &mfc_veclen) < 0) {
	        E_FATAL("Can't read input features\n");
	    }
	    assert(mfc_veclen == in_veclen);
        }
        else {
	    if (corpus_get_generic_featurevec(&mfcc, &n_frame, in_veclen) < 0) {
	        E_FATAL("Can't read input features\n");
	    }
        }

/* END CHANGES BY BHIKSHA */


	printf(" %4u", n_frame);

	if (n_frame < 9) {
	    E_WARN("utt %s too short\n", corpus_utt());
	    if (mfcc) {
		ckd_free(mfcc[0]);
		ckd_free(mfcc);
	    }
	    continue;
	}

	if ((maxuttlen > 0) && (n_frame > maxuttlen)) {
	    E_INFO("utt # frames > -maxuttlen; skipping\n");
	    n_frame_skipped += n_frame;
	    if (mfcc) {
		ckd_free(mfcc[0]);
		ckd_free(mfcc);
	    }

	    continue;
	}

	corpus_get_sildel(&del_sf, &del_ef, &n_del);
	silcomp_set_del_seg(del_sf, del_ef, n_del);

	svd_n_frame = n_frame;
	/* compute feature vectors from the MFCC data */
	f = feat_compute(mfcc, &n_frame);

	printf(" %4u", n_frame - svd_n_frame);

	/* Get the transcript */
	corpus_get_sent(&trans);

	if (upd_timer)
	    timing_start(upd_timer);
	if (!viterbi) {
	    /* create a sentence HMM */
	    state_seq = next_utt_states(&n_state, lex, inv, mdef, trans, sil_del, silence_str);
	    printf(" %5u", n_state);

	    /* accumulate reestimation sums for the utterance */

	    if (baum_welch_update(&log_lik,
				  f, n_frame,
				  spkr_xfrm_ainv,
				  spkr_xfrm_b,
				  state_seq, n_state,
				  inv,
				  a_beam,
				  b_beam,
				  spthresh,
				  mixw_reest,
				  tmat_reest,
				  mean_reest,
				  var_reest,
				  pass2var,
				  mllr_mult,
				  mllr_add
				  ) == S3_SUCCESS) {
		total_frames += n_frame;
		total_log_lik += log_lik;
		
		printf(" %e %e",
		       (n_frame > 0 ? log_lik / n_frame : 0.0),
		       log_lik);
	    }

	}
	else {
	    char **word;
	    uint32 n_word;
	    acmod_id_t *phone;
	    uint32 n_phone;
	    char *btw_mark;
	    uint32 *s_seq;
	    uint32 *t_seq;
	    uint32 *ms_seq;

	    corpus_get_seg(&seg, &n_seg);
	    assert(n_seg == n_frame);

	    word = mk_wordlist(trans, &n_word);
	    phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);
	    cvt2triphone(inv->mdef->acmod_set, phone, btw_mark, n_phone);
	    s_seq = mk_sseq(seg, n_frame, phone, n_phone, inv->mdef);
	    mk_trans_seq(&t_seq, &ms_seq, seg, n_frame, phone, n_phone, inv->mdef);

	    ckd_free(seg);
	    ckd_free(word);
	    ckd_free(phone);
	    ckd_free(btw_mark);

	    /* create a tied state sequence from the state segmentation */
	    
	    if (viterbi_update(&log_lik,
			       f,
			       s_seq,
			       t_seq,
			       ms_seq,
			       n_frame,
			       inv,
			       mixw_reest,
			       tmat_reest,
			       mean_reest,
			       var_reest,
			       pass2var) == S3_SUCCESS) {
		total_frames += n_frame;
		total_log_lik += log_lik;
		printf(" %e", log_lik);
	    }
	    ckd_free(s_seq);
	    ckd_free(t_seq);
	    ckd_free(ms_seq);
	}
	if (upd_timer)
	    timing_stop(upd_timer);

	free(mfcc[0]);
	ckd_free(mfcc);
	feat_free(f);
	free(trans);	/* alloc'ed using strdup() */

	seq_no++;

	if (utt_timer)
	    timing_stop(utt_timer);
	
	if (profile) {
	    printf(" utt %4.3fx %4.3fe"
		   " upd %4.3fx %4.3fe"
		   " fwd %4.3fx %4.3fe"
		   " bwd %4.3fx %4.3fe"
		   " gau %4.3fx %4.3fe"
		   " rsts %4.3fx %4.3fe"
		   " rstf %4.3fx %4.3fe"
		   " rstu %4.3fx %4.3fe",
		   
		   utt_timer->t_cpu/(n_frame*0.01),
		   (utt_timer->t_cpu > 0 ? utt_timer->t_elapsed / utt_timer->t_cpu : 0.0),

		   upd_timer->t_cpu/(n_frame*0.01),
		   (upd_timer->t_cpu > 0 ? upd_timer->t_elapsed / upd_timer->t_cpu : 0.0),

		   fwd_timer->t_cpu/(n_frame*0.01),
		   (fwd_timer->t_cpu > 0 ? fwd_timer->t_elapsed / fwd_timer->t_cpu : 0.0),

		   bwd_timer->t_cpu/(n_frame*0.01),
		   (bwd_timer->t_cpu > 0 ? bwd_timer->t_elapsed / bwd_timer->t_cpu : 0.0),

		   gau_timer->t_cpu/(n_frame*0.01),
		   (gau_timer->t_cpu > 0 ? gau_timer->t_elapsed / gau_timer->t_cpu : 0.0),

		   rsts_timer->t_cpu/(n_frame*0.01),
		   (rsts_timer->t_cpu > 0 ? rsts_timer->t_elapsed / rsts_timer->t_cpu : 0.0),

		   rstf_timer->t_cpu/(n_frame*0.01),
		   (rstf_timer->t_cpu > 0 ? rstf_timer->t_elapsed / rstf_timer->t_cpu : 0.0),

		   rstu_timer->t_cpu/(n_frame*0.01),
		   (rstu_timer->t_cpu > 0 ? rstu_timer->t_elapsed / rstu_timer->t_cpu : 0.0));
	}
	printf("\n");
	fflush(stdout);

	++n_utt;

	if ((ckpt_intv > 0) &&
	    ((n_utt % ckpt_intv) == 0) &&
	    (cmd_ln_access("-accumdir") != NULL)) {
	    while (accum_dump(cmd_ln_access("-accumdir"),
			      inv,
			      mixw_reest,
			      tmat_reest,
			      mean_reest,
			      var_reest,
			      pass2var,
			      mllr_mult,
			      mllr_add,
			      TRUE) != S3_SUCCESS) {
		static int notified = FALSE;
		time_t t;
		char time_str[64];
		
		/*
		 * If we were not able to dump the parameters, write one log entry
		 * about the failure
		 */
		if (notified == FALSE) {
		    t = time(NULL);
		    strcpy(time_str, (const char *)ctime((const time_t *)&t));
		    /* nuke the newline at the end of this. */
		    time_str[strlen(time_str)-1] = '\0';
		    E_WARN("Ckpt count dump failed on %s.  Retrying dump every %3.1f hour until success.\n",
			   time_str, DUMP_RETRY_PERIOD/3600.0);
		    
		    notified = TRUE;
		    no_retries++;
		    if(no_retries>10){ 
		      E_FATAL("Failed to get the files after 10 retries(about 5 minutes).\n ");
		    }
		}
		sleep(DUMP_RETRY_PERIOD);

	    }
	}
    }
    
    printf("overall> %s %u (-%u) %e %e",
	   get_host_name(),
	   total_frames,
	   n_frame_skipped,
	   (total_frames > 0 ? total_log_lik / total_frames : 0.0),
	   total_log_lik);
    if (profile) {
	printf(" %4.3fx %4.3fe",
	       (total_frames > 0 ? utt_timer->t_tot_cpu/(total_frames*0.01) : 0.0),
	       (utt_timer->t_tot_cpu > 0 ? utt_timer->t_tot_elapsed / utt_timer->t_tot_cpu : 0.0));
    }
    
    printf("\n");
    
    fflush(stdout);

    no_retries=0;
    /* dump the accumulators to a file system */
    while (cmd_ln_access("-accumdir") != NULL &&
	   accum_dump(cmd_ln_access("-accumdir"), inv,
		      mixw_reest,
		      tmat_reest,
		      mean_reest,
		      var_reest,
		      pass2var,
		      mllr_mult,
		      mllr_add,
		      FALSE) != S3_SUCCESS) {
	static int notified = FALSE;
	time_t t;
	char time_str[64];

	/*
	 * If we were not able to dump the parameters, write one log entry
	 * about the failure
	 */
	if (notified == FALSE) {
	    t = time(NULL);
	    strcpy(time_str, (const char *)ctime((const time_t *)&t));
	    /* nuke the newline at the end of this. */
	    time_str[strlen(time_str)-1] = '\0';
	    E_WARN("Count dump failed on %s.  Retrying dump every %3.1f hour until success.\n",
		   time_str, DUMP_RETRY_PERIOD/3600.0);

	    notified = TRUE;
	    no_retries++;
	    if(no_retries>10){ 
	      E_FATAL("Failed to get the files after 10 retries(about 5 minutes).\n ");
	    }
	}
	
	sleep(DUMP_RETRY_PERIOD);


    }

    /* Write a log entry on success */
    if (cmd_ln_access("-accumdir"))
	E_INFO("Counts saved to %s\n", cmd_ln_access("-accumdir"));
    else
	E_INFO("Counts NOT saved.\n");

    mod_inv_free(inv);
}

int main(int argc, char *argv[])
{
    model_inventory_t *inv;
    lexicon_t *lex;
    model_def_t *mdef;
    float32 ****spkr_xfrm_ainv;
    float32 ***spkr_xfrm_b;
    
    (void) prefetch_init();	/* should do this BEFORE any allocations */

    if (main_initialize(argc, argv,
			&inv, &lex, &mdef, &spkr_xfrm_ainv, &spkr_xfrm_b) != S3_SUCCESS) {
	E_FATAL("initialization failed\n");
    }

    main_reestimate(inv, lex, mdef, spkr_xfrm_ainv, spkr_xfrm_b, *(int32 *)cmd_ln_access("-viterbi"));
		    
    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.14  2006/02/24  15:50:23  eht
 * Output an informational message to the log that collecting profiling
 * information about bw consumes significant CPU resources and suggest
 * using -timing no if profiling isn't needed.
 * 
 * Revision 1.13  2006/02/23 22:21:29  eht
 * add -outputfullpath and -fullsuffixmatch arguments to bw.
 *
 * Default behavior is to keep the existing system behavior when the
 * corpus module tries to match the transcript utterance id with the
 * partial path contained in the control file.
 *
 * Using -fullsuffixmatch yes will do the following:
 * 	The corpus module will check whether the string contained
 * 	inside parentheses in the transcript for the utterances
 * 	matches the final part of the control file partial path
 * 	for the utterance.  For instance, if the control file
 * 	partial path is:
 * 		tidigits/train/man/ae/243za
 * 	the following strings will be considered to match:
 * 		243za
 * 		ae/243za
 * 		man/ae/243za
 * 		.
 * 		.
 * 		.
 * 	In any event, the utterance will be used by bw for training.
 * 	This switch just modifies when the warning message for
 * 	mismatching control file and transcripts is generated.
 *
 * Using -outputfullpath yes will output the entire subpath from the
 * control file in the log output of bw rather than just the final path
 * component.  This allows for simpler automatic processing of the output
 * of bw.
 *
 * Revision 1.12  2005/09/27 02:02:47  arthchan2003
 * Check whether utterance is too short in init_gau, bw and agg_seg.
 *
 * Revision 1.11  2005/09/15 19:36:00  dhdfu
 * Add (as yet untested) support for letter-to-sound rules (from CMU
 * Flite) when constructing sentence HMMs in Baum-Welch.  Currently only
 * rules for CMUdict exist.  Of course this is not a substitute for
 * actually checking pronunciations...
 *
 * Revision 1.10  2005/09/15 19:32:36  dhdfu
 * Another (meaningless) signedness fix
 *
 * Revision 1.9  2004/11/17 01:46:58  arthchan2003
 * Change the sleeping time to be at most 30 seconds. No one will know whether the code dies or not if keep the code loop infinitely.
 *
 * Revision 1.8  2004/07/22 00:08:39  egouvea
 * Fixed some compilation warnings.
 *
 * Revision 1.7  2004/07/21 18:30:33  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.6  2004/07/17 08:00:23  arthchan2003
 * deeply regretted about one function prototype, now revert to the state where multiple pronounciations code doesn't exist
 *
 * Revision 1.4  2004/06/17 19:17:14  arthchan2003
 * Code Update for silence deletion and standardize the name for command -line arguments
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
 * Revision 1.30  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.29  1996/08/06  14:04:13  eht
 * Silence deletion file implementation
 *
 * Revision 1.28  1996/07/29  16:16:44  eht
 * Wrap up more initialization functionality into mod_inv module
 * - MLLR reestimation
 * Bunch of relatively minor changes
 *
 * Revision 1.27  1996/03/26  13:54:51  eht
 * - Deal w/ case of n_top > n_density in a better way
 * - Fix bug of float32 beams
 * - Deal w/ 2d MFCC data rather than the old 1d form
 * - Add flag to control whether timing stats are printed
 * - Add feature that when '-accumdir' is not specified, no
 *   counts are written.  This allows debugging runs w/o the
 *   fear of overwriting data.  A warning is printed at initialization
 *   time that no '-accumdir' argument has been given.
 *
 * Revision 1.26  1996/03/04  15:58:36  eht
 * Added more CPU time counters
 *
 * Revision 1.25  1996/02/02  17:55:20  eht
 * *** empty log message ***
 *
 * Revision 1.24  1996/02/02  17:40:32  eht
 * Deal with alpha and beta beams.
 *
 * Revision 1.23  1996/01/26  18:23:49  eht
 * Deal w/ case when MFC file cannot be read.
 * Free comments w/ were not freed before.
 *
 * Revision 1.22  1995/12/15  18:37:07  eht
 * Added some type cases for memory alloc/free
 *
 * Revision 1.21  1995/12/14  19:47:59  eht
 * Added some sanity checks to prevent seg faults and weird behavior if
 * the user gives inconsistent input arguments.
 *
 * Revision 1.20  1995/12/01  03:58:22  eht
 * Fixed transcript core leak
 *
 * Revision 1.19  1995/11/30  20:46:44  eht
 * Added change to allow state parameter definitions to be used.
 * Added change to allow transition matrix reestimation to be turned off.
 *
 * Revision 1.18  1995/11/10  19:37:44  eht
 * Use new profile
 *
 * Revision 1.16  1995/10/18  11:18:38  eht
 * Include compatibility macros for Windows NT so that
 * sleep(x) is converted into Sleep(x * 1000)
 *
 * Revision 1.15  1995/10/17  14:02:26  eht
 * Changed so that would port to Windows NT
 *
 * Revision 1.14  1995/10/10  12:43:50  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.13  1995/10/09  14:55:33  eht
 * Change interface to new ckd_alloc routines
 *
 * Revision 1.12  1995/10/05  12:52:17  eht
 * Get rid of the U Toronto malloc package statements
 *
 * Revision 1.11  95/09/14  14:19:36  14:19:36  eht (Eric Thayer)
 * Added support for U Toronto debug malloc library
 * 
 * Revision 1.10  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.9  1995/09/07  18:53:03  eht
 * Get the seq number of the first utterance in a subcorpus
 * from the corpus module rather than the command line.  Allows
 * the corpus module to figure this out.  May eventually need
 * to call this for each utterance, but no need now.
 *
 * Revision 1.8  1995/08/29  12:25:26  eht
 * Updates to reflect new interface to corpus
 * configuration and initialization
 *
 * Revision 1.7  1995/08/24  19:58:50  eht
 * Merged in PWP's prefetching code
 *
 *
 * Revision 1.6  1995/08/24  19:49:43  eht
 * Upgrade to allow a single LSN file for the corpus
 *
 * Revision 1.5  1995/08/09  20:18:31  eht
 * Add output when mixing weight normalization fails
 *
 * Revision 1.4  1995/07/07  12:00:29  eht
 * Include initial mixing weights and transition probabilities
 * in verbose output.  Also, got rid of the last vestiges of
 * the tying DAG.  Also, added some arguments to state_seq_print
 * so that it could produce more informative output.
 *
 * Revision 1.3  1995/06/28  14:38:32  eht
 * Removed include of tying DAG header file
 *
 * Revision 1.2  1995/06/28  14:31:55  eht
 * Removed tying DAG creation.  Now, next_utt_states() builds state seq using
 * tying structure in a model_def_t data structure (see libio/model_def_io.c
 * for details).
 *
 * Revision 1.1  1995/06/02  20:39:40  eht
 * Initial revision
 *
 *
 */
