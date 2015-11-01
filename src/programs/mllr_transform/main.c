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
#include <s3/s3.h>
#include <sphinxbase/err.h>

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

static int
read_mllr_files(const char *mllrmatfn,
		const char *cb2mllrfn,
		float32 *****out_A,
		float32 ****out_B,
		int32 **out_cb2mllr,
		uint32 *out_n_mllr_class,
		uint32 *out_n_mgau,
		uint32 *out_n_feat,
		uint32 **out_veclen,
		uint32 inverse)
{
    uint32 n_mllr_class_rd, m, j;

    E_INFO("Read %s\n", mllrmatfn);
    if (read_reg_mat(mllrmatfn,
	 	     out_veclen,
		     out_n_mllr_class,
		     out_n_feat,
		     out_A,
		     out_B) != S3_SUCCESS) {
	E_FATAL("Unable to read %s\n",mllrmatfn);
    }
    E_INFO("n_mllr_class = %d\n", *out_n_mllr_class);

    if (inverse) {
	for (m = 0; m < *out_n_mllr_class; ++m) {
	    for (j = 0; j < *out_n_feat; ++j) {
		invert((*out_A)[m][j], (*out_A)[m][j], (*out_veclen)[j]);
	    }
	}
    }

    if (strcmp(cb2mllrfn, ".1cls.") == 0) {
	n_mllr_class_rd = 1;
        *out_cb2mllr = NULL;
	*out_n_mgau = 0;
    }
    else {
	if (s3cb2mllr_read(cb2mllrfn,
			   out_cb2mllr,
			   out_n_mgau,
			   &n_mllr_class_rd) != S3_SUCCESS) {
	    E_FATAL("Unable to read %s\n", cb2mllrfn);
	}
    }
    if (n_mllr_class_rd != *out_n_mllr_class) {
	E_FATAL("# MLLR class, %u, inconsistent w/ cb2mllr # MLLR class, %u\n",
		n_mllr_class_rd, *out_n_mllr_class);
    }
    return S3_SUCCESS;
}

static int
read_mdef(const char *moddeffn,
	  model_def_t **out_mdef,
	  uint32 *out_gau_begin,
	  int32 *cb2mllr)
{
    uint32 i;

    if (! moddeffn) {
	E_FATAL("-moddeffn is not given.\n");
    }
    else if (model_def_read(out_mdef, moddeffn) != S3_SUCCESS) {
	E_FATAL("Can not read model definition file %s\n", moddeffn);
    }

    if (cb2mllr == NULL)
	cb2mllr = (int32 *)ckd_calloc((*out_mdef)->n_total_state,sizeof(char));
	
    *out_gau_begin = (*out_mdef)->n_tied_ci_state;
    for (i = 0; i < *out_gau_begin; i++) {
	cb2mllr[i] = -1;                    /* skip CI senones */
    }
    E_INFO("Use CD senones only. (index >= %d)\n",(*out_mdef)->n_tied_ci_state);

    return S3_SUCCESS;
}

