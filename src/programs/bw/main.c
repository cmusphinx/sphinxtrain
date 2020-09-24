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

#include <s3/common.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>
#include <s3/mk_sseq.h>
#include <s3/mk_trans_seq.h>
#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/mllr.h>
#include <s3/mllr_io.h>
#include <s3/ts2cb.h>
#include <s3/s3cb2mllr_io.h>
#include <sys_compat/misc.h>
#include <sys_compat/time.h>
#include <sys_compat/file.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/profile.h>
#include <sphinxbase/feat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define DUMP_RETRY_PERIOD	3	/* If a count dump fails, retry every # of sec's */

/* the following parameters are used for MMIE training */
#define LOG_ZERO	-1.0E10
static float32 lm_scale = 11.5;

/* FIXME: Should go in libutil */
char *
string_join(const char *base, ...)
{
    va_list args;
    size_t len;
    const char *c;
    char *out;

    va_start(args, base);
    len = strlen(base);
    while ((c = va_arg(args, const char *)) != NULL) {
        len += strlen(c);
    }
    len++;
    va_end(args);

    out = ckd_calloc(len, 1);
    va_start(args, base);
    strcpy(out, base);
    while ((c = va_arg(args, const char *)) != NULL) {
        strcat(out, c);
    }
    va_end(args);

    return out;
}

