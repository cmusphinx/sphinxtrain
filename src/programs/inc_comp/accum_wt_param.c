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
 * File: accum_wt_param.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "accum_wt_param.h"

#include <s3/s3gau_io.h>
#include <s3/gauden.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/matrix.h>
#include <s3/s3.h>

#include <sys_compat/file.h>

int
accum_wt_param(char **accum_dir,
	       vector_t ****out_wt_mean,
	       vector_t ****out_wt_var,
	       int32 *out_pass2var,
	       float32 ****out_dnom,
	       uint32 *out_n_mgau,
	       uint32 *out_n_feat,
	       uint32 *out_n_density)
{
    uint32 i;
    char fn[MAXPATHLEN];
    
    vector_t ***in_wt_mean;
    vector_t ***in_wt_var;
    float32 ***in_dnom;

    vector_t ***acc_mean = 0;
    vector_t ***acc_var = 0;
    float32 ***acc_dnom = 0;

    uint32 n_mgau = 0;
    uint32 t_n_mgau;
    uint32 n_feat = 0;
    uint32 t_n_feat;
    uint32 n_density = 0;
    uint32 t_n_density;
    uint32 *t_veclen;
    uint32 *veclen = NULL;
    int32 pass2var;

    for (i = 0; accum_dir[i] != NULL; i++) {
	sprintf(fn, "%s/gauden_counts", accum_dir[i]);

	if (access(fn, F_OK) == 0) {
	    E_INFO("Reading gaussian density count file %s\n", fn);

	    if (s3gaucnt_read(fn,
			      &in_wt_mean,
			      &in_wt_var,
			      &pass2var,
			      &in_dnom,
			      &t_n_mgau,
			      &t_n_feat,
			      &t_n_density,
			      &t_veclen) != S3_SUCCESS) {
		fflush(stdout);
		perror(fn);
		
		return S3_ERROR;
	    }

	    if (n_mgau == 0) {
		n_mgau = t_n_mgau;
		n_feat = t_n_feat;
		n_density = t_n_density;
		veclen = t_veclen;

		acc_mean = gauden_alloc_param(n_mgau, n_feat, n_density, veclen);
		acc_var  = gauden_alloc_param(n_mgau, n_feat, n_density, veclen);
		acc_dnom = (float32 ***)ckd_calloc_3d(n_mgau,
						      n_feat,
						      n_density,
						      sizeof(float32));
	    }
	    else {
		if (n_mgau != t_n_mgau) {
		    E_ERROR("n_mgau, %u, in %s is inconsistent w/ the rest, %u.\n",
			    t_n_mgau, fn, n_mgau);
		}
		if (n_feat != t_n_feat) {
		    E_ERROR("n_feat, %u, in %s is inconsistent w/ the rest, %u.\n",
			    t_n_feat, fn, n_feat);
		}
		if (n_density != t_n_density) {
		    E_ERROR("n_density, %u, in %s is inconsistent w/ the rest, %u.\n",
			    t_n_density, fn, n_density);
		}

		ckd_free(t_veclen);
	    }
	}
	else {
	    in_wt_var = in_wt_mean = NULL;
	    in_dnom = NULL;
	    t_veclen = NULL;
	}

	if (in_wt_mean == NULL) {
	    E_ERROR("Could not get weighted means for accumdir %s\n", accum_dir[i]);

	    exit(1);
	}

	if (in_wt_var == NULL) {
	    E_ERROR("Need weighted variances in accumdir %s to increment densities\n", accum_dir[i]);

	    exit(1);
	}

	/* accumulate the parameters read in from the count file */
	gauden_accum_param(acc_mean, in_wt_mean,
			   n_mgau, n_feat, n_density, veclen);
	ckd_free((void *)in_wt_mean[0][0][0]);
	ckd_free_3d((void ***)in_wt_mean);

	gauden_accum_param(acc_var, in_wt_var,
			   n_mgau, n_feat, n_density, veclen);
	ckd_free((void *)in_wt_var[0][0][0]);
	ckd_free_3d((void ***)in_wt_var);

	accum_3d(acc_dnom, in_dnom,
		 n_mgau, n_feat, n_density);
	ckd_free_3d((void ***)in_dnom);
    }

    /* pass outputs to the caller */
    *out_wt_mean = acc_mean;
    *out_wt_var  = acc_var;
    *out_dnom    = acc_dnom;
    *out_n_mgau  = n_mgau;
    *out_n_feat  = n_feat;
    *out_n_density = n_density;
    *out_pass2var = pass2var;

    return S3_SUCCESS;
}
