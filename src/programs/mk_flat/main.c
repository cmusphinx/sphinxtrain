/*********************************************************************
 *
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/s2_read_seno.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s2_param.h>
#include <s3/topo_read.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#define NO_STATE	0xffffffff

static char *ord_suff(uint32 i)
{
    return (i == 1 ? "st" :
	    (i == 2 ? "nd" :
	     (i == 3 ? "rd" : "th")));
}

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
	   __FILE__, __LINE__, (const char *)cmd_ln_access("-moddeffn"));
    
    if (model_def_read(&mdef, cmd_ln_access("-moddeffn")) !=
	S3_SUCCESS) {
	return 1;
    }
    printf("%s(%d): %d models defined\n",
	   __FILE__, __LINE__, mdef->n_defn);

    if (cmd_ln_access("-tmatfn")) {
	if (topo_read(&proto_tmat, &n_state_pm, cmd_ln_access("-topo")) != S3_SUCCESS)
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

	if (s3tmat_write(cmd_ln_access("-tmatfn"),
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
    n_stream = *(int32 *)cmd_ln_access("-nstream");
    n_density = *(int32 *)cmd_ln_access("-ndensity");

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

    if (s3mixw_write(cmd_ln_access("-mixwfn"),
		     mixw,
		     n_tied_state,
		     n_stream,
		     n_density) != S3_SUCCESS) {
	retval = 2;
    }

    ckd_free_3d((void ***)mixw);
    
    return retval;
}



