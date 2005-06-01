/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1996-2005 Carnegie Mellon University.
 * All rights reserved.
 *
 * Author:
 * 	Sam-Joo Doh (sjdoh@cs.cmu.edu)
 * 	David Huggins-Daines (dhuggins@cs.cmu.edu)
 *
 *********************************************************************/

/* The SPHINX-III common library */
#include <stdio.h>
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/s3gau_io.h>
#include <sys_compat/file.h>
#include <s3/model_def_io.h>
#include <s3/s3ts2cb_io.h>
#include <s3/ts2cb.h>
#include <s3/s3cb2mllr_io.h>
#include <s3/mllr_io.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s3.h>
#include <s3/err.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define  ABS(x)         ((x)<0. ? -(x):(x))

#include "parse_cmd_ln.h"

static int
initialize(int argc,
	   char *argv[])
{
    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);
    
    return S3_SUCCESS;
}

static int
map_adapt(const char *mapmeanfn,
	  const char *mapvarfn,
	  const char *mlmeanfn,
	  const char *mlvarfn,
	  const char *mlcntfn,
	  const char *simeanfn,
	  const char *sivarfn)
{
    vector_t 	  ***mapmean;	/* output */
    /* vector_t 	  ***mapvar;	*/ /*output - currently unused */
    vector_t 	  ***simean;
    vector_t 	  ***sivar;
    vector_t 	  ***mlmean;
    vector_t 	  ***mlvar;
    float32  	  ***dnom;	/* use this instead of mlcnt */
    float32  	  dnom_weight;

    uint32 	  n_cb,      n_cb_rd;
    uint32 	  n_feat,    n_feat_rd;
    uint32 	  n_density, n_density_rd;
    const uint32  *veclen,   *veclen_rd;
    uint32 	  i, j, k, m;

    /*************************************/
    if (s3gau_read(simeanfn,
		   &simean,
		   &n_cb,
		   &n_feat,
		   &n_density,
		   &veclen) != S3_SUCCESS){
	E_FATAL("Unable to read %s\n",simeanfn);
    }
    E_INFO("Reading %s : n_cb = %d, n_feat = %d, n_density = %d\n",
	   simeanfn, n_cb, n_feat, n_density);


    /*************************************/
    if (s3gau_read(sivarfn,
		   &sivar,
		   &n_cb_rd,
		   &n_feat_rd,
		   &n_density_rd,
		   &veclen_rd) != S3_SUCCESS){
	E_FATAL("Unable to read %s\n",sivarfn);
    }
    E_INFO("Reading %s\n", sivarfn);

    if (n_cb_rd != n_cb) {
        E_FATAL("# of cb are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_cb, sivarfn, n_cb_rd);
    }
    if (n_feat_rd != n_feat) {
        E_FATAL("# of feat are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_feat, sivarfn, n_feat_rd);
    }
    if (n_density_rd != n_density) {
        E_FATAL("# of density are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_density, sivarfn, n_density_rd);
    }
    for (i=0;i<n_feat;++i) {
        if (veclen[i] != veclen_rd[i]) {
            E_FATAL("veclen (%u element) are inconsistent.   %s : %u,   %s : %u\n",
                    i, simeanfn, veclen[i], sivarfn, veclen_rd[i]);
        }
    }
    ckd_free((void *)veclen_rd);

    /*************************************/
    if (s3gau_read(mlmeanfn,
		   &mlmean,
		   &n_cb_rd,
		   &n_feat_rd,
		   &n_density_rd,
		   &veclen_rd) != S3_SUCCESS){
	E_FATAL("Unable to read %s\n",mlmeanfn);
    }
    E_INFO("Reading %s\n", mlmeanfn);

    if (n_cb_rd != n_cb) {
        E_FATAL("# of cb are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_cb, mlmeanfn, n_cb_rd);
    }
    if (n_feat_rd != n_feat) {
        E_FATAL("# of feat are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_feat, mlmeanfn, n_feat_rd);
    }
    if (n_density_rd != n_density) {
        E_FATAL("# of density are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_density, mlmeanfn, n_density_rd);
    }
    for (i=0;i<n_feat;++i) {
        if (veclen[i] != veclen_rd[i]) {
            E_FATAL("veclen (%u element) are inconsistent.   %s : %u,   %s : %u\n",
                    i, simeanfn, veclen[i], mlmeanfn, veclen_rd[i]);
        }
    }
    ckd_free((void *)veclen_rd);

    if (s3gau_read(mlvarfn,
		   &mlvar,
		   &n_cb_rd,
		   &n_feat_rd,
		   &n_density_rd,
		   &veclen_rd) != S3_SUCCESS){
	E_FATAL("Unable to read %s\n",mlvarfn);
    }
    E_INFO("Reading %s\n", mlvarfn);

    if (n_cb_rd != n_cb) {
        E_FATAL("# of cb are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_cb, mlvarfn, n_cb_rd);
    }
    if (n_feat_rd != n_feat) {
        E_FATAL("# of feat are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_feat, mlvarfn, n_feat_rd);
    }
    if (n_density_rd != n_density) {
        E_FATAL("# of density are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_density, mlvarfn, n_density_rd);
    }
    for (i=0;i<n_feat;++i) {
        if (veclen[i] != veclen_rd[i]) {
            E_FATAL("veclen (%u element) are inconsistent.   %s : %u,   %s : %u\n",
                    i, simeanfn, veclen[i], mlvarfn, veclen_rd[i]);
        }
    }
    ckd_free((void *)veclen_rd);


    /*************************************/
    if (s3gaudnom_read(mlcntfn,
                       &dnom,
                       &n_cb_rd,
                       &n_feat_rd,
                       &n_density_rd) != S3_SUCCESS) {
        exit(1);
    }
    E_INFO("Reading %s\n", mlcntfn);

    if (n_cb_rd != n_cb) {
        E_FATAL("# of cb are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_cb, mlcntfn, n_cb_rd);
    }
    if (n_feat_rd != n_feat) {
        E_FATAL("# of feat are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_feat, mlcntfn, n_feat_rd);
    }
    if (n_density_rd != n_density) {
        E_FATAL("# of density are inconsistent.   %s : %u,   %s : %u\n",
                simeanfn, n_density, mlcntfn, n_density_rd);
    }

    /**********  Now calculate new MAP mean  **********************/
    mapmean  = gauden_alloc_param(n_cb, n_feat, n_density, veclen);

    dnom_weight = *(float32 *)cmd_ln_access("-dnom_weight");

    for (i = 0; i < n_cb; i++) {
      for (j = 0; j < n_feat; j++) {

        for (k = 0; k < n_density; k++) {
	  for (m = 0; m < veclen[j]; m++) {

	    if ((ABS(sivar[i][j][k][m]) < 1.0e-10)
		|| (ABS(mlvar[i][j][k][m]) < 1.0e-4)
		|| (ABS(dnom[i][j][k]) < 1.0e-4))
	      mapmean[i][j][k][m] = simean[i][j][k][m];
	    else
	      mapmean[i][j][k][m] =
	        (float)( dnom_weight * dnom[i][j][k] * sivar[i][j][k][m] * mlmean[i][j][k][m]
	               + mlvar[i][j][k][m] * simean[i][j][k][m])
	      / (float)(dnom_weight * dnom[i][j][k] * sivar[i][j][k][m] + mlvar[i][j][k][m]);
	  }
	}
      }
    }


    if (s3gau_write(mapmeanfn,
		    (const vector_t ***)mapmean,
		    n_cb,
		    n_feat,
		    n_density,
		    veclen) != S3_SUCCESS)
	E_FATAL("Unable to write prior mean to %s\n",mapmeanfn);

    gauden_free_param(mapmean);
    gauden_free_param(simean);
    gauden_free_param(sivar);
    gauden_free_param(mlmean);
    gauden_free_param(mlvar);
    ckd_free_3d((void *)dnom);
    ckd_free((void *)veclen);
    
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    const char *mapmeanfn;
    const char *mlcntfn;
    const char *mlmeanfn;
    const char *mlvarfn;
    const char *simeanfn;
    const char *sivarfn;

    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("Errors initializing.\n");
	exit(1);
    }

    mlmeanfn = (const char *)cmd_ln_access("-mlmeanfn");
    if (mlmeanfn == NULL)
	E_FATAL("You must specify -mlmeanfn\n");
    mlvarfn = (const char *)cmd_ln_access("-mlvarfn");
    if (mlvarfn == NULL)
	E_FATAL("You must specify -mlvarfn\n");
    mlcntfn = (const char *)cmd_ln_access("-mlcntfn");
    if (mlcntfn == NULL)
	E_FATAL("You must specify -mlcntfn\n");
    simeanfn = (const char *)cmd_ln_access("-simeanfn");
    if (simeanfn == NULL)
	E_FATAL("You must specify -simeanfn\n");
    sivarfn = (const char *)cmd_ln_access("-sivarfn");
    if (sivarfn == NULL)
	E_FATAL("You must specify -sivarfn\n");
    mapmeanfn = (const char *)cmd_ln_access("-mapmeanfn");
    if (mapmeanfn == NULL)
	E_FATAL("You must specify -mapmeanfn\n");

    if (map_adapt(mapmeanfn, NULL, mlmeanfn, mlvarfn, mlcntfn, simeanfn, sivarfn)
	!= S3_SUCCESS)
	E_FATAL("MAP adaptation failed\n");

    exit(0);
}
