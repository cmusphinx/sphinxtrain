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
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"
#include "param_cnt.h"
#include "ts_cnt.h"

#include <s3/s3ts2cb_io.h>
#include <s3/ts2cb.h>
#include <s3/corpus.h>
#include <sphinxbase/cmd_ln.h>
#include <s3/lexicon.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>

#include <sys_compat/file.h>

#include <string.h>

static int
initialize(lexicon_t **out_lex,
	   model_def_t **out_mdef,
	   int argc,
	   char *argv[])
{
    lexicon_t *lex;
    model_def_t *mdef;
    const char *fdictfn;
    const char *dictfn;
    const char *ts2cbfn;
    uint32 n_ts;

    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    corpus_set_seg_dir(cmd_ln_str("-segdir"));
    corpus_set_seg_ext(cmd_ln_str("-segext"));

    if (cmd_ln_str("-lsnfn"))
	corpus_set_lsn_filename(cmd_ln_str("-lsnfn"));
    else {
	corpus_set_sent_dir(cmd_ln_str("-sentdir"));
	corpus_set_sent_ext(cmd_ln_str("-sentext"));
    }

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
    
    E_INFO("Reading: %s\n", cmd_ln_str("-moddeffn"));
    
    /* Read in the model definitions.  Defines the set of
       CI phones and context dependent phones.  Defines the
       transition matrix tying and state level tying. */
    if (model_def_read(&mdef, cmd_ln_str("-moddeffn")) != S3_SUCCESS) {
	return S3_ERROR;
    }

    ts2cbfn = cmd_ln_str("-ts2cbfn");
    if (ts2cbfn) {
	E_INFO("Reading %s\n", cmd_ln_str("-ts2cbfn"));

	       
	if (strcmp(ts2cbfn, SEMI_LABEL) == 0) {		/* see <s3/ts2cb.h> */
	    mdef->cb = semi_ts2cb(mdef->n_tied_state);
	    n_ts = mdef->n_tied_state;
	    mdef->n_cb = 1;
	}
	else if (strcmp(ts2cbfn, CONT_LABEL) == 0) {	/* see <s3/ts2cb.h> */
	    mdef->cb = cont_ts2cb(mdef->n_tied_state);
	    n_ts = mdef->n_tied_state;
	    mdef->n_cb = mdef->n_tied_state;
	}
	else if (strcmp(PTM_LABEL, ts2cbfn) == 0) {
	    mdef->cb = ptm_ts2cb(mdef);
	    n_ts = mdef->n_tied_state;
	    mdef->n_cb = mdef->acmod_set->n_ci;
	}
	else if (s3ts2cb_read(cmd_ln_str("-ts2cbfn"),
			      &mdef->cb,
			      &n_ts,
			      &mdef->n_cb) != S3_SUCCESS) {
	    return S3_ERROR;
	}

	if (n_ts != mdef->n_tied_state) {
	    E_WARN("# of tied states in ts2cb mapping, %u != # tied states in model def, %u\n", n_ts, mdef->n_tied_state);
	}

    }

    *out_mdef = mdef;
    
    dictfn = cmd_ln_str("-dictfn");

    if (dictfn == NULL) {
	E_FATAL("You must specify a content dictionary using -dictfn\n");
    }

    E_INFO("Reading: %s\n", dictfn);

    lex = lexicon_read(NULL,	/* no lexicon to start */
		       dictfn,
		       mdef->acmod_set);
    if (lex == NULL)
	return S3_ERROR;
    
    fdictfn = cmd_ln_str("-fdictfn");

    if (fdictfn) {
	E_INFO("Reading: %s\n", fdictfn);

	(void)lexicon_read(lex,	/* add filler words content lexicon */
			   fdictfn,
			   mdef->acmod_set);
    }

    *out_lex = lex;

    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    lexicon_t *lex;
    model_def_t *mdef;
    const char *type;
    const char *outfn;
    FILE *out_fp = stdout;

    if (initialize(&lex, &mdef, argc, argv) != S3_SUCCESS) {
	E_ERROR("errors initializing.\n");
	return 1;
    }

    type = cmd_ln_str("-paramtype");
    outfn = cmd_ln_str("-outputfn");
    if (outfn != NULL) {
	out_fp = fopen(outfn, "w");
	if (out_fp == NULL) {
	    E_ERROR_SYSTEM("Couldn't open %s for writing\n", outfn);
	}
    }

    if (param_cnt(out_fp, lex, mdef, type) != S3_SUCCESS) {
	return 1;
    }

    return 0;
}
