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
 *	This routine uses a model definition file, a state parameter
 *	definition file and a control file to create a set of initial
 *	means and variance parameters for 1 density mixture Gaussian
 *	context independent system.
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include "init_gau.h"
#include "parse_cmd_ln.h"

#include <s3/corpus.h>
#include <s3/lexicon.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/ts2cb.h>
#include <s3/s3.h>

#include <string.h>

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/feat.h>


static lexicon_t *lex;
static model_def_t *mdef;
static feat_t *feat;

static int
initialize(int argc,
	   char *argv[])
{
    const char *fdictfn;
    const char *dictfn;
    const char *ts2cbfn;
    uint32 n_ts;
    uint32 n_cb;

    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

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


    if (cmd_ln_str("-segdir"))
	corpus_set_seg_dir(cmd_ln_str("-segdir"));
    if (cmd_ln_str("-segext"))
	corpus_set_seg_ext(cmd_ln_str("-segext"));

    corpus_set_mfcc_dir(cmd_ln_str("-cepdir"));
    corpus_set_mfcc_ext(cmd_ln_str("-cepext"));

    if (cmd_ln_str("-lsnfn"))
	corpus_set_lsn_filename(cmd_ln_str("-lsnfn"));

    corpus_set_ctl_filename(cmd_ln_str("-ctlfn"));

    if (cmd_ln_int32("-nskip") && cmd_ln_int32("-runlen")) {
        corpus_set_interval(cmd_ln_int32("-nskip"),
			    cmd_ln_int32("-runlen"));
    } else if (cmd_ln_int32("-part") && cmd_ln_int32("-npart")) {
	corpus_set_partition(cmd_ln_int32("-part"),
			     cmd_ln_int32("-npart"));
    }
    

    if (corpus_init() != S3_SUCCESS) {
	return S3_ERROR;
    }
    
    if (cmd_ln_str("-moddeffn")) {
	E_INFO("Reading %s\n", cmd_ln_str("-moddeffn"));
    
	/* Read in the model definitions.  Defines the set of
	   CI phones and context dependent phones.  Defines the
	   transition matrix tying and state level tying. */
	if (model_def_read(&mdef,
			   cmd_ln_str("-moddeffn")) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	
	ts2cbfn = cmd_ln_str("-ts2cbfn");
	if (strcmp(SEMI_LABEL, ts2cbfn) == 0) {
	    mdef->cb = semi_ts2cb(mdef->n_tied_state);
	    n_ts = mdef->n_tied_state;
	    n_cb = 1;
	}
	else if (strcmp(CONT_LABEL, ts2cbfn) == 0) {
	    mdef->cb = cont_ts2cb(mdef->n_tied_state);
	    n_ts = mdef->n_tied_state;
	    n_cb = mdef->n_tied_state;
	}
	else if (strcmp(PTM_LABEL, ts2cbfn) == 0) {
	    mdef->cb = ptm_ts2cb(mdef);
	    n_ts = mdef->n_tied_state;
	    n_cb = mdef->acmod_set->n_ci;
	}
	else if (s3ts2cb_read(ts2cbfn,
			      &mdef->cb,
			      &n_ts,
			      &n_cb) != S3_SUCCESS) {
	    return S3_ERROR;
	}

	dictfn = cmd_ln_str("-dictfn");

	if (dictfn == NULL) {
	    E_FATAL("You must specify a content dictionary using -dictfn\n");
	}

	E_INFO("Reading %s\n", dictfn);
	
	lex = lexicon_read(NULL,	/* no lexicon to start */
			   dictfn,
			   mdef->acmod_set);
	if (lex == NULL)
	    return S3_ERROR;
    
	fdictfn = cmd_ln_str("-fdictfn");

	if (fdictfn) {
	    E_INFO("Reading %s\n", fdictfn);
	    
	    (void)lexicon_read(lex,	/* add filler words content lexicon */
			       fdictfn,
			       mdef->acmod_set);
	}
    }

    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("Errors initializing.\n");
	return 1;
    }
    
    if (init_gau(lex, mdef, feat) != S3_SUCCESS) {
	return 1;
    }

    return 0;
}

