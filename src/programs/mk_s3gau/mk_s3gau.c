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
 * File: mk_s3gau.c
 * 
 * Description: 
 * 	Conversion from SPHINX-II codebooks to SPHINX-III mean and
 *	variance files.
 *	
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/gauden.h>
#include <s3/s3gau_io.h>
#include <s3/s2_param.h>
#include <s3/s2_read_cb.h>
#include <s3/err.h>
#include <sphinxbase/cmd_ln.h>

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

    g = gauden_alloc();

    gauden_set_min_var(cmd_ln_float32("-varfloor"));

    cb_basename[0] = cmd_ln_str("-cepcb");
    cb_basename[1] = cmd_ln_str("-dcepcb");
    cb_basename[2] = cmd_ln_str("-powcb");
    cb_basename[3] = cmd_ln_str("-2dcepcb");

    s2_read_cb(g,
	       cmd_ln_str("-cbdir"),
	       cb_basename,
	       cmd_ln_str("-meanext"),
	       cmd_ln_str("-varext"),
	       TRUE,	/* floor variances */
	       cmd_ln_str("-fixpowvar"));

    t = time(NULL);

    sprintf(comment,
	    "Generated on %s\nby %s.\nFrom codebooks in %s\n",
	    ctime(&t),
	    argv[0],
	    cmd_ln_str("-cbdir"));
    
    E_INFO("writing %s\n", cmd_ln_str("-meanfn"));

    if (s3gau_write(cmd_ln_str("-meanfn"),
		    (const vector_t ***)gauden_mean(g),
		    gauden_n_mgau(g),
		    gauden_n_feat(g),
		    gauden_n_density(g),
		    gauden_veclen(g)) != S3_SUCCESS) {
	E_FATAL_SYSTEM("could not write means file\n");
    }
    
    if (cmd_ln_str("-varfn")) {
	E_INFO("Writing %s\n",
	       cmd_ln_str("-varfn"));

	if (s3gau_write(cmd_ln_str("-varfn"),
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
 * Revision 1.5  2004/07/21  19:17:25  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.4  2004/06/17 19:17:23  arthchan2003
 * Code Update for silence deletion and standardize the name for command -line arguments
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
 * Revision 1.2  97/07/16  11:24:05  eht
 * Changes for new I/O routines
 * 
 * Revision 1.1  97/03/07  08:56:56  eht
 * Initial revision
 * 
 *
 */
