/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
#include <s3/cmd_ln.h>
#include <s3/lexicon.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <s3/ckd_alloc.h>
#include <s3/feat.h>
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

    corpus_set_seg_dir(cmd_ln_access("-segdir"));
    corpus_set_seg_ext(cmd_ln_access("-segext"));

    if (cmd_ln_access("-lsnfn"))
	corpus_set_lsn_filename(cmd_ln_access("-lsnfn"));
    else {
	corpus_set_sent_dir(cmd_ln_access("-sentdir"));
	corpus_set_sent_ext(cmd_ln_access("-sentext"));
    }

    corpus_set_ctl_filename(cmd_ln_access("-ctlfn"));
    
    if (cmd_ln_access("-part") && cmd_ln_access("-npart")) {
	corpus_set_partition(*(uint32 *)cmd_ln_access("-part"),
			     *(uint32 *)cmd_ln_access("-npart"));
    }
    else if (cmd_ln_access("-nskip") && cmd_ln_access("-runlen")) {
	corpus_set_interval(*(uint32 *)cmd_ln_access("-nskip"),
			    *(uint32 *)cmd_ln_access("-runlen"));
    }

    if (corpus_init() != S3_SUCCESS) {
	return S3_ERROR;
    }
    
    E_INFO("Reading: %s\n", cmd_ln_access("-moddeffn"));
    
    /* Read in the model definitions.  Defines the set of
       CI phones and context dependent phones.  Defines the
       transition matrix tying and state level tying. */
    if (model_def_read(&mdef, cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	return S3_ERROR;
    }

    ts2cbfn = cmd_ln_access("-ts2cbfn");
    if (ts2cbfn) {
	E_INFO("Reading %s\n", cmd_ln_access("-ts2cbfn"));

	       
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
	else if (s3ts2cb_read(cmd_ln_access("-ts2cbfn"),
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
    
    dictfn = cmd_ln_access("-dictfn");

    if (dictfn == NULL) {
	E_FATAL("You must specify a content dictionary using -dictfn\n");
    }

    E_INFO("Reading: %s\n", dictfn);

    lex = lexicon_read(NULL,	/* no lexicon to start */
		       dictfn,
		       mdef->acmod_set);
    if (lex == NULL)
	return S3_ERROR;
    
    fdictfn = cmd_ln_access("-fdictfn");

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

    if (initialize(&lex, &mdef, argc, argv) != S3_SUCCESS) {
	E_ERROR("errors initializing.\n");
	return 1;
    }

    type = (const char *)cmd_ln_access("-paramtype");
    
    if (param_cnt(lex, mdef, type) != S3_SUCCESS) {
	return 1;
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:32  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/03/07  08:41:14  eht
 * - Deal w/ new i/o routines
 * 
 * Revision 1.1  1996/03/25  15:21:20  eht
 * Initial revision
 *
 *
 */
