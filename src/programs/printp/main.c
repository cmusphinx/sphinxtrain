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
 *	This prints out the various parameter files.
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3lamb_io.h>
#include <s3/s3regmat_io.h>

#include <sys_compat/file.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>

#include <s3/s3.h>
#include <s3/err.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#

static int print(void);

static char e_fmt[20];
static char e_fmt_ld_sp[20];
 

static int
initialize(int argc,
	   char *argv[])
{
    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    sprintf(e_fmt, "%%.%de ", *(int32 *)cmd_ln_access("-sigfig") - 1);
    sprintf(e_fmt_ld_sp, " %%.%de", *(int32 *)cmd_ln_access("-sigfig") - 1);
    
    return S3_SUCCESS;
}

int
print_lambda(const char *fn, float32 min, float32 max)
{
    float32 *lmb;
    uint32 n_lmb;
    uint32 i;

    E_INFO("Reading %s\n",  fn);

    if (s3lamb_read(fn, &lmb, &n_lmb) != S3_SUCCESS)
	return S3_ERROR;

    for (i = 0; i < n_lmb; i++) {
	if ((lmb[i] >= min) && (lmb[i] <= max))
	    printf("%u: %0.4f\n", i, lmb[i]);
    }

    return S3_SUCCESS;
}

int
print_gau(const char *fn)
{
    vector_t ***mean;
    uint32 n_mgau;
    uint32 n_feat;
    const uint32 *veclen;
    uint32 n_density;
    uint32 i, j, k, l;

    E_INFO("Reading %s\n",  fn);
    
    if (s3gau_read(fn,
		   &mean,
		   &n_mgau,
		   &n_feat,
		   &n_density,
		   &veclen) != S3_SUCCESS)
	return S3_ERROR;

    printf("param %u %u %u\n", n_mgau, n_feat, n_density);

    for (i = 0; i < n_mgau; i++) {
	printf("mgau %u\n", i);
	for (j = 0; j < n_feat; j++) {
	    printf("feat %u\n", j);
	    for (k = 0; k < n_density; k++) {
		printf("density %4u ", k);
		for (l = 0; l < veclen[j]; l++) {
		    printf(e_fmt, mean[i][j][k][l]);
		}
		printf("\n");
	    }
	}
    }
    return S3_SUCCESS;
}

int
print_regmat_cnt(const char *fn)
{
    float32 ****regr;
    float32 *****regl;
    uint32 n_mllr_class;
    uint32 n_feat;
    const uint32 *veclen;
    uint32 mllr_mult;
    uint32 mllr_add;
    uint32 c;
    uint32 f;
    uint32 i, j, k, l;

    if (s3regmatcnt_read(fn,
			 &regr,
			 &regl,
			 &n_mllr_class,
			 &n_feat,
			 &veclen,
			 &mllr_mult,
			 &mllr_add) != S3_SUCCESS) {
	return S3_ERROR;
    }

    for (c = 0; c < n_mllr_class; c++) {
	for (f = 0; f < n_feat; f++) {
	    l = veclen[f];
	    for (i = 0; i < l; i++) {
		printf("regmat=(%u %u %u)\n", c, f, i);
		for (j = 0; j <= l; j++) {
		    printf("%2u:", j);
		    printf(e_fmt_ld_sp, regr[c][f][i][j]);
		    printf("r");
		    for (k = 0; k <= l; k++) {
			printf(e_fmt_ld_sp, regl[c][f][i][j][k]);
		    }
		    printf("\n");
		}
	    }
	}
    }
    ckd_free((void *)veclen);

    for (c = 0; c < n_mllr_class; c++) {
	for (f = 0; f < n_feat; f++) {
	    ckd_free_2d ((void **)regr[c][f]);
	    ckd_free_3d ((void ***)regl[c][f]);
	}
    }
    
    ckd_free_2d ((void **)regr);
    ckd_free_2d ((void **)regl);

    return S3_SUCCESS;
}

