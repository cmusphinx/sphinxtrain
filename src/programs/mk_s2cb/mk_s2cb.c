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
 *
 * File: mk_s2cb.c
 * 
 * Description: 
 *     Make SPHINX-II codebook files given SPHINX-III format
 *     means/var files.
 *
 * Author: 
 *     Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/
#include "parse_cmd_ln.h"

#include <s3/gauden.h>
#include <s3/s3gau_io.h>
#include <s3/s2_param.h>
#include <s3/s2_write_cb.h>
#include <s3/cmd_ln.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

int main(int argc, char *argv[])
{
    gauden_t *g;
    const char *cb_basename[4];
    char comment[8192];
    time_t t;
    vector_t ***mean;
    vector_t ***var;
    uint32 n_mgau;
    uint32 n_feat;
    uint32 n_density;
    const uint32 *veclen;
    const uint32 *vl;
    uint32 i, j, k;

    parse_cmd_ln(argc, argv);

    g = gauden_alloc();

    /* 
     * Read the new model
     */

    E_INFO("Reading means in %s\n", cmd_ln_access("-meanfn"));
    if (s3gau_read(cmd_ln_access("-meanfn"),
		    &mean,
		    &n_mgau,
		    &n_feat,
		    &n_density,
		    &veclen) != S3_SUCCESS) {
	E_ERROR_SYSTEM("Could not read means file");
	exit(1);
    }

    if (cmd_ln_access("-varfn")) {
	E_INFO("Reading variances in %s\n", cmd_ln_access("-varfn"));

	if (s3gau_read(cmd_ln_access("-varfn"),
			&var,
			&i,
			&j,
			&k,
			&vl) != S3_SUCCESS) {
	    E_ERROR_SYSTEM("Could not read variance file");
	    exit(1);
	}

	if (i != n_mgau) {
	    E_FATAL("mean n_mgau %u inconsistent w/ var n_mgau %u\n",
		    n_mgau, i);
	}
	if (j != n_feat) { 
	    E_FATAL("mean n_feat %u inconsistent w/ var n_feat %u\n",
		    n_feat, j);
	}
	if (k != n_density) { 
	    E_FATAL("mean n_density %u inconsistent w/ var n_density %u\n",
		    n_density, k);
	}
	for (i = 0; i < n_feat; i++) {
	    if (vl[i] != veclen[i]) {
		E_FATAL("feature %u vector length inconsistent between means and variances (%u != %u)\n",
			i, veclen[i], vl[i]);
	    }
	}
    }
    else
	var = NULL;

    gauden_set_n_mgau(g, n_mgau);
    gauden_set_feat(g, n_feat, veclen);
    gauden_set_n_density(g, n_density);
    /* gauden_set_n_top(g, *(int32 *)cmd_ln_access("-topn")); */

    gauden_set_mean(g, mean);
    gauden_set_min_var(*(float32 *)cmd_ln_access("-varfloor"));

	  /* MAKE SURE min_var is set, or else this will trash your data!!!! */
    gauden_set_var(g, var);


    /*
     * Output it in the old format.
     */


    cb_basename[0] = cmd_ln_access("-cepcb");
    cb_basename[1] = cmd_ln_access("-dcepcb");
    cb_basename[2] = cmd_ln_access("-powcb");
    cb_basename[3] = cmd_ln_access("-2dcepcb");

    s2_write_cb(g,
		cmd_ln_access("-cbdir"),
		cb_basename,
		cmd_ln_access("-meanext"),
		cmd_ln_access("-varext"),
		TRUE);		/* floor variances */

    t = time(NULL);

    sprintf(comment,
	    "Generated on %s\nby %s.\nFrom codebooks in %s\n",
	    ctime(&t),
	    argv[0],
	    (char *)cmd_ln_access("-cbdir"));
    

    exit(0);
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/11/22 21:23:18  awb
 * *** empty log message ***
 *
 * Revision 1.5  97/07/16  11:31:23  eht
 * Add comments and use updated library functions
 * 
 *
 */