static void
print_all_timers(bw_timers_t *timers, int32 n_frame)
{
    printf(" utt %4.3fx %4.3fe"
	   " upd %4.3fx %4.3fe"
	   " fwd %4.3fx %4.3fe"
	   " bwd %4.3fx %4.3fe"
	   " gau %4.3fx %4.3fe"
	   " rsts %4.3fx %4.3fe"
	   " rstf %4.3fx %4.3fe"
	   " rstu %4.3fx %4.3fe",

	timers->utt_timer.t_cpu/(n_frame*0.01),
	(timers->utt_timer.t_cpu > 0 ? timers->utt_timer.t_elapsed / timers->utt_timer.t_cpu : 0.0),

	timers->upd_timer.t_cpu/(n_frame*0.01),
	(timers->upd_timer.t_cpu > 0 ? timers->upd_timer.t_elapsed / timers->upd_timer.t_cpu : 0.0),

	timers->fwd_timer.t_cpu/(n_frame*0.01),
	(timers->fwd_timer.t_cpu > 0 ? timers->fwd_timer.t_elapsed / timers->fwd_timer.t_cpu : 0.0),

	timers->bwd_timer.t_cpu/(n_frame*0.01),
	(timers->bwd_timer.t_cpu > 0 ? timers->bwd_timer.t_elapsed / timers->bwd_timer.t_cpu : 0.0),

	timers->gau_timer.t_cpu/(n_frame*0.01),
	(timers->gau_timer.t_cpu > 0 ? timers->gau_timer.t_elapsed / timers->gau_timer.t_cpu : 0.0),

	timers->rsts_timer.t_cpu/(n_frame*0.01),
	(timers->rsts_timer.t_cpu > 0 ? timers->rsts_timer.t_elapsed / timers->rsts_timer.t_cpu : 0.0),

	timers->rstf_timer.t_cpu/(n_frame*0.01),
	(timers->rstf_timer.t_cpu > 0 ? timers->rstf_timer.t_elapsed / timers->rstf_timer.t_cpu : 0.0),

	timers->rstu_timer.t_cpu/(n_frame*0.01),
	(timers->rstu_timer.t_cpu > 0 ? timers->rstu_timer.t_elapsed / timers->rstu_timer.t_cpu : 0.0));
    printf("\n");
}


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
		feat_t **out_feat)
{
    model_inventory_t *inv;	/* the model inventory */
    lexicon_t *lex;		/* the lexicon to be returned to the caller */
    model_def_t *mdef;
    feat_t *feat;
    uint32 n_map;
    uint32 n_ts;
    uint32 n_cb;
    uint32 n_mllr;
    int mixw_reest;
    int tmat_reest;
    int mean_reest;
    int var_reest;
    int did_restore = FALSE;
    const char *fn;
    int32 *mllr_idx = NULL;
    const char *hmmdir;
    const char *mdeffn, *meanfn, *varfn, *mixwfn, *tmatfn, *fdictfn;

    /* Note these are forward transforms for use
       in training.  The inverse transform of the accumulators is now
       done externally by mllr_transform. */
    float32 ****sxfrm_a = NULL;
    float32 ***sxfrm_b = NULL;

    E_INFO("Compiled on %s at %s\n", __DATE__, __TIME__);

    /* define, parse and (partially) validate the command line */
    train_cmd_ln_parse(argc, argv);

    feat =
        feat_init(cmd_ln_str("-feat"),
                  cmn_type_from_str(cmd_ln_str("-cmn")),
                  cmd_ln_boolean("-varnorm"),
                  agc_type_from_str(cmd_ln_str("-agc")),
                  1, cmd_ln_int32("-ceplen"));
    *out_feat = feat;


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


    /* create a new model inventory structure */
    *out_inv = inv = mod_inv_new();

    mod_inv_set_n_feat(inv, feat_dimension1(feat));

    mdeffn = cmd_ln_str("-moddeffn");
    meanfn = cmd_ln_str("-meanfn");
    varfn = cmd_ln_str("-varfn");
    mixwfn = cmd_ln_str("-mixwfn");
    tmatfn = cmd_ln_str("-tmatfn");
    fdictfn = cmd_ln_str("-fdictfn");

    /* Note: this will leak a small amount of memory but we really
     * don't care. */
    if ((hmmdir = cmd_ln_str("-hmmdir")) != NULL) {
	if (mdeffn == NULL)
	    mdeffn = string_join(hmmdir, "/mdef", NULL);
	if (meanfn == NULL)
	    meanfn = string_join(hmmdir, "/means", NULL);
	if (varfn == NULL)
	    varfn = string_join(hmmdir, "/variances", NULL);
	if (mixwfn == NULL)
	    mixwfn = string_join(hmmdir, "/mixture_weights", NULL);
	if (tmatfn == NULL)
	    tmatfn = string_join(hmmdir, "/transition_matrices", NULL);
	if (fdictfn == NULL)
	    fdictfn = string_join(hmmdir, "/noisedict", NULL);
    }
    E_INFO("Reading %s\n", mdeffn);
    
    /* Read in the model definitions.  Defines the set of
       CI phones and context dependent phones.  Defines the
       transition matrix tying and state level tying. */
    if (model_def_read(&mdef, mdeffn) != S3_SUCCESS) {
	return S3_ERROR;
    }

    *out_mdef = mdef;

    fn = cmd_ln_str("-ts2cbfn");
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
    else if (strcmp(fn, PTM_LABEL) == 0) {
      mdef->cb = ptm_ts2cb(mdef);
      n_ts = mdef->n_tied_state;
      n_cb = mdef->acmod_set->n_ci;
    }
    else if (s3ts2cb_read(fn,
			  &mdef->cb,
			  &n_ts,
			  &n_cb) != S3_SUCCESS) {
	return S3_ERROR;
    }

    inv->acmod_set = mdef->acmod_set;
    inv->mdef = mdef;

    if (mod_inv_read_mixw(inv, mdef, mixwfn,
			  cmd_ln_float32("-mwfloor")) != S3_SUCCESS)
	return S3_ERROR;
    
    if (n_ts != inv->n_mixw) {
	E_WARN("%u mappings from tied-state to cb, but %u tied-state in %s\n",
	       mdef->n_cb, inv->n_mixw, mixwfn);
    }

    if (mod_inv_read_tmat(inv, tmatfn,
			  cmd_ln_float32("-tpfloor")) != S3_SUCCESS)
	return S3_ERROR;

    if (mod_inv_read_gauden(inv, meanfn, varfn,
			    cmd_ln_float32("-varfloor"),
			    cmd_ln_int32("-topn"),
			    cmd_ln_int32("-fullvar")) != S3_SUCCESS) {
	    if (!cmd_ln_int32("-fullvar")) {
		    return S3_ERROR;
	    }
	    else {
		    /* If reading full variances failed, try reading
		     * them as diagonal variances (allows us to
		     * initialize full vars from diagonal ones) */
		    if (mod_inv_read_gauden(inv, meanfn, varfn,
					    cmd_ln_float32("-varfloor"),
					    cmd_ln_int32("-topn"),
					    FALSE) != S3_SUCCESS) {
			    return S3_ERROR;
		    }
	    }
	    
    }

    /* If we want to use diagonals only, and we didn't read diagonals
     * above, then we have to extract them here. */
    if (cmd_ln_int32("-diagfull") && inv->gauden->var == NULL) {
	    /* Extract diagonals and use them for Gaussian computation. */
	    gauden_t *g;
	    uint32 i, j, k, l;

	    g = inv->gauden;
	    g->var = gauden_alloc_param(g->n_mgau,
					g->n_feat,
					g->n_density,
					g->veclen);
	    for (i = 0; i < g->n_mgau; ++i)
		    for (j = 0; j < g->n_feat; ++j)
			    for (k = 0; k < g->n_density; ++k)
				    for (l = 0; l < g->veclen[j]; ++l)
					    g->var[i][j][k][l] =
						    g->fullvar[i][j][k][l][l];
	    gauden_free_param_full(g->fullvar);
	    g->fullvar = NULL;
	    gauden_floor_variance(g);
    }
    
    if (gauden_eval_precomp(inv->gauden) != S3_SUCCESS) {
	E_ERROR("Problems precomputing values used during Gaussian density evaluation\n");
	return S3_ERROR;
    }

    if (inv->gauden->n_mgau != n_cb) {
	E_ERROR("# of codebooks in mean/var files, %u, inconsistent with ts2cb mapping %u\n", inv->gauden->n_mgau, n_cb);
	return S3_ERROR;
    }

    mixw_reest = cmd_ln_int32("-mixwreest");
    mean_reest = cmd_ln_int32("-meanreest");
    var_reest  = cmd_ln_int32("-varreest");
    tmat_reest = cmd_ln_int32("-tmatreest");

    E_INFO("Will %sreestimate mixing weights.\n",
	   (mixw_reest ? "" : "NOT "));
    E_INFO("Will %sreestimate means.\n",
	   (mean_reest ? "" : "NOT "));
    E_INFO("Will %sreestimate variances.\n",
	   (var_reest ? "" : "NOT "));

    if (cmd_ln_int32("-mixwreest")) {
        if (mod_inv_alloc_mixw_acc(inv) != S3_SUCCESS)
	    return S3_ERROR;
    }

    E_INFO("Will %sreestimate transition matrices\n",
	   (cmd_ln_int32("-tmatreest") ? "" : "NOT "));
    if (cmd_ln_int32("-tmatreest")) {
	if (mod_inv_alloc_tmat_acc(inv) != S3_SUCCESS)
	    return S3_ERROR;
    }

    if (cmd_ln_int32("-meanreest") ||
	cmd_ln_int32("-varreest")) {
	if (mod_inv_alloc_gauden_acc(inv) != S3_SUCCESS)
	    return S3_ERROR;
    }

    E_INFO("Reading main dictionary: %s\n",
	   cmd_ln_str("-dictfn"));

    lex = lexicon_read(NULL,
		       cmd_ln_str("-dictfn"),
		       mdef->acmod_set);
    if (lex == NULL)
	return S3_ERROR;
    
    if (fdictfn) {
	E_INFO("Reading filler dictionary: %s\n",
	       fdictfn);
	(void)lexicon_read(lex,
			   fdictfn,
			   mdef->acmod_set);
    }

    *out_lex = lex;


    /*
     * Configure corpus module (controls sequencing/access of per utterance data)
     */

    /* set the data directory and extension for cepstrum files */
    corpus_set_mfcc_dir(cmd_ln_str("-cepdir"));
    corpus_set_mfcc_ext(cmd_ln_str("-cepext"));

    if (cmd_ln_str("-lsnfn")) {
	/* use a LSN file which has all the transcripts */
	corpus_set_lsn_filename(cmd_ln_str("-lsnfn"));
    }
    else {
	/* set the data directory and extension for word transcript
	   files */
	corpus_set_sent_dir(cmd_ln_str("-sentdir"));
	corpus_set_sent_ext(cmd_ln_str("-sentext"));
    }

    if (cmd_ln_str("-ctlfn")) {
	corpus_set_ctl_filename(cmd_ln_str("-ctlfn"));
    }

    if (cmd_ln_str("-phsegdir")) {
	    corpus_set_phseg_dir(cmd_ln_str("-phsegdir"));
	    corpus_set_phseg_ext(cmd_ln_str("-phsegext"));
    }

    if (cmd_ln_str("-accumdir")) {
	char fn[MAXPATHLEN+1];
	FILE *fp;

	sprintf(fn, "%s/ckpt", cmd_ln_str("-accumdir"));
	
	fp = fopen(fn, "r");
	if (fp != NULL) {
	    const uint32* feat_veclen;
	    fclose(fp);

	    E_INFO("RESTORING CHECKPOINTED COUNTS IN %s\n", cmd_ln_str("-accumdir"));
	    
    	    feat_veclen = (uint32 *)feat_stream_lengths(feat);
	    	    
	    if (mod_inv_restore_acc(inv,
				    cmd_ln_str("-accumdir"),
				    mixw_reest,
				    mean_reest,
				    var_reest,
				    tmat_reest,
				    feat_veclen) != S3_SUCCESS) {
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
	if (cmd_ln_int32("-nskip") && cmd_ln_int32("-runlen")) {
    	    corpus_set_interval(cmd_ln_int32("-nskip"),
			    cmd_ln_int32("-runlen"));
	} else if (cmd_ln_int32("-part") && cmd_ln_int32("-npart")) {
	    corpus_set_partition(cmd_ln_int32("-part"),
			     cmd_ln_int32("-npart"));
	}
    }

    /* BEWARE: this function call must be done after all the other corpus
       configuration */
    corpus_init();

    if (cmd_ln_str("-mllrmat")) {
	uint32 *tmp_veclen, *feat_veclen;
	uint32 tmp_n_mllrcls;
	uint32 tmp_n_stream;
	uint32 j;

	if (read_reg_mat(cmd_ln_str("-mllrmat"),
			 &tmp_veclen,
			 &tmp_n_mllrcls,
			 &tmp_n_stream,
			 &sxfrm_a, &sxfrm_b) != S3_SUCCESS) {
	    E_FATAL("Unable to read %s\n", cmd_ln_str("-mllrmat"));
	}

	if (feat_dimension1(feat) != tmp_n_stream) {
	    E_FATAL("# feature streams in -mllrmat %s != # feature streams configured on cmd ln\n");
	}
	
	feat_veclen = (uint32 *)feat_stream_lengths(feat);

	for (j = 0; j < tmp_n_stream; j++) {
	    if (feat_veclen[j] != tmp_veclen[j]) {
		E_FATAL("# components of stream %u in -mllrmat inconsistent w/ -feat config (%u != %u)\n",
			j, tmp_veclen[j], feat_veclen[j]);
	    }
	}
	ckd_free((void *)tmp_veclen);

	fn = cmd_ln_str("-cb2mllrfn");
	if (fn != NULL) {
	    if (strcmp(fn, ".1cls.") == 0) {
		mllr_idx = ckd_calloc(inv->gauden->n_mgau, sizeof(int32));
		n_mllr = 1;
		n_map = inv->gauden->n_mgau;
	    }
	    else if (s3cb2mllr_read(cmd_ln_str("-cb2mllrfn"),
				    &mllr_idx,
				    &n_map,
				    &n_mllr) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	    if (n_map != inv->gauden->n_mgau) {
		E_FATAL("cb2mllr maps %u cb, but read %u cb from files\n",
			n_map, inv->gauden->n_mgau);
	    }
	}

	/* Transform the means using the speaker transform if available. */
	mllr_transform_mean(inv->gauden->mean,
			    inv->gauden->var,
			    0, inv->gauden->n_mgau,
			    inv->gauden->n_feat,
			    inv->gauden->n_density,
			    inv->gauden->veclen,
			    sxfrm_a, sxfrm_b,
			    mllr_idx, n_mllr);
	ckd_free(mllr_idx);
	free_mllr_A(sxfrm_a, n_mllr, tmp_n_stream);
	free_mllr_B(sxfrm_b, n_mllr, tmp_n_stream);
    }

    return S3_SUCCESS;
}

void
main_reestimate(model_inventory_t *inv,
		lexicon_t *lex,
		model_def_t *mdef,
		feat_t *feat,
		int32 viterbi)
{
    vector_t *mfcc;	/* utterance cepstra */	
    int32 n_frame;	/* # of cepstrum frames  */
    uint32 svd_n_frame;	/* # of cepstrum frames  */
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
    char *trans;
    const char *pdumpdir;
    FILE *pdumpfh;
    uint32 in_veclen;

    bw_timers_t* timers = NULL;
    int32 profile;

    int32 pass2var;
    int32 var_is_full;

    uint32 n_utt;

    s3phseg_t *phseg = NULL;

    uint32 maxuttlen;
    uint32 n_frame_skipped = 0;

    uint32 ckpt_intv = 0;
    uint32 no_retries = 0;

    uint32 outputfullpath = 0;

    E_INFO("Reestimation: %s\n",
	(viterbi ? "Viterbi" : "Baum-Welch"));

    profile = cmd_ln_int32("-timing");
    if (profile) {
	E_INFO("Generating profiling information consumes significant CPU resources.\n");
	E_INFO("If you are not interested in profiling, use -timing no\n");
    }
    outputfullpath = cmd_ln_int32("-outputfullpath");

    if (profile) {
	timers = ckd_calloc(1, sizeof(bw_timers_t));
	ptmr_init(&timers->utt_timer);
	ptmr_init(&timers->upd_timer);
	ptmr_init(&timers->fwd_timer);
	ptmr_init(&timers->bwd_timer);
	ptmr_init(&timers->gau_timer);
	ptmr_init(&timers->rsts_timer);
	ptmr_init(&timers->rstf_timer);
	ptmr_init(&timers->rstu_timer);
    }

    mixw_reest = cmd_ln_int32("-mixwreest");
    tmat_reest = cmd_ln_int32("-tmatreest");
    mean_reest = cmd_ln_int32("-meanreest");
    var_reest = cmd_ln_int32("-varreest");
    pass2var = cmd_ln_int32("-2passvar");
    var_is_full = cmd_ln_int32("-fullvar");
    pdumpdir = cmd_ln_str("-pdumpdir");
    in_veclen = cmd_ln_int32("-ceplen");

    if (cmd_ln_str("-ckptintv")) {
	ckpt_intv = cmd_ln_int32("-ckptintv");
    }

    if (cmd_ln_str("-accumdir") == NULL) {
	E_WARN("NO ACCUMDIR SET.  No counts will be written; assuming debug\n");
    }

    if (!mixw_reest && !tmat_reest && !mean_reest && !var_reest) {
	E_WARN("No reestimation specified!  None done.\n");
	
	return;
    }

    total_log_lik = 0;
    total_frames = 0;

    a_beam = cmd_ln_float64("-abeam");
    b_beam = cmd_ln_float64("-bbeam");
    spthresh = cmd_ln_float32("-spthresh");
    maxuttlen = cmd_ln_int32("-maxuttlen");

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
    if (!cmd_ln_int32("-viterbi")) {
	printf("\t<avg_states_beta>\n");
	printf("\t<avg_states_reest>\n");
	printf("\t<avg_posterior_prune>\n");
    }
    printf("\t<frame_log_lik>\n");
    printf("\t<utt_log_lik>\n");
    printf("\t... timing info ... \n");

    n_utt = 0;

    while (corpus_next_utt()) {
	/* Zero timers before utt processing begins */
	if (timers) {
	    ptmr_reset(&timers->utt_timer);
	    ptmr_reset(&timers->upd_timer);
	    ptmr_reset(&timers->fwd_timer);
	    ptmr_reset(&timers->bwd_timer);
	    ptmr_reset(&timers->gau_timer);
	    ptmr_reset(&timers->rsts_timer);
	    ptmr_reset(&timers->rstf_timer);
	    ptmr_reset(&timers->rstu_timer);
	}
	
	if (timers)
	    ptmr_start(&timers->utt_timer);

	printf("utt> %5u %25s", 
	       seq_no,
	       (outputfullpath ? corpus_utt_full_name() : corpus_utt()));

        if (corpus_get_generic_featurevec(&mfcc, &n_frame, in_veclen) < 0) {
	        E_FATAL("Can't read input features\n");
	}

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

	svd_n_frame = n_frame;

	f = feat_array_alloc(feat, n_frame + feat_window_size(feat));
	feat_s2mfc2feat_live(feat, mfcc, &n_frame, TRUE, TRUE, f);

	printf(" %4u", n_frame - svd_n_frame);

	/* Get the transcript */
	corpus_get_sent(&trans);

	/* Get the phone segmentation */
	corpus_get_phseg(inv->acmod_set, &phseg);

	/* Open a dump file if required. */
	if (pdumpdir) {
		char *pdumpfn, *uttid;

		uttid = (outputfullpath ? corpus_utt_full_name() : corpus_utt());
		pdumpfn = ckd_calloc(strlen(pdumpdir) + 1
				     + strlen(uttid)
				     + strlen(".pdump") + 1, 1);
		strcpy(pdumpfn, pdumpdir);
		strcat(pdumpfn, "/");
		strcat(pdumpfn, uttid);
		strcat(pdumpfn, ".pdump");
		if ((pdumpfh = fopen(pdumpfn, "w")) == NULL)
			E_FATAL_SYSTEM("Failed to open %s for writing", pdumpfn);
		ckd_free(pdumpfn);
	}
	else
		pdumpfh = NULL;

        if (timers)
	    ptmr_start(&timers->upd_timer);
	/* create a sentence HMM */
	state_seq = next_utt_states(&n_state, lex, inv, mdef, trans);
	printf(" %5u", n_state);
	
	if (state_seq == NULL) {
	    E_WARN("Skipped utterance '%s'\n", trans);
	} else if (!viterbi) {
	    /* accumulate reestimation sums for the utterance */
	    if (baum_welch_update(&log_lik,
				  f, n_frame,
				  state_seq, n_state,
				  inv,
				  a_beam,
				  b_beam,
				  spthresh,
				  phseg,
				  mixw_reest,
				  tmat_reest,
				  mean_reest,
				  var_reest,
				  pass2var,
				  var_is_full,
				  pdumpfh,
				  timers,
				  feat) == S3_SUCCESS) {
		total_frames += n_frame;
		total_log_lik += log_lik;
		
		printf(" %e %e",
		       (n_frame > 0 ? log_lik / n_frame : 0.0),
		       log_lik);
	    }

	} else {
	    /* Viterbi search and accumulate in it */
	    if (viterbi_update(&log_lik,
			       f, n_frame,
			       state_seq, n_state,
			       inv,
			       a_beam,
			       spthresh,
			       phseg,
			       mixw_reest,
			       tmat_reest,
			       mean_reest,
			       var_reest,
			       pass2var,
			       var_is_full,
			       pdumpfh, 
			       timers,
			       feat) == S3_SUCCESS) {
		total_frames += n_frame;
		total_log_lik += log_lik;
		printf(" %e %e",
		       (n_frame > 0 ? log_lik / n_frame : 0.0),
		       log_lik);
	    }
	}

	if (timers)
	    ptmr_stop(&timers->upd_timer);

	if (pdumpfh)
		fclose(pdumpfh);
	free(mfcc[0]);
	ckd_free(mfcc);
	feat_array_free(f);
	free(trans);	/* alloc'ed using strdup() */

	seq_no++;
	n_utt++;

        if (timers)
	    ptmr_stop(&timers->utt_timer);
    
	if (profile)
	    print_all_timers(timers, n_frame);

	printf("\n");
	fflush(stdout);

	if ((ckpt_intv > 0) &&
	    ((n_utt % ckpt_intv) == 0) &&
	    (cmd_ln_str("-accumdir") != NULL)) {
	    while (accum_dump(cmd_ln_str("-accumdir"),
			      inv,
			      mixw_reest,
			      tmat_reest,
			      mean_reest,
			      var_reest,
			      pass2var,
			      var_is_full,
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

    printf("overall> stats %u (-%u) %e %e",
	   total_frames,
	   n_frame_skipped,
	   (total_frames > 0 ? total_log_lik / total_frames : 0.0),
	   total_log_lik);
    if (profile) {
	printf(" %4.3fx %4.3fe",
	       (total_frames > 0 ? timers->utt_timer.t_tot_cpu/(total_frames*0.01) : 0.0),
	       (timers->utt_timer.t_tot_cpu > 0 ? timers->utt_timer.t_tot_elapsed / timers->utt_timer.t_tot_cpu : 0.0));
    }    
    printf("\n");
    fflush(stdout);

    no_retries=0;
    /* dump the accumulators to a file system */
    while (cmd_ln_str("-accumdir") != NULL &&
	   accum_dump(cmd_ln_str("-accumdir"), inv,
		      mixw_reest,
		      tmat_reest,
		      mean_reest,
		      var_reest,
		      pass2var,
		      var_is_full,
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

    if (profile) {
	ckd_free(timers);
    }

    /* Write a log entry on success */
    if (cmd_ln_str("-accumdir"))
	E_INFO("Counts saved to %s\n", cmd_ln_str("-accumdir"));
    else
	E_INFO("Counts NOT saved.\n");
}

/* x=log(a) y=log(b), log_add(x,y) = log(a+b) */
float64
log_add(float64 x, float64 y)
{
  float64 z;
  
  if (x<y)
    return log_add(y, x);
  if (y == LOG_ZERO)
    return x;
  else
    {
      z = exp(y-x);
      return x+log(1.0+z);
    }
}

/* forward-backward computation on lattice */
int
lat_fwd_bwd(s3lattice_t *lat)
{
  int i, j;
  uint32 id;
  float64 ac_score, lm_score;

  /* step forward */
  for (i=0; i<lat->n_arcs; i++) {
    /* initialise alpha */
    lat->arc[i].alpha = LOG_ZERO;
    if (lat->arc[i].good_arc == 1) {
      /* get the acoustic and lm socre for a word hypothesis */
      ac_score = lat->arc[i].ac_score / lm_scale;
      lm_score = lat->arc[i].lm_score;

      /* compute alpha */
      for (j=0; j<lat->arc[i].n_prev_arcs; j++) {
	id = lat->arc[i].prev_arcs[j];
	if (id == 0) {
	  if (lat->arc[i].sf == 1) {
	    lat->arc[i].alpha = log_add(lat->arc[i].alpha, 0);
	  }
	}
	else {
	  if (lat->arc[id-1].good_arc == 1)
	    lat->arc[i].alpha = log_add(lat->arc[i].alpha, lat->arc[id-1].alpha);
	}
      }
      lat->arc[i].alpha += ac_score + lm_score;
    }
  }

  /* initialise overall log-likelihood */
  lat->prob = LOG_ZERO;

  /* step backward */
  for (i=lat->n_arcs-1; i>=0 ;i--) {
    /* initialise beta */
    lat->arc[i].beta = LOG_ZERO;

    if (lat->arc[i].good_arc == 1) {
      /* get the acoustic and lm socre for a word hypothesis */
      ac_score = lat->arc[i].ac_score / lm_scale;
      lm_score = lat->arc[i].lm_score;

      /* compute beta */
      for (j=0; j<lat->arc[i].n_next_arcs; j++) {
	id = lat->arc[i].next_arcs[j];
	if (id == 0) {
	  lat->arc[i].beta = log_add(lat->arc[i].beta, 0);
	}
	else {
	  if (lat->arc[id-1].good_arc == 1)
        ;
	  lat->arc[i].beta = log_add(lat->arc[i].beta, lat->arc[id-1].beta);
	}
      }
      lat->arc[i].beta += ac_score + lm_score;

      /* compute overall log-likelihood loglid=beta(1)=alpha(Q) */
      if (lat->arc[i].sf == 1)
	lat->prob = log_add(lat->prob, lat->arc[i].beta);
    }
  }

  /* compute gamma */
  for (i=0; i<lat->n_arcs; i++)
    {
      /* initialise gamma */
      lat->arc[i].gamma = LOG_ZERO;
      if (lat->arc[i].good_arc == 1)
	{
	  ac_score = lat->arc[i].ac_score / lm_scale;
	  lm_score = lat->arc[i].lm_score;
	  lat->arc[i].gamma = lat->arc[i].alpha + lat->arc[i].beta - (ac_score + lm_score + lat->prob);
	}
    }

  /* compute the posterior probability of the true path */
  lat->postprob = 0;
  for (i=lat->n_arcs-lat->n_true_arcs; i<lat->n_arcs; i++)
    lat->postprob += lat->arc[i].gamma;
  
  return S3_SUCCESS;
}

/* mmie training: take random left and right context for viterbi run */
int
mmi_rand_train(model_inventory_t *inv,
	       model_def_t *mdef,
	       lexicon_t *lex,
	       vector_t **f,
	       s3lattice_t *lat,
	       float64 a_beam,
	       uint32 mean_reest,
	       uint32 var_reest,
	       feat_t *fcb)
{
  uint32 k, n;
  uint32 n_rand;/* random number */
  uint32 n_max_run;/* the maximum number of viterbi run */
  char pword[128], cword[128], nword[128];      /* previous, current, next word */
  vector_t **arc_f = NULL;/* feature vector for a word arc */
  uint32 n_word_obs;/* frames of a word arc */
  uint32 rand_prev_id, rand_next_id;/* randomly selected previous and next arc id */
  uint32 *lphone, *rphone;        /* the last and first phone of previous and next word hypothesis */
  state_t *state_seq;/* HMM state sequence for an arc */
  uint32 n_state = 0;/* number of HMM states */
  float64 log_lik;/* log-likelihood of an arc */
  
  /* viterbi run on each arc */
  printf(" %5u", lat->n_arcs);
  
  for(n=0; n<lat->n_arcs; n++) {

    /* total observations of this arc */
    /* this is not very accurate, as it consumes one more frame for each word at the end */
    n_word_obs = lat->arc[n].ef - lat->arc[n].sf + 1;
    
    /* get the feature for this arc */
    arc_f = (vector_t **) ckd_calloc(n_word_obs, sizeof(vector_t *));
    for (k=0; k<n_word_obs; k++)
      arc_f[k] = f[k+lat->arc[n].sf-1];
    
    /* in case the viterbi run fails at a certain left and right context,
       at most randomly pick context n_prev_arcs * n_next_arcs times */
    n_max_run = lat->arc[n].n_prev_arcs * lat->arc[n].n_next_arcs;
    
    /* seed the random-number generator with current time */
    srand( (unsigned)time( NULL ) );
    
    /* randomly pick the left and right context */
    while (n_max_run > 0 && lat->arc[n].good_arc == 0) {
      
      /* get left arc id */
      if (lat->arc[n].n_prev_arcs == 1) {
	n_rand = 0;
      }
      else {
	n_rand = (uint32) (((double) rand() / (((double) RAND_MAX) + 1)) * lat->arc[n].n_prev_arcs );
      }
      rand_prev_id = lat->arc[n].prev_arcs[n_rand];

      /* get right arc id */
      if (lat->arc[n].n_next_arcs == 1) {
	n_rand = 0;
      }
      else {
	n_rand = (uint32) (((double) rand() / (((double) RAND_MAX) + 1)) * lat->arc[n].n_next_arcs );
      }
      rand_next_id = lat->arc[n].next_arcs[n_rand];
      
      /* get the triphone list */
      strcpy(cword, lat->arc[n].word);
      if (rand_prev_id == 0)
	strcpy(pword, "<s>");
      else
	strcpy(pword, lat->arc[rand_prev_id-1].word);
      lphone = mk_boundary_phone(pword, 0, lex);
      if (rand_next_id == 0)
	strcpy(nword, "</s>");
      else
	strcpy(nword, lat->arc[rand_next_id-1].word);
      rphone = mk_boundary_phone(nword, 1, lex);

      state_seq = next_utt_states_mmie(&n_state, lex, inv, mdef, cword, lphone, rphone);

      /* viterbi compuation to get the acoustic score for a word hypothesis */
      if (mmi_viterbi_run(&log_lik,
			  arc_f, n_word_obs,
			  state_seq, n_state,
			  inv,
			  a_beam) == S3_SUCCESS) {
	lat->arc[n].good_arc = 1;
	lat->arc[n].ac_score = log_lik;
	lat->arc[n].best_prev_arc = rand_prev_id;
	lat->arc[n].best_next_arc = rand_next_id;
      }

      n_max_run--;
      ckd_free(lphone);
      ckd_free(rphone);
    }
    
    ckd_free(arc_f);
    
    if (lat->arc[n].good_arc == 0) {
      E_INFO("arc_%d is ignored (viterbi run failed)\n", n+1);
    }
  }

  /* lattice-based forward-backward computation */
  lat_fwd_bwd(lat);

  /* update Gaussian parameters */
  for (n=0; n<lat->n_arcs; n++) {
    
    /* only if the arc was successful in viterbi run */
    if (lat->arc[n].good_arc == 1) {
      
      /* total observations of this arc */
      n_word_obs = lat->arc[n].ef - lat->arc[n].sf + 1;
      arc_f = (vector_t **) ckd_calloc(n_word_obs, sizeof(vector_t *));
      for (k=0; k<n_word_obs; k++)
	arc_f[k] = f[k+lat->arc[n].sf-1];
      
      /* get the randomly picked left and right context */
      rand_prev_id = lat->arc[n].best_prev_arc;
      rand_next_id = lat->arc[n].best_next_arc;
      
      /* get the triphone list */
      strcpy(cword, lat->arc[n].word);
      if (rand_prev_id == 0)
	strcpy(pword, "<s>");
      else
	strcpy(pword, lat->arc[rand_prev_id-1].word);
      lphone = mk_boundary_phone(pword, 0, lex);
      if (rand_next_id == 0)
	strcpy(nword, "</s>");
      else
	strcpy(nword, lat->arc[rand_next_id-1].word);
      rphone = mk_boundary_phone(nword, 1, lex);
      
      /* make state list */
      state_seq = next_utt_states_mmie(&n_state, lex, inv, mdef, cword, lphone, rphone);
      
      /* viterbi update model parameters */
      if (mmi_viterbi_update(arc_f, n_word_obs,
			     state_seq, n_state,
			     inv,
			     a_beam,
			     mean_reest,
			     var_reest,
			     lat->arc[n].gamma,
			     fcb) != S3_SUCCESS) {
	E_ERROR("arc_%d is ignored (viterbi update failed)\n", n+1);
      }
      ckd_free(arc_f);
      ckd_free(lphone);
      ckd_free(rphone);
    }
  }
  
  return S3_SUCCESS;
}

/* mmie training: take the best left and right context for viterbi run */
int
mmi_best_train(model_inventory_t *inv,
	       model_def_t *mdef,
	       lexicon_t *lex,
	       vector_t **f,
	       s3lattice_t *lat,
	       float64 a_beam,
	       uint32 mean_reest,
	       uint32 var_reest,
	       feat_t *fcb)
{
  uint32 i, j, k, n;
  char pword[128], cword[128], nword[128];      /* previous, current and next word hypothesis */
  vector_t **arc_f = NULL;/* feature vector for a word arc */
  uint32 n_word_obs;/* frames of a word arc */
  uint32 prev_id, next_id;/* previous and next arc id */
  uint32 *lphone, *rphone;/* the last and first phone of previous and next arc */
  uint32 prev_lphone, prev_rphone;/* the lphone and rphone of previous viterbi run on arc */
  state_t *state_seq;/* HMM state sequence for an arc */
  uint32 n_state = 0;/* number of HMM states */
  float64 log_lik;/* log-likelihood of an arc */
  
  /* viterbi run on each arc */
  printf(" %5u", lat->n_arcs);
  
  for(n=0; n<lat->n_arcs; n++) {
    
    /* total observations of this arc */
    /* this is not very accurate, as it consumes one more frame for each word at the end */
    n_word_obs = lat->arc[n].ef - lat->arc[n].sf + 1;
    
    /* get the feature for this arc */
    arc_f = (vector_t **) ckd_calloc(n_word_obs, sizeof(vector_t *));
    for (k=0; k<n_word_obs; k++)
      arc_f[k] = f[k+lat->arc[n].sf-1];
    
    /* now try to find the best left and right context for viterbi run */
    /* current word hypothesis */
    strcpy(cword, lat->arc[n].word);
    
    /* initialise previous lphone */
    prev_lphone = 0;
    
    /* try all left context */
    for (i=0; i<lat->arc[n].n_prev_arcs; i++) {
      /* preceding word */
      prev_id = lat->arc[n].prev_arcs[i];
      if (prev_id == 0) {
	strcpy(pword, "<s>");
      }
      else {
	strcpy(pword, lat->arc[prev_id-1].word);
      }
      
      /* get the left boundary triphone */
      lphone = mk_boundary_phone(pword, 0, lex);
      
      /* if the previous preceeding arc has different context as the new one */
      if (*lphone != prev_lphone || i == 0) {
	
	/* initialize rphone */
	prev_rphone = 0;
	
	/* try all right context */
	for(j=0; j<lat->arc[n].n_next_arcs; j++) {
	  /* succeeding word */
	  next_id = lat->arc[n].next_arcs[j];
	  if (next_id == 0)
	    strcpy(nword, "</s>");
	  else
	    strcpy(nword, lat->arc[next_id-1].word);
	    
	  /* get the right boundary triphone */
	  rphone = mk_boundary_phone(nword, 1, lex);
	    
	  /* if the previous succeeding arc has different context as the new one */
	  if (*rphone != prev_rphone || j == 0) {
	        
	    /* make state list */
	    state_seq = next_utt_states_mmie(&n_state, lex, inv, mdef, cword, lphone, rphone);
	        
	    /* viterbi compuation to get the acoustic score for a word hypothesis */
	    if (mmi_viterbi_run(&log_lik,
				arc_f, n_word_obs,
				state_seq, n_state,
				inv,
				a_beam) == S3_SUCCESS) {
	      if (lat->arc[n].good_arc == 0) {
		lat->arc[n].good_arc = 1;
		lat->arc[n].ac_score = log_lik;
		lat->arc[n].best_prev_arc = lat->arc[n].prev_arcs[i];
		lat->arc[n].best_next_arc = lat->arc[n].next_arcs[j];
	      }
	      else if (log_lik > lat->arc[n].ac_score) {
		lat->arc[n].ac_score = log_lik;
		lat->arc[n].best_prev_arc = lat->arc[n].prev_arcs[i];
		lat->arc[n].best_next_arc = lat->arc[n].next_arcs[j];
	      }
	    }
	    /* save the current right context */
	    prev_rphone = *rphone;
	  }
	  ckd_free(rphone);
	}
	/* save the current left context */
	prev_lphone = *lphone;
      }
      ckd_free(lphone);
    }
    
    ckd_free(arc_f);
    
    if (lat->arc[n].good_arc == 0) {
      E_INFO("arc_%d is ignored (viterbi run failed)\n", n+1);
    }
  }
  
  /* lattice-based forward-backward computation */
  lat_fwd_bwd(lat);
  
  /* update Gaussian parameters */
  for (n=0; n<lat->n_arcs; n++) {
    
    /* only if the arc was successful in viterbi run */
    if (lat->arc[n].good_arc == 1) {
      
      /* total observations of this arc */
      n_word_obs = lat->arc[n].ef - lat->arc[n].sf + 1;
      arc_f = (vector_t **) ckd_calloc(n_word_obs, sizeof(vector_t *));
      for (k=0; k<n_word_obs; k++)
	arc_f[k] = f[k+lat->arc[n].sf-1];
      
      /* get the best left and right context */
      prev_id = lat->arc[n].best_prev_arc;
      next_id = lat->arc[n].best_next_arc;
      
      /* get best triphone list */
      strcpy(cword, lat->arc[n].word);
      if (prev_id == 0)
	strcpy(pword, "<s>");
      else
	strcpy(pword, lat->arc[prev_id-1].word);
      lphone = mk_boundary_phone(pword, 0, lex);
      if (next_id == 0)
	strcpy(nword, "</s>");
      else
	strcpy(nword, lat->arc[next_id-1].word);
      rphone = mk_boundary_phone(nword, 1, lex);
      
      /* make state list */
      state_seq = next_utt_states_mmie(&n_state, lex, inv, mdef, cword, lphone, rphone);
      
      /* viterbi update model parameters */
      if (mmi_viterbi_update(arc_f, n_word_obs,
			     state_seq, n_state,
			     inv,
			     a_beam,
			     mean_reest,
			     var_reest,
			     lat->arc[n].gamma,
			     fcb) != S3_SUCCESS) {
	E_ERROR("arc_%d is ignored (viterbi update failed)\n", n+1);
      }
      ckd_free(arc_f);
      ckd_free(lphone);
      ckd_free(rphone);
    }
  }
  
  return S3_SUCCESS;
}

/* mmie training: use context-independent hmms for word boundary models */
int
mmi_ci_train(model_inventory_t *inv,
	     model_def_t *mdef,
	     lexicon_t *lex,
	     vector_t **f,
	     s3lattice_t *lat,
	     float64 a_beam,
	     uint32 mean_reest,
	     uint32 var_reest,
	     feat_t *fcb)
{
  uint32 k, n;
  vector_t **arc_f = NULL;/* feature vector for a word arc */
  uint32 n_word_obs;/* frames of a word arc */
  state_t *state_seq;/* HMM state sequence for an arc */
  uint32 n_state = 0;/* number of HMM states */
  float64 log_lik;/* log-likelihood of an arc */
  
  /* viterbi run on each arc */
  printf(" %5u", lat->n_arcs);

  for(n=0; n<lat->n_arcs; n++) {
    
    /* total observations of this arc */
    /* this is not very accurate, as it consumes one more frame for each word at the end */
    n_word_obs = lat->arc[n].ef - lat->arc[n].sf + 1;
    
    /* get the feature for this arc */
    arc_f = (vector_t **) ckd_calloc(n_word_obs, sizeof(vector_t *));
    for (k=0; k<n_word_obs; k++)
      arc_f[k] = f[k+lat->arc[n].sf-1];
    
    /* make state list */
    state_seq = next_utt_states(&n_state, lex, inv, mdef, lat->arc[n].word);
    
    /* viterbi compuation to get the acoustic score for a word hypothesis */
    if (mmi_viterbi_run(&log_lik,
			arc_f, n_word_obs,
			state_seq, n_state,
			inv,
			a_beam) == S3_SUCCESS) {
      lat->arc[n].good_arc = 1;
      lat->arc[n].ac_score = log_lik;
    }
    
    ckd_free(arc_f);
    
    if (lat->arc[n].good_arc == 0) {
      E_INFO("arc_%d is ignored (viterbi run failed)\n", n+1);
    }
  }
  
  /* lattice-based forward-backward computation */
  lat_fwd_bwd(lat);
  
  /* update Gaussian parameters */
  for (n=0; n<lat->n_arcs; n++) {
    
    /* only if the arc was successful in viterbi run */
    if (lat->arc[n].good_arc == 1) {
      
      /* total observations of this arc */
      n_word_obs = lat->arc[n].ef - lat->arc[n].sf + 1;
      arc_f = (vector_t **) ckd_calloc(n_word_obs, sizeof(vector_t *));
      for (k=0; k<n_word_obs; k++)
	arc_f[k] = f[k+lat->arc[n].sf-1];
      
      /* make state list */
      state_seq = next_utt_states(&n_state, lex, inv, mdef, lat->arc[n].word);
      
      /* viterbi update model parameters */
      if (mmi_viterbi_update(arc_f, n_word_obs,
			     state_seq, n_state,
			     inv,
			     a_beam,
			     mean_reest,
			     var_reest,
			     lat->arc[n].gamma,
			     fcb) != S3_SUCCESS) {
	E_ERROR("arc_%d is ignored (viterbi update failed)\n", n+1);
      }
      
      ckd_free(arc_f);
    }
  }
  
  return S3_SUCCESS;
}

/* main mmie training program */
void
main_mmi_reestimate(model_inventory_t *inv,
		    lexicon_t *lex,
		    model_def_t *mdef,
		    feat_t *feat)
{
  vector_t *mfcc;/* utterance cepstra */
  int32 n_frame;/* # of cepstrum frames  */
  uint32 svd_n_frame;        /* # of cepstrum frames  */
  vector_t **f;/* independent feature streams derived from cepstra */
  float32 ***lda = NULL;
  uint32 total_frames;        /* # of frames over the corpus */
  float64 a_beam;/* alpha pruning beam */
  float64 b_beam;/* beta pruning beam */
  float32 spthresh;        /* state posterior probability threshold */
  uint32 seq_no;/* sequence # of utterance in corpus */
  uint32 mean_reest;        /* if TRUE, reestimate means */
  uint32 var_reest;        /* if TRUE, reestimate variances */

  const char *lat_dir;        /* lattice directory */
  const char *lat_ext;/* denominator or numerator lattice */
  const char *mmi_type;/* different methods to get left and right context for Viterbi run on lattice */
  uint32 n_mmi_type = 0;/* convert the mmi_type string to a int */
  s3lattice_t *lat = NULL;/* input lattice */
  float64 total_log_postprob = 0;/* total posterior probability of the correct hypotheses */
  uint32 n_utt_fail = 0;        /* number of sentences failed */
  uint32 i;

  char *trans;
  uint32 in_veclen;
  uint32 n_utt;

  uint32 no_retries=0;

  uint32 maxuttlen;
  uint32 n_frame_skipped = 0;

  /* get rid of unnecessary arguments */
  if (cmd_ln_int32("-2passvar")) {
    E_FATAL("for MMIE training, set -2passvar to no\n");
  }
  if (cmd_ln_int32("-fullvar")) {
    E_FATAL("current MMIE training don't support full variance matrix, set -fullvar to no\n");
  }
  if (cmd_ln_int32("-timing")) {
    E_FATAL("current MMIE training don't support timing, set -timing to no\n");
  }
  if (cmd_ln_int32("-mixwreest")) {
    E_FATAL("current MMIE training don't support mixture weight reestimation, set -mixwreest to no\n");
  }
  if (cmd_ln_int32("-tmatreest")) {
    E_FATAL("current MMIE training don't support transition matrix reestimation, set -tmatreest to no\n");
  }
  if (cmd_ln_int32("-outputfullpath")) {
    E_FATAL("current MMIE training don't support outputfullpath, set -outputfullpath to no\n");
  }
  if (cmd_ln_str("-ckptintv")) {
    E_FATAL("current MMIE training don't support ckptintv, remove -ckptintv\n");
  }
  if (cmd_ln_str("-pdumpdir")) {
    E_FATAL("current MMIE training don't support pdumpdir, set -pdumpdir to no\n");
  }

  /* get lattice related parameters */
  lat_dir = cmd_ln_str("-latdir");
  lat_ext = cmd_ln_str("-latext");
  if (strcmp(lat_ext, "denlat") != 0 && strcmp(lat_ext, "numlat") != 0) {
    E_FATAL("-latext should be either denlat or numlat\n");
  }
  else {
    printf("MMIE training for %s \n", lat_ext);
  }
  mmi_type = cmd_ln_str("-mmie_type");
  if (strcmp(mmi_type, "rand") == 0) {
    n_mmi_type = 1;
    printf("MMIE training: take random left and right context for Viterbi run \n");
  }
  else if (strcmp(mmi_type, "best") == 0) {
    n_mmi_type = 2;
    printf("MMIE training: take the best left and right context for Viterbi run \n");
  }
  else if (strcmp(mmi_type, "ci") == 0) {
    printf("MMIE training: use context-independent hmms for boundary word models \n");
    n_mmi_type = 3;
  }
  else {
    E_FATAL("-mmie_type should be rand, best or ci\n");
  }
  lm_scale = cmd_ln_float32("-lw");

  mean_reest = cmd_ln_int32("-meanreest");
  var_reest = cmd_ln_int32("-varreest");
  in_veclen = cmd_ln_int32("-ceplen");
  
  /* Read in an LDA matrix for accumulation. */
  if (cmd_ln_str("-lda")) {
	feat_read_lda(feat, cmd_ln_str("-lda"), 
			    cmd_ln_int32("-ldadim"));
	lda = feat->lda;
  }

  if (cmd_ln_str("-accumdir") == NULL) {
    E_WARN("NO ACCUMDIR SET.  No counts will be written; assuming debug\n");
    return;
  }

  if (!mean_reest && !var_reest) {
    E_FATAL("No reestimation specified! Nothing done. Set -meanreest or -varreest \n");
    return;
  }

  total_frames = 0;

  a_beam = cmd_ln_float64("-abeam");
  b_beam = cmd_ln_float64("-bbeam");
  spthresh = cmd_ln_float32("-spthresh");
  maxuttlen = cmd_ln_int32("-maxuttlen");

  /* Begin by skipping over some (possibly zero) # of utterances.
   * Continue to process utterances until there are no more (either EOF
   * or end of run). */
  seq_no = corpus_get_begin();

  printf("column defns\n");
  printf("\t<seq>\n");
  printf("\t<id>\n");
  printf("\t<n_frame_in>\n");
  printf("\t<n_frame_del>\n");
  printf("\t<lattice_cat>\n");
  printf("\t<n_word>\n");
  printf("\t<lattice_log_postprob>\n");

  /* accumulate density for each training sentence */
  n_utt = 0;
  while (corpus_next_utt()) {
    printf("utt> %5u %25s",  seq_no, corpus_utt());
    
    if (corpus_get_generic_featurevec(&mfcc, &n_frame, in_veclen) < 0) {
	E_FATAL("Can't read input features\n");
    }
    
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
      
  
    svd_n_frame = n_frame;
      
    f = feat_array_alloc(feat, n_frame + feat_window_size(feat));
    feat_s2mfc2feat_live(feat, mfcc, &n_frame, TRUE, TRUE, f);
      
    printf(" %4u", n_frame - svd_n_frame);
      
    /* Get the transcript */
    corpus_get_sent(&trans);

    /* accumulate density counts on lattice */
    if (corpus_load_lattice(&lat, lat_dir, lat_ext) == S3_SUCCESS) {
      
      /* different type of mmie training */
      switch (n_mmi_type) {
	/* take random left and right context for viterbi run */
      case 1:
	{
	  if (mmi_rand_train(inv, mdef, lex, f, lat,
			     a_beam, mean_reest,
			     var_reest, feat) == S3_SUCCESS) {
	    total_log_postprob += lat->postprob;
	    printf("   %e", lat->postprob);
	  }
	  else {
	    n_utt_fail++;
	  }
	  break;
	}
	/* take the best left and right context for viterbi run */
      case 2:
	{
	  if (mmi_best_train(inv, mdef, lex, f, lat,
			      a_beam, mean_reest,
			     var_reest, feat) == S3_SUCCESS) {
	    total_log_postprob += lat->postprob;
	    printf("   %e", lat->postprob);
	  }
	  else {
	    n_utt_fail++;
	  }
	  break;
	}
	/* use context-independent hmms for word boundary models */
      case 3:
	{
	  if (mmi_ci_train(inv, mdef, lex, f, lat,
			   a_beam, mean_reest,
			   var_reest, feat) == S3_SUCCESS) {
	    total_log_postprob += lat->postprob;
	    printf("   %e", lat->postprob);
	  }
	  else {
	    n_utt_fail++;
	  }
	  break;
	}
	/* mmi_type error */
      default:
	{
	  E_FATAL("Invalid -mmie_type, try rand, best or ci \n");
	  break;
	}
      }
      
      /* free memory for lattice */
      for(i=0; i<lat->n_arcs; i++) {
	ckd_free(lat->arc[i].prev_arcs);
	ckd_free(lat->arc[i].next_arcs);
      }
      ckd_free(lat->arc);
      ckd_free(lat);
    }
    else {
      E_WARN("Can't read input lattice");
    }
    
    free(mfcc[0]);
    ckd_free(mfcc);
    feat_array_free(f);
    free(trans);
      
    seq_no++;
    n_utt++;

    printf("\n");
  }
    
  printf ("overall> stats %u (-%u) %e %e",
	  n_utt-n_utt_fail,
	  n_utt_fail,
	  (n_utt-n_utt_fail>0 ? total_log_postprob/(n_utt-n_utt_fail) : 0.0),
	  total_log_postprob);  
  printf("\n");
    
  no_retries=0;
  /* dump the accumulators to a file system */
  while (cmd_ln_str("-accumdir") != NULL &&
	 accum_mmie_dump(cmd_ln_str("-accumdir"),
			 lat_ext,
			 inv,
			 mean_reest,
			 var_reest) != S3_SUCCESS) {
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
  if (cmd_ln_str("-accumdir"))
    E_INFO("Counts saved to %s\n", cmd_ln_str("-accumdir"));
  else
    E_INFO("Counts NOT saved.\n");
}

int main(int argc, char *argv[])
{
    model_inventory_t *inv;
    lexicon_t *lex = NULL;
    model_def_t *mdef = NULL;
    feat_t *feat = NULL;
    
    if (main_initialize(argc, argv,
			&inv, &lex, &mdef, &feat) != S3_SUCCESS) {
	E_FATAL("initialization failed\n");
    }

    if (cmd_ln_int32("-mmie")) {
      main_mmi_reestimate(inv, lex, mdef, feat);
    }
    else {
      main_reestimate(inv, lex, mdef, feat, cmd_ln_int32("-viterbi"));
    }
    
    if (feat)
	feat_free(feat);
    if (mdef)
	model_def_free(mdef);    
    if (inv)
	mod_inv_free(inv);
    if (lex)
	lexicon_free(lex);

    return 0;
}
