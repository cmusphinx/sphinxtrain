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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * 	Create a tied-state-to-codebook mapping file for semi-continuous,
 *	phone dependent or fully continuous Gaussian density tying.
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/s3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int
main(int argc, char *argv[])
{
    uint32 *spd;
    model_def_t *mdef;
    const char *tying_type;
    uint32 i;
    uint32 n_cb=0;
    
    parse_cmd_ln(argc, argv);

    E_INFO("Reading model definition file %s\n", cmd_ln_access("-moddeffn"));
    
    if (model_def_read(&mdef, cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	exit(1);
    }

    E_INFO("%d tied states defined\n", mdef->n_tied_state);

    tying_type = cmd_ln_access("-tyingtype");

    E_INFO("Generating state parameter definitions for %s tying\n", tying_type);
    
    spd = ckd_calloc(mdef->n_tied_state, sizeof(uint32));

    if (strcmp(tying_type, "semi") == 0) {
	n_cb = 1;
    }
    else if (strcmp(tying_type, "pd") == 0) {
	E_INFO("Phone dependent codebooks not yet implemented\n");
	exit(1);
    }
    else if (strcmp(tying_type, "cont") == 0) {
	n_cb = mdef->n_tied_state;
	for (i = 0; i < mdef->n_tied_state; i++)
	    spd[i] = i;
    }
    else {
	E_FATAL("Unknown tying type %s given\n", tying_type);
    }

    E_INFO("Writing %s\n", cmd_ln_access("-ts2cbfn"));

    if (s3ts2cb_write(cmd_ln_access("-ts2cbfn"),
		      spd,
		      mdef->n_tied_state,
		      n_cb) != S3_SUCCESS) {
	E_FATAL_SYSTEM("Unable to write %s\n", cmd_ln_access("-ts2cbfn"));
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.4  97/03/07  08:54:27  eht
 * - deal w/ new i/o routines
 * 
 * Revision 1.3  1995/12/01  16:59:18  eht
 * Add the SPHINX-III standard comments
 *
 *
 */