int
print_gau_cnt(const char *fn)
{
    vector_t ***wt_mean;
    vector_t ***wt_var;
    float32  ***dnom;
    uint32 n_mgau;
    uint32 n_feat;
    uint32 n_density;
    const uint32 *veclen;
    uint32 i, j, k, l;
    int32 pass2var;
    char mv_fmt[64];
    int32 sf;

    if (s3gaucnt_read(fn,
		      &wt_mean,
		      &wt_var,
		      &pass2var,
		      &dnom,
		      &n_mgau,
		      &n_feat,
		      &n_density,
		      &veclen) != S3_SUCCESS) {
	fflush(stdout);
	perror(fn);
	    
	return S3_ERROR;
    }

    if (wt_var == NULL) {
	E_INFO("No variance counts\n");
    }

    sf = *(int32 *)cmd_ln_access("-sigfig");

    sprintf(mv_fmt, "%%.%de,%%.%de ", sf-1, sf-1);

    for (i = 0; i < n_mgau; i++) {
	printf("mgau %u\n", i);
	for (j = 0; j < n_feat; j++) {
	    printf("feature %u\n", j);
	    for (k = 0; k < n_density; k++) {
		printf("density %u: ", k);
		
		printf(e_fmt, dnom[i][j][k]);
		for (l = 0; l < veclen[j]; l++) {
		    printf(mv_fmt,
			   wt_mean[i][j][k][l],
			   (wt_var ? wt_var[i][j][k][l] : 0.0));
		}
		printf("\n");
	    }
	}
    }

    if (wt_mean) {
	ckd_free(wt_mean[0][0][0]);
	ckd_free_3d((void ***)wt_mean);
    }

    if (wt_var) {
	ckd_free(wt_var[0][0][0]);
	ckd_free_3d((void ***)wt_mean);
    }
    return S3_SUCCESS;
}

