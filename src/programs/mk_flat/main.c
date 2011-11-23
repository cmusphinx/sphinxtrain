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
 * 	(probably) Eric H. Thayer
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/model_def_io.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/topo_read.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#define NO_STATE	0xffffffff

int
main(int argc, char *argv[])
{
    model_def_t *mdef;
    uint32 n_tmat;
    uint32 n_tied_state;
    uint32 n_state_pm;
    uint32 n_stream;
    uint32 n_density;
    float32 ***tmat;
    float32 **proto_tmat;
    float32 ***mixw;
    uint32 i, j, k;
    float32 mixw_ini;
    int retval = 0;

    parse_cmd_ln(argc, argv);
    
    printf("%s(%d): Reading model definition file %s\n",
	   __FILE__, __LINE__, cmd_ln_str("-moddeffn"));
    
    if (model_def_read(&mdef, cmd_ln_str("-moddeffn")) !=
	S3_SUCCESS) {
	return 1;
    }
    printf("%s(%d): %d models defined\n",
	   __FILE__, __LINE__, mdef->n_defn);

    if (!cmd_ln_str("-tmatfn") && ! cmd_ln_str("-mixwfn")){
        E_FATAL("Both -tmatfn and -mixwfn were not specified, forced exit\n");
    }

    if (cmd_ln_str("-tmatfn")) {
	if (topo_read(&proto_tmat, &n_state_pm, cmd_ln_str("-topo")) != S3_SUCCESS)
	    return 1;

	/* proto_tmat is normalized */
	
	n_tmat = mdef->n_tied_tmat;
	tmat = (float32 ***)ckd_calloc_3d(n_tmat, n_state_pm-1, n_state_pm,
					  sizeof(float32));
	
	for (k = 0; k < n_tmat; k++) {
	    for (i = 0; i < n_state_pm-1; i++) {
		for (j = 0; j < n_state_pm; j++) {
		    /* perhaps this could be replaced
		       with a block copy per tmat */
		    tmat[k][i][j] = proto_tmat[i][j];
		}
	    }
	}

	if (s3tmat_write(cmd_ln_str("-tmatfn"),
			 tmat,
			 n_tmat,
			 n_state_pm) != S3_SUCCESS) {
	    retval = 1;
	}

	ckd_free_3d((void ***)tmat);
    }
    else {
	E_INFO("No tmat file given; none generated\n");
    }

    n_tied_state = mdef->n_tied_state;
    n_stream = cmd_ln_int32("-nstream");
    n_density = cmd_ln_int32("-ndensity");

    mixw = (float32 ***)ckd_calloc_3d(n_tied_state, n_stream, n_density,
				      sizeof(float32));

    /* weight each density uniformly */
    mixw_ini = 1.0 / (float)n_density;

    for (i = 0; i < n_tied_state; i++) {
	for (j = 0; j < n_stream; j++) {
	    for (k = 0; k < n_density; k++) {
		mixw[i][j][k] = mixw_ini;
	    }
	}
    }

    if (cmd_ln_str("-mixwfn")) {
        if (s3mixw_write(cmd_ln_str("-mixwfn"),
	  	         mixw,
		         n_tied_state,
		         n_stream,
		         n_density) != S3_SUCCESS) {
	  retval = 2;
	}
    } 
    else {
      E_INFO("No mixw file given; none generated\n");
    }

    ckd_free_3d((void ***)mixw);
    
    return retval;
}



