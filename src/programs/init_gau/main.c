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
#include <s3/cmd_ln.h>
#include <s3/lexicon.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/ts2cb.h>
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
    lexicon_t *lex = NULL;
    model_def_t *mdef = NULL;
    const char *fdictfn;
    const char *dictfn;
    const char *ts2cbfn;
    uint32 n_ts;
    uint32 n_cb;

    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    if (cmd_ln_access("-feat") != NULL) {
	feat_set((const char *)cmd_ln_access("-feat"));
	feat_set_in_veclen(*(int32 *)cmd_ln_access("-ceplen"));
    }
    else {
	E_ERROR("Specify the feature extraction algorithm using -feat\n");

	return S3_ERROR;
    }

    if (cmd_ln_access("-segdir"))
	corpus_set_seg_dir(cmd_ln_access("-segdir"));
    if (cmd_ln_access("-segext"))
	corpus_set_seg_ext(cmd_ln_access("-segext"));

    corpus_set_mfcc_dir(cmd_ln_access("-cepdir"));
    corpus_set_mfcc_ext(cmd_ln_access("-cepext"));

    if (cmd_ln_access("-lsnfn"))
	corpus_set_lsn_filename(cmd_ln_access("-lsnfn"));

    corpus_set_ctl_filename(cmd_ln_access("-ctlfn"));
    
    if ( cmd_ln_access("-nskip") && cmd_ln_access("-runlen") ) {
	corpus_set_interval(*(int32 *)cmd_ln_access("-nskip"),
			    *(int32 *)cmd_ln_access("-runlen"));
    }
    else if (cmd_ln_access("-part") && cmd_ln_access("-npart")) {
	corpus_set_partition(*(uint32 *)cmd_ln_access("-part"),
			     *(uint32 *)cmd_ln_access("-npart"));
    }
    

    if (corpus_init() != S3_SUCCESS) {
	return S3_ERROR;
    }
    
    if (cmd_ln_access("-moddeffn")) {
	E_INFO("Reading %s\n", cmd_ln_access("-moddeffn"));
    
	/* Read in the model definitions.  Defines the set of
	   CI phones and context dependent phones.  Defines the
	   transition matrix tying and state level tying. */
	if (model_def_read(&mdef,
			   cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	
	ts2cbfn = (const char *)cmd_ln_access("-ts2cbfn");
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
	else if (s3ts2cb_read(ts2cbfn,
			      &mdef->cb,
			      &n_ts,
			      &n_cb) != S3_SUCCESS) {
	    return S3_ERROR;
	}

	dictfn = cmd_ln_access("-dictfn");

	if (dictfn == NULL) {
	    E_FATAL("You must specify a content dictionary using -dictfn\n");
	}

	E_INFO("Reading %s\n", dictfn);
	
	lex = lexicon_read(NULL,	/* no lexicon to start */
			   dictfn,
			   mdef->acmod_set);
	if (lex == NULL)
	    return S3_ERROR;
    
	fdictfn = cmd_ln_access("-fdictfn");

	if (fdictfn) {
	    E_INFO("Reading %s\n", fdictfn);
	    
	    (void)lexicon_read(lex,	/* add filler words content lexicon */
			       fdictfn,
			       mdef->acmod_set);
	}
    }

    *out_mdef = mdef;
    *out_lex = lex;

    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    lexicon_t *lex;
    model_def_t *mdef;

    if (initialize(&lex, &mdef,
		   argc, argv) != S3_SUCCESS) {
	E_ERROR("errors initializing.\n");
	return 1;
    }
    
    if (init_gau(lex, mdef) != S3_SUCCESS) {
	return 1;
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:30:34  egouvea
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
 * Revision 1.9  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.8  96/08/06  14:15:15  eht
 * Define missing prototype
 * 
 * Revision 1.7  1996/08/06  14:08:28  eht
 * Deal w/ new model definition structure which includes an acoustic model set
 * mapping structure
 *
 * Revision 1.6  1996/03/25  15:43:43  eht
 * Deal w setting the input feature vector length
 * Deal w/ -nskip and -runlen arguments
 *
 * Revision 1.5  1996/01/26  18:22:07  eht
 * Use the feat module.
 *
 * Revision 1.4  1995/12/14  20:00:17  eht
 * Added (void *) type cast for ckd_free() argument.
 *
 * Revision 1.3  1995/12/14  19:57:44  eht
 * Yet another interim development version
 *
 * Revision 1.2  1995/12/01  20:55:40  eht
 * interim development version
 *
 * Revision 1.1  1995/12/01  16:39:04  eht
 * Initial revision
 *
 *
 */
