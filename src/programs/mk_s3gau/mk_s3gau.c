/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
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
 * File: mk_s3gau.c
 * 
 * Description: 
 * 	Conversion from SPHINX-II codebooks to SPHINX-III mean and
 *	variance files.
 *	
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "cmd_ln.h"

#include <s3/gauden.h>
#include <s3/s3gau_io.h>
#include <s3/s2_param.h>
#include <s3/s2_read_cb.h>
#include <s3/feat.h>
#include <s3/err.h>
#include <s3/cmd_ln.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

int
main(int argc, char *argv[])
{
    gauden_t *g;
    const char *cb_basename[4];
    char comment[1024];
    time_t t;

    parse_cmd_ln(argc, argv);

    if (cmd_ln_access("-feat") != NULL) {
	feat_set((const char *)cmd_ln_access("-feat"));
	if (cmd_ln_access("-ceplen") != NULL) {
	    feat_set_in_veclen(*(int32 *)cmd_ln_access("-ceplen"));
	}
	else {
	    E_FATAL("Define the input feature vector len using -veclen argument\n");
	}
    }
    else {
	E_FATAL("Define a feature set using -feat argument\n");
    }

    g = gauden_alloc();

    gauden_set_min_var(*(float32 *)cmd_ln_access("-varfloor"));

    cb_basename[0] = (const char *)cmd_ln_access("-cepcb");
    cb_basename[1] = (const char *)cmd_ln_access("-dcepcb");
    cb_basename[2] = (const char *)cmd_ln_access("-powcb");
    cb_basename[3] = (const char *)cmd_ln_access("-2dcepcb");

    s2_read_cb(g,
	       cmd_ln_access("-cbdir"),
	       cb_basename,
	       cmd_ln_access("-meanext"),
	       cmd_ln_access("-varext"),
	       TRUE,	/* floor variances */
	       cmd_ln_access("-fixpowvar"));

    t = time(NULL);

    sprintf(comment,
	    "Generated on %s\nby %s.\nFrom codebooks in %s\n",
	    ctime(&t),
	    argv[0],
	    (const char *)cmd_ln_access("-cbdir"));
    
    E_INFO("writing %s\n", cmd_ln_access("-meanfn"));

    if (s3gau_write((const char *)cmd_ln_access("-meanfn"),
		    (const vector_t ***)gauden_mean(g),
		    gauden_n_mgau(g),
		    gauden_n_feat(g),
		    gauden_n_density(g),
		    gauden_veclen(g)) != S3_SUCCESS) {
	E_FATAL_SYSTEM("could not write means file\n");
    }
    
    if (cmd_ln_access("-varfn")) {
	E_INFO("Writing %s\n",
	       cmd_ln_access("-varfn"));

	if (s3gau_write((const char *)cmd_ln_access("-varfn"),
			(const vector_t ***)gauden_var(g),
			gauden_n_mgau(g),
			gauden_n_feat(g),
			gauden_n_density(g),
			gauden_veclen(g)) != S3_SUCCESS) {
	    E_FATAL_SYSTEM("could not write var file\n");
	}
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:14  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  97/07/16  11:24:05  eht
 * Changes for new I/O routines
 * 
 * Revision 1.1  97/03/07  08:56:56  eht
 * Initial revision
 * 
 *
 */
