/*********************************************************************
 *
 * $Header: 
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Modify Gaussian mean vector using mllr (the output of mllr_mat)
 *
 * Modified: sjdoh@cs, 990315
 *
 * Copyright (c) 1996 -2004Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************/

/*
 * 2004-07-25 ARCHAN: first adapted from Sam Joo's program to SphinxTrain. 
 */

#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3gau_io.h>
#include <s3/gauden.h>
#include <s3/mllr.h>
#include <s3/mllr_io.h>
#include <s3/s3cb2mllr_io.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s3.h>
#include <s3/err.h>

#include <sys_compat/file.h>
#include <sys_compat/misc.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define MAX2(x,y) 	((x) > (y) ? (x):(y))
#define ABS(x) 		((x) < 0. ? (-(x)):(x))

static int
initialize(int argc,
	   char *argv[])
{
    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);
    
    return S3_SUCCESS;
}


/* ARCHAN 20040726 : I kept all Sam Joo's steps mark in the comment. I think they are pretty neat.  */
static int
mllr_adapt_mean(const char *outmeanfn,
		const char *inmeanfn,
	        const char *mllrmatfn,
	        const char *cb2mllrfn,
	        const char *moddeffn,
	        const char *varfn,
		float32	   varfloor,
		uint32	   cdonly)
{
    model_def_t  *mdef = NULL;
    vector_t 	 ***mean = NULL;
    vector_t 	 ***var = NULL;			/* for smoothing the result (hack!!!) */

    uint32 	 n_mgau;
    uint32 	 n_feat;
    uint32 	 n_density;
    const uint32 *veclen = NULL;

    uint32 	 n_mgau_rd;
    uint32 	 n_feat_rd;
    uint32 	 n_density_rd;
    const uint32 *veclen_rd = NULL;

    uint32  	 n_mllr_class;
    uint32  	 n_mllr_class_rd;

    int32	 mc;
    int32  	 *cb2mllr = NULL;
    float32 	 ****A    = NULL;
    float32 	 ***B     = NULL;
    float32 	 *tmean    = NULL;
    uint32 	 i, j, k, l, m;
    uint32	 gau_begin;


    /*----- 1. Read input mean -----*/

    fprintf(stderr,"\n");
    if (s3gau_read(inmeanfn,
		   &mean,
		   &n_mgau,
		   &n_feat,
		   &n_density,
		   &veclen) != S3_SUCCESS) {
	E_FATAL("Unable to read Gaussian means from %s\n",inmeanfn);
    }

    /*----- 2. Read input var for smoothing output mean -----*/

    if (varfn) {
        fprintf(stderr,"\n");
        E_INFO("Smoothing output mean values by using var (HACK!!!).\n");

	if (s3gau_read(varfn,
		       &var,
		       &n_mgau_rd,
		       &n_feat_rd,
		       &n_density_rd,
		       &veclen_rd) != S3_SUCCESS) {
	    E_FATAL("Unable to read Gaussian var from %s\n",varfn);
	}
        if (n_mgau_rd != n_mgau) {
	    E_FATAL("# means inconsistent. (%u : %u)\n", n_mgau_rd, n_mgau);
        }
        if (n_feat_rd != n_feat) {
	    E_FATAL("n_feat inconsistent. (%u : %u)\n", n_feat_rd, n_feat);
        }
        if (n_density_rd != n_density) {
	    E_FATAL("n_density inconsistent. (%u : %u)\n", n_density_rd, n_density);
        }
	for (j=0; j < n_feat; j++) {
            if (veclen_rd[j] != veclen[j]) {
	        E_FATAL("veclen inconsistent. (%u : %u)\n", veclen_rd[j], veclen[j]);
            }
	}
        ckd_free((void *)veclen_rd);


        for (i = 0; i < n_mgau; i++) {
	    for (j = 0; j < n_feat; j++) {
	        for (k = 0; k < n_density; k++) {
		    for (l = 0; l < veclen[j]; l++) {
                        if (var[i][j][k][l] < varfloor) var[i][j][k][l] = varfloor;
		    }
		}
	    }
	}
    }


    /*----- 3. Read mllr matrices -----*/

    E_INFO("Read %s\n", mllrmatfn);
    if (read_reg_mat(mllrmatfn,
	 	     &veclen_rd,
		     &n_mllr_class,
		     &n_feat_rd,
		     &A,
		     &B) != S3_SUCCESS) {
	E_FATAL("Unable to read %s\n",mllrmatfn);
    }
    E_INFO("n_mllr_class = %d\n", n_mllr_class);

    if (n_feat_rd != n_feat) {
        E_FATAL("# feature stream inconsistent. (%u : %u)\n",n_feat_rd,n_feat);
    }
    else {
        for (j=0; j < n_feat; j++) {
            if (veclen_rd[j] != veclen[j]) {
                E_FATAL("veclen inconsistent. (%u : %u)\n",veclen_rd[j],veclen[j]);
            }
        }
    }
    ckd_free((void *)veclen_rd);


    /*----- 4. Read cb2mllr -----*/

    if (strcmp(cb2mllrfn, ".1cls.") == 0) {
	n_mllr_class_rd = 1;
        cb2mllr = ckd_calloc(n_mgau, sizeof(int32));
    }
    else {
	if (s3cb2mllr_read(cb2mllrfn,
			   &cb2mllr,
			   &n_mgau_rd,
			   &n_mllr_class_rd) != S3_SUCCESS) {
	    E_FATAL("Unable to read %s\n", cb2mllrfn);
	}
        if (n_mgau_rd != n_mgau) {
	    E_FATAL("# means %u inconsistent w/ cb2mllr # cb %u\n", n_mgau_rd, n_mgau);
        }
    }
    if (n_mllr_class_rd != n_mllr_class) {
	E_FATAL("# MLLR class, %u, inconsistent w/ cb2mllr # MLLR class, %u\n",
		n_mllr_class_rd, n_mllr_class);
    }

    gau_begin = 0;

    if (cdonly) {
        if (! moddeffn) {
            E_FATAL("-moddeffn is not given.\n");
        }
        else if (model_def_read(&mdef, moddeffn) != S3_SUCCESS) {
            E_FATAL("Can not read model definition file %s\n", moddeffn);
        }
	gau_begin = mdef->n_tied_ci_state;
        for (i=0; i<gau_begin; i++) {
            cb2mllr[i] = -1;                    /* skip CI senones */
        }
        E_INFO("Use CD senones only. (index >= %d)\n",mdef->n_tied_ci_state);
    }

    
    /*----- 4. Adapt mean -----*/
    fprintf(stderr,"\n");
    E_INFO("Adapt mean values.\n");

    for (i = gau_begin; i < n_mgau; i++) {
	mc = cb2mllr[i];
	if (mc < 0) continue;	/* skip */

	for (j = 0; j < n_feat; j++) {
	    tmean = (float32 *)ckd_calloc(veclen[j],sizeof(float32));

	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    tmean[l] = 0;
		    for (m = 0; m < veclen[j]; m++) {
			tmean[l] += A[mc][j][l][m] * mean[i][j][k][m];

		    }
		    tmean[l] += B[mc][j][l];
		}

		/* Write back the transformed mean vector */
		for (l = 0; l < veclen[j]; l++) 
		  mean[i][j][k][l] = tmean[l];
		
	    }
	    ckd_free(tmean);  tmean = NULL;
	}
    }
	

    /*----- 7. Write to outmeanfn  -----*/

    fprintf(stderr,"\n");
    if (s3gau_write(outmeanfn,
		    (const vector_t ***)mean,
		    n_mgau,
		    n_feat,
		    n_density,
		    veclen) != S3_SUCCESS) {
	E_FATAL("Unable to write Gaussian means to %s\n",outmeanfn);
    }

    
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    const char 	*outmeanfn;
    const char 	*inmeanfn, *mllrmatfn, *cb2mllrfn, *moddeffn;
    const char 	*varfn;
    float32	varfloor;
    uint32     	cdonly;

    printf("----- Compiled on %s, %s\n\n",__DATE__,__TIME__);

    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("Errors initializing.\n");
	return 0;
    }

    outmeanfn = (const char *)cmd_ln_access("-outmeanfn");
    inmeanfn  = (const char *)cmd_ln_access("-inmeanfn");
    mllrmatfn = (const char *)cmd_ln_access("-mllrmat");
    cb2mllrfn = (const char *)cmd_ln_access("-cb2mllrfn");
    moddeffn  = (const char *)cmd_ln_access("-moddeffn");
    varfn     = (const char *)cmd_ln_access("-varfn");
    varfloor  = *(float32 *)cmd_ln_access("-varfloor");
    cdonly    = *(uint32 *)cmd_ln_access("-cdonly");

    if (! (outmeanfn && inmeanfn && mllrmatfn && cb2mllrfn)) {
	E_FATAL("You must specify all of input, mllr and output files\n");
    }
    
    if (mllr_adapt_mean(outmeanfn,
			inmeanfn,
			mllrmatfn,
			cb2mllrfn,
			moddeffn,
			varfn,
			varfloor,
			cdonly) == S3_SUCCESS) {
        E_INFO("mllr_adapt finished successfully.\n");
        return(0);
    }
    else {
	E_FATAL("Error in mllr_adapt_mean().\n");
	return (1);
    }
    return 0 ;
}