static int
mllr_adapt_mean(const char *outmeanfn,
		const char *inmeanfn,
	        const char *mllrmatfn,
	        const char *cb2mllrfn,
	        const char *moddeffn,
		uint32	   cdonly,
		uint32     inverse)
{
    model_def_t  *mdef = NULL;
    vector_t 	 ***mean = NULL;

    uint32 	 n_mgau;
    uint32 	 n_feat;
    uint32 	 n_density;
    uint32 	 *veclen = NULL;

    uint32 	 n_mgau_rd;
    uint32 	 n_feat_rd;
    uint32 	 *veclen_rd = NULL;

    uint32  	 n_mllr_class;

    int32  	 *cb2mllr = NULL;
    float32 	 ****A    = NULL;
    float32 	 ***B     = NULL;
    uint32 	 j;
    uint32	 gau_begin;


    read_mllr_files(mllrmatfn, cb2mllrfn,
		    &A, &B, &cb2mllr,
		    &n_mllr_class,
		    &n_mgau,
		    &n_feat,
		    &veclen, inverse);

    /* Read input means */
    fprintf(stderr,"\n");
    if (s3gau_read(inmeanfn,
		   &mean,
		   &n_mgau_rd,
		   &n_feat_rd,
		   &n_density,
		   &veclen_rd) != S3_SUCCESS) {
	E_FATAL("Unable to read Gaussian means from %s\n",inmeanfn);
    }
    if (n_mgau == 0)
	n_mgau = n_mgau_rd;

    if (n_mgau_rd != n_mgau) {
	E_FATAL("# means %u inconsistent w/ cb2mllr # cb %u\n", n_mgau_rd, n_mgau);
    }
    else if (n_feat_rd != n_feat) {
        E_FATAL("# feature stream inconsistent. (%u : %u)\n",n_feat_rd,n_feat);
    }
    else {
        for (j=0; j < n_feat; j++) {
            if (veclen_rd[j] != veclen[j]) {
                E_FATAL("veclen inconsistent. (%u : %u)\n",veclen_rd[j],veclen[j]);
            }
        }
    }
    ckd_free(veclen_rd);

    gau_begin = 0;
    if (cdonly)
	read_mdef(moddeffn, &mdef, &gau_begin, cb2mllr);
    
    fprintf(stderr,"\n");
    E_INFO("Adapt mean values.\n");

    mllr_transform_mean(mean, NULL, gau_begin, n_mgau,
			n_feat, n_density, veclen,
			A, B, cb2mllr, n_mllr_class);

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

static int
mllr_adapt_gaucnt(const char *outgaufn,
		  const char *ingaufn,
		  const char *mllrmatfn,
		  const char *cb2mllrfn,
		  const char *moddeffn,
		  uint32 cdonly,
		  uint32 inverse)
{
    model_def_t  *mdef = NULL;
    vector_t 	 ***wt_mean = NULL;
    vector_t 	 ***wt_var = NULL;
    int32	 pass2var;
    float32	 ***dnom = NULL;
	
    uint32 	 n_mgau;
    uint32 	 n_feat;
    uint32 	 n_density;
    uint32 	 *veclen = NULL;

    uint32 	 n_mgau_rd;
    uint32 	 n_feat_rd;
    uint32 	 *veclen_rd = NULL;

    uint32  	 n_mllr_class;
    uint32	 gau_begin;

    int32  	 *cb2mllr = NULL;
    float32 	 ****A    = NULL;
    float32 	 ***B     = NULL;
    uint32 	 j;

    read_mllr_files(mllrmatfn, cb2mllrfn,
		    &A, &B, &cb2mllr,
		    &n_mllr_class,
		    &n_mgau,
		    &n_feat,
		    &veclen, inverse);

    fprintf(stderr,"\n");
    if (s3gaucnt_read(ingaufn,
		      &wt_mean,
		      &wt_var,
		      &pass2var,
		      &dnom,
		      &n_mgau_rd,
		      &n_feat_rd,
		      &n_density,
		      &veclen_rd) != S3_SUCCESS) {
	E_FATAL("Unable to read Gaussian counts from %s\n", ingaufn);
    }
    if (n_mgau == 0)
	n_mgau = n_mgau_rd;

    if (n_mgau_rd != n_mgau) {
	E_FATAL("# means %u inconsistent w/ cb2mllr # cb %u\n", n_mgau_rd, n_mgau);
    }
    else if (n_feat_rd != n_feat) {
        E_FATAL("# feature stream inconsistent. (%u : %u)\n",n_feat_rd,n_feat);
    }
    else {
        for (j=0; j < n_feat; j++) {
            if (veclen_rd[j] != veclen[j]) {
                E_FATAL("veclen inconsistent. (%u : %u)\n",veclen_rd[j],veclen[j]);
            }
        }
    }
    ckd_free(veclen_rd);

    gau_begin = 0;
    if (cdonly)
	read_mdef(moddeffn, &mdef, &gau_begin, cb2mllr);

    fprintf(stderr,"\n");
    E_INFO("Adapt mean values.\n");

    mllr_transform_mean(wt_mean, NULL, 0, n_mgau,
			n_feat, n_density, veclen,
			A, B, cb2mllr, n_mllr_class);

    /*----- 7. Write to outmeanfn  -----*/

    fprintf(stderr,"\n");
    if (s3gaucnt_write(outgaufn,
		       wt_mean,
		       wt_var,
		       pass2var,
		       dnom,
		       n_mgau,
		       n_feat,
		       n_density,
		       veclen) != S3_SUCCESS) {
	E_FATAL("Unable to write Gaussian counts to %s\n",outgaufn);
    }
    
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    const char 	*outmeanfn, *inmeanfn;
    const char 	*outgaufn, *ingaufn;
    const char 	*mllrmatfn, *cb2mllrfn, *moddeffn;
    uint32     	cdonly, inverse;

    printf("----- Compiled on %s, %s\n\n",__DATE__,__TIME__);

    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("Errors initializing.\n");
	return 0;
    }

    outmeanfn = cmd_ln_str("-outmeanfn");
    inmeanfn  = cmd_ln_str("-inmeanfn");
    outgaufn  = cmd_ln_str("-outgaucntfn");
    ingaufn   = cmd_ln_str("-ingaucntfn");
    mllrmatfn = cmd_ln_str("-mllrmat");
    cb2mllrfn = cmd_ln_str("-cb2mllrfn");
    moddeffn  = cmd_ln_str("-moddeffn");
    cdonly    = cmd_ln_int32("-cdonly");
    inverse   = cmd_ln_int32("-inverse");

    if (! (mllrmatfn && cb2mllrfn)) {
	E_FATAL("You must specify all mllr files\n");
    }
    if (outmeanfn) {
	if (mllr_adapt_mean(outmeanfn,
			    inmeanfn,
			    mllrmatfn,
			    cb2mllrfn,
			    moddeffn,
			    cdonly,
			    inverse) == S3_SUCCESS) {
	    E_INFO("mllr_adapt finished successfully.\n");
	    return(0);
	}
	else {
	    E_FATAL("Error in mllr_adapt_mean().\n");
	    return (1);
	}
    }
    else if (outgaufn) {
	if (mllr_adapt_gaucnt(outgaufn,
			      ingaufn,
			      mllrmatfn,
			      cb2mllrfn,
			      moddeffn,
			      cdonly,
			      inverse) == S3_SUCCESS) {
	    E_INFO("mllr_adapt finished successfully.\n");
	    return(0);
	}
	else {
	    E_FATAL("Error in mllr_adapt_mean().\n");
	    return (1);
	}
    }
    else {
	E_FATAL("No -outmeanfn or -outgaucntfn given, nothing done.\n");
    }
    return 0 ;
}