int
print_tmat(const char *fn)
{
    float32 ***tmat;
    uint32 n_tmat;
    uint32 n_state_pm;
    uint32 i, j, k;
    int32 normalize = *(int32 *)cmd_ln_access("-norm");
    float32 sum;
    
    E_INFO("Reading %s%s\n",
	   (normalize ? " and normalizing." : "."),
	   fn);
    
    if (s3tmat_read(fn,
		    &tmat,
		    &n_tmat,
		    &n_state_pm) != S3_SUCCESS) {
	return S3_ERROR;
    }

    printf("tmat %u %u\n", n_tmat, n_state_pm);

    for (i = 0; i < n_tmat; i++) {
	printf("tmat [%u]\n", i);

	for (j = 0; j < n_state_pm-1; j++) {

	    if (normalize) {
		for (k = 0, sum = 0; k < n_state_pm; k++) {
		    sum += tmat[i][j][k];
		}

		if (sum == 0) {
		    E_WARN("tmat [%u][%u][*] zero\n", i, j);
		    sum = 1;
		}

	    }
	    else
		sum = 1;

	    for (k = 0; k < n_state_pm; k++) {
		if (tmat[i][j][k] > 0.0) {
		    printf(e_fmt_ld_sp, tmat[i][j][k]/sum);
		}
		else {
		    printf(" %8s", "");
		}
	    }
	    printf("\n");
	}
    }
    return S3_SUCCESS;
}
int
print_mixw(const char *fn)
{
    float32 ***mixw;
    uint32 n_mixw;
    uint32 mixw_s = NO_ID;
    uint32 mixw_e = NO_ID;
    uint32 n_feat;
    uint32 n_density;
    uint32 i, p_i, j, k;
    int32 normalize = *(int32 *)cmd_ln_access("-norm");
    float64 sum;

    E_INFO("Reading %s%s\n",
	   fn, (normalize ? "and normalizing." : "."));

    if (cmd_ln_access("-mixws")) {
	mixw_s = *(uint32 *)cmd_ln_access("-mixws");
    }
    if (cmd_ln_access("-mixwe")) {
	mixw_e = *(uint32 *)cmd_ln_access("-mixwe");
    }
    if (mixw_s == NO_ID && mixw_e == NO_ID) {
	if (s3mixw_read(fn,
			&mixw,
			&n_mixw,
			&n_feat,
			&n_density) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	printf("mixw %u %u %u\n", n_mixw, n_feat, n_density);
    }
    else {
	if (s3mixw_intv_read(fn,
			     mixw_s,
			     mixw_e,
			     &mixw,
			     &n_mixw,
			     &n_feat,
			     &n_density) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	printf("mixw %u %u %u subinterval [%u %u]\n",
	       n_mixw, n_feat, n_density, mixw_s, mixw_e);
    }


    for (i = 0; i < n_mixw; i++) {
	if (mixw_s != NO_ID)
	    p_i = i + mixw_s;
	else
	    p_i = i;

	for (j = 0; j < n_feat; j++) {
	    for (k = 0, sum = 0; k < n_density; k++) {
		sum += mixw[i][j][k];
	    }
		
	    printf("mixw [%u %u] %e\n", p_i, j, sum);

	    if (normalize) {
		if (sum == 0) {
		    E_WARN("mixw [%u][%u][*] zero\n", p_i, j);
		    sum = 1;
		}
	    }
	    else
		sum = 1;

	    for (k = 0; k < n_density; k++) {
		if ((k % 8) == 0)
		    printf("\n\t");
		printf(e_fmt, mixw[i][j][k]/sum);
	    }
	    printf("\n");
	}
    }
    return S3_SUCCESS;
}

static int
print()
{
    model_def_t *mdef;
    int ret_val;
    const char *fn;

    ret_val = S3_SUCCESS;

    fn = (const char *)cmd_ln_access("-moddeffn");

    if (fn && model_def_read(&mdef, fn) != S3_SUCCESS) {
      E_ERROR("Problems reading model definition file %s\n", fn);
    }

    fn = (const char *)cmd_ln_access("-mixwfn");
    if (fn && (print_mixw(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }
    
    fn = (const char *)cmd_ln_access("-tmatfn");
    if (fn && (print_tmat(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = (const char *)cmd_ln_access("-gaufn");
    if (fn && (print_gau(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = (const char *)cmd_ln_access("-gaucntfn");
    if (fn && (print_gau_cnt(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = (const char *)cmd_ln_access("-regmatcntfn");
    if (fn && (print_regmat_cnt(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = (const char *)cmd_ln_access("-lambdafn");
    if (fn && (print_lambda(fn,
			    *(float32 *)cmd_ln_access("-lambdamin"),
			    *(float32 *)cmd_ln_access("-lambdamax")) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }
    
    if(ret_val != S3_SUCCESS){
      E_FATAL("Please specify input by either -tmatfn, -mixwfn, -gaufn, -gaucntfn, -regmatcntfn or -lambdafn\n");
    }
    return ret_val;
}

int
main(int argc, char *argv[])
{
    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("Errors initializing.\n");
	return 1;
    }

    if (print() != S3_SUCCESS)
	return 1;

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.6  2004/09/27  22:48:01  egouvea
 * Fixed the failure condition check
 * 
 * Revision 1.5  2004/08/07 21:24:24  arthchan2003
 * add help and example to printp
 *
 * Revision 1.4  2004/07/21 19:17:26  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:32  awb
 * *** empty log message ***
 *
 * Revision 1.12  97/07/16  11:17:32  eht
 * Allow printing of a subrange of all mixing weight arrays.
 * Probably should do this for others, but haven't needed them yet.
 * 
 * Revision 1.11  97/03/07  08:37:17  eht
 * - define prototype for parse_cmd_ln()
 * - use new I/O routines
 * - allow users to select # of significant figures for floats
 * - fixed up compiler warnings
 * - interpolation parameters printing added
 * - regression matrix count printing added
 * 
 * Revision 1.10  1996/03/25  15:23:35  eht
 * For transition matrices, print spaces for 0.0 prob arcs rather
 * than all zeros.  Should be easier to read.
 *
 * Revision 1.9  1996/01/30  17:06:44  eht
 * coalesce mean and variance code into Gaussian code since there
 * was no difference in form for mean and variance code or data
 * files.
 *
 * Revision 1.8  1996/01/26  17:39:17  eht
 * Employ E_INFO() macros instead of fflush(); fprintf(); hand constructions.
 *
 * Revision 1.7  1995/11/10  19:18:43  eht
 * add print_var() function
 *
 * Revision 1.6  1995/10/10  13:09:40  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.5  1995/10/09  15:30:33  eht
 * Removed __FILE__, __LINE__ arguments to ckd_alloc routines
 *
 * Revision 1.4  1995/09/08  19:11:14  eht
 * Updated to use the new acmod_set module.  Prior to test
 * using TI digits corpus
 *
 * Revision 1.3  1995/09/07  19:34:48  eht
 * Don't require -moddeffn argument when it is not necessary
 *
 * Revision 1.2  95/08/09  20:37:06  20:37:06  eht (Eric Thayer)
 * *** empty log message ***
 * 
 * Revision 1.1  1995/06/02  20:36:50  eht
 * Initial revision
 *
 *
 */
