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

#include <sphinxbase/err.h>
#include <sphinxbase/bio.h>

#include <s3/common.h>
#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3lamb_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/s3io.h>
#include <sys_compat/file.h>
#include <s3/s3.h>
#include <s3/mllr_io.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

static int print(void);

static char e_fmt[20];
static char e_fmt_ld_sp[20];
 

static int
initialize(int argc,
	   char *argv[])
{
    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    sprintf(e_fmt, "%%.%de ", cmd_ln_int32("-sigfig") - 1);
    sprintf(e_fmt_ld_sp, " %%.%de", cmd_ln_int32("-sigfig") - 1);
    
    return S3_SUCCESS;
}

int
print_ts2cb(const char *fn)
{
    uint32 *map;
    uint32 n_ts, n_cb;
    uint32 i;

    E_INFO("Reading %s\n",  fn);

    if (s3ts2cb_read(fn, &map, &n_ts, &n_cb) != S3_SUCCESS)
	return S3_ERROR;

    for (i = 0; i < n_ts; i++) {
	printf("%d: %d\n", i, map[i]);
    }

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
print_lda(const char *fn)
{
    FILE *fh;
    float32 ***lda;
    uint32 n_lda, m, n;
    uint32 i, j, k, swap, chksum;

    E_INFO("Reading %s\n",  fn);
    
    if ((fh = s3open(fn, "rb", &swap)) == NULL)
	return S3_ERROR;
    if (bio_fread_3d((void ****)&lda, sizeof(float32),
		  &n_lda, &m, &n, fh, swap, &chksum) < 0)
	return S3_ERROR;

    printf("lda %u %u %u\n", n_lda, m, n);

    for (i = 0; i < n_lda; i++) {
	printf("lda %u\n", i);
	for (j = 0; j < m; j++) {
	    for (k = 0; k < n; k++) {
		printf("%e ", lda[i][j][k]);
	    }
	    printf("\n");
	}
	printf("\n");
    }
    return S3_SUCCESS;
}

int
print_gau(const char *fn)
{
    vector_t ***mean;
    uint32 n_mgau;
    uint32 n_feat;
    uint32 *veclen;
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
    ckd_free(veclen);
    return S3_SUCCESS;
}

int
print_full_gau(const char *fn)
{
    vector_t ****var;
    uint32 n_mgau;
    uint32 n_feat;
    uint32 *veclen;
    uint32 n_density;
    uint32 i, j, k, l, m;

    E_INFO("Reading %s\n",  fn);
    
    if (s3gau_read_full(fn,
			&var,
			&n_mgau,
			&n_feat,
			&n_density,
			&veclen) != S3_SUCCESS)
	return S3_ERROR;

    printf("param %u %u %u (full)\n", n_mgau, n_feat, n_density);

    for (i = 0; i < n_mgau; i++) {
	printf("mgau %u\n", i);
	for (j = 0; j < n_feat; j++) {
	    printf("feat %u\n", j);
	    for (k = 0; k < n_density; k++) {
		printf("density %4u (%ux%u)\n", k, veclen[j], veclen[j]);
		for (l = 0; l < veclen[j]; l++) {
		    for (m = 0; m < veclen[j]; m++) {
			printf(e_fmt, var[i][j][k][l][m]);
		    }
		printf("\n");
		}
	    }
	}
    }
    ckd_free(veclen);
    return S3_SUCCESS;
}

int
print_regmat_cnt(const char *fn)
{
    float32 ****regl;
    float32 ***regr;
    uint32 n_mllr_class;
    uint32 n_feat;
    uint32 *veclen;
    uint32 c, f, j, k;

    if (read_reg_mat (fn, &veclen, &n_mllr_class, &n_feat, &regl, &regr) != S3_SUCCESS) {
	return S3_ERROR;
    }

    for (c = 0; c < n_mllr_class; c++) {
	for (f = 0; f < n_feat; f++) {
	    printf("%d\n", veclen[f]); 
	    for (j = 0; j < veclen[f]; j++) {
		for (k = 0; k < veclen[f]; ++k) {
		    printf("%f ", regl[c][f][j][k]);
		}
		printf("\n");
	    }
	    for (j = 0; j < veclen[f]; j++) {
		printf("%f ", regr[c][f][j]);
	    }
	    printf("\n");
	    /* Identity transform for variances. */
	    for (j = 0; j < veclen[f]; j++) {
		printf("1.0 ");
	    }
	    printf("\n");
	}
    }
    ckd_free(veclen);

    free_mllr_reg(&regl, &regr, n_mllr_class, n_feat);

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
    uint32 *veclen;
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
    if (pass2var) {
	E_INFO("-2passvar yes\n");
    }

    sf = cmd_ln_int32("-sigfig");

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
	ckd_free_3d((void ***)wt_var);
    }
    ckd_free(veclen);
    return S3_SUCCESS;
}

int
print_tmat(const char *fn)
{
    float32 ***tmat;
    uint32 n_tmat;
    uint32 n_state_pm;
    uint32 i, j, k;
    int32 normalize = cmd_ln_int32("-norm");
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
    int32 normalize = cmd_ln_int32("-norm");
    float64 sum;

    E_INFO("Reading %s%s\n",
	   fn, (normalize ? "and normalizing." : "."));

    if (cmd_ln_str("-mixws")) {
	mixw_s = cmd_ln_int32("-mixws");
    }
    if (cmd_ln_str("-mixwe")) {
	mixw_e = cmd_ln_int32("-mixwe");
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

    fn = cmd_ln_str("-moddeffn");

    if (fn && model_def_read(&mdef, fn) != S3_SUCCESS) {
      E_ERROR("Problems reading model definition file %s\n", fn);
    }

    fn = cmd_ln_str("-mixwfn");
    if (fn && (print_mixw(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }
    
    fn = cmd_ln_str("-tmatfn");
    if (fn && (print_tmat(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-gaufn");
    if (fn && (print_gau(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-fullgaufn");
    if (fn && (print_full_gau(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-gaucntfn");
    if (fn && (print_gau_cnt(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-regmatcntfn");
    if (fn && (print_regmat_cnt(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-lambdafn");
    if (fn && (print_lambda(fn,
			    cmd_ln_float32("-lambdamin"),
			    cmd_ln_float32("-lambdamax")) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-lda");
    if (fn && (print_lda(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }

    fn = cmd_ln_str("-ts2cbfn");
    if (fn && (print_ts2cb(fn) != S3_SUCCESS)) {
	ret_val = S3_ERROR;
    }
    
    if(ret_val != S3_SUCCESS){
      E_FATAL("Please specify input by either -tmatfn, -mixwfn, -gaufn, -gaucntfn, -regmatcntfn, -lda, -ts2cbfn or -lambdafn\n");
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
