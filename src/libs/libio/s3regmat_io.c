/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: s3regmat_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/s3regmat_io.h>
#include <s3/s3io.h>

#include <s3/err.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <stdio.h>
#include <string.h>
int
s3regmatcnt_read(const char *fn,
		 float32 *****out_regr,
		 float32 ******out_regl,
		 uint32 *out_n_class,
		 uint32 *out_n_feat,
		 const uint32 **out_veclen,
		 uint32 *out_mllr_mult,
		 uint32 *out_mllr_add)
{
    uint32 rd_chksum = 0;
    uint32 sv_chksum;
    uint32 ignore;
    char *ver;
    char *do_chk;
    FILE *fp;
    uint32 n_feat, n_class;
    const uint32 *veclen;
    float32 ****l_regr;
    float32 *****l_regl;
    uint32 d1, d2, d3, m, f;
    uint32 swap;
    
    fp = s3open(fn, "rb", &swap);
    if (fp == NULL)
	return S3_ERROR;

    /* check version id */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, REGMATCNT_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, REGMATCNT_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    if (s3read_1d((void **)&veclen, sizeof(uint32), &n_feat, fp, swap, &rd_chksum) != S3_SUCCESS) {
	return S3_ERROR;
    }

    if (s3read((void *)&n_class, sizeof(uint32), 1, fp, swap, &rd_chksum) != 1) {
	return S3_ERROR;
    }

    if (s3read((void *)out_mllr_mult, sizeof(uint32), 1, fp, swap, &rd_chksum) != 1) {
	return S3_ERROR;
    }

    if (s3read((void *)out_mllr_add, sizeof(uint32), 1, fp, swap, &rd_chksum) != 1) {
	return S3_ERROR;
    }

    l_regr = (float32 ****)ckd_calloc_2d(n_class, n_feat, sizeof(float32 ***));
    l_regl = (float32 *****)ckd_calloc_2d(n_class, n_feat, sizeof(float32 ****));

    for (m = 0; m < n_class; m++) {
	for (f = 0; f < n_feat; f++) {
	    if (s3read_3d((void ****)&l_regl[m][f],
			  sizeof(float32), &d1, &d2, &d3, fp, swap, &rd_chksum) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	    if (d1 != veclen[f]) {
		E_FATAL("left-hand side regression matrices inconsistent w/ feature set.  Should be %ux%ux%u, but %ux%ux%u\n", veclen[f], veclen[f]+1, veclen[f]+1, d1, d2, d3);
	    }
	    if ((d2 != d1 + 1) ||
		(d3 != d1 + 1)) {
		E_FATAL("left-hand side regression matrices should be %ux%ux%u, but %ux%ux%u\n", d1, d1+1, d1+1, d1, d2, d3);
	    }

	    if (s3read_2d((void ***)&l_regr[m][f], sizeof(float32), &d1, &d2, fp, swap, &rd_chksum) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	    if (d1 != veclen[f]) {
		E_FATAL("right-hand side regression matrices inconsistent w/ feature set.  Should be %ux%u, but %ux%u\n", veclen[f], veclen[f]+1, d1, d2);
	    }
	    if (d2 != d1 + 1) {
		E_FATAL("right-hand side regression matrices should be %ux%u, but %ux%u\n", d1, d1+1, d1, d2);
	    }
	}
    }

    if (do_chk) {
	/* See if the checksum in the file matches that which
	   was computed from the read data */

	if (s3read(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) != 1) {
	    s3close(fp);
	    return S3_ERROR;
	}
	
	if (sv_chksum != rd_chksum) {
	    E_FATAL("Checksum error; read corrupt data.\n");
	}
    }
    
    s3close(fp);

    *out_regl = l_regl;
    *out_regr = l_regr;
    *out_n_class = n_class;
    *out_n_feat = n_feat;
    *out_veclen = veclen;

    /* other two output args done above */

    E_INFO("Read %s %u*%u regl and regr arrays\n",
	   fn, n_class, n_feat);
    

    return S3_SUCCESS;
}

int
s3regmatcnt_write(const char *fn,
		  float32 ****regr,
		  float32 *****regl,
		  uint32 n_class,
		  uint32 n_feat,
		  const uint32 *veclen,
		  uint32 mllr_mult,
		  uint32 mllr_add)
{
    FILE *fp;
    uint32 chksum = 0;
    uint32 ignore = 0;
    uint32 m, f;

    s3clr_fattr();
    s3add_fattr("version", REGMATCNT_FILE_VERSION, TRUE);
    s3add_fattr("chksum0", "yes", TRUE);

    fp = s3open(fn, "wb", NULL);
    if (fp == NULL)
	return S3_ERROR;


    if (s3write_1d((void *)veclen, sizeof(uint32), n_feat, fp, &chksum) != S3_SUCCESS) {
	return S3_ERROR;
    }

    if (s3write((void *)&n_class, sizeof(uint32), 1, fp, &chksum) != 1) {
	return S3_ERROR;
    }

    if (s3write((void *)&mllr_mult, sizeof(uint32), 1, fp, &chksum) != 1) {
	return S3_ERROR;
    }

    if (s3write((void *)&mllr_add, sizeof(uint32), 1, fp, &chksum) != 1) {
	return S3_ERROR;
    }

    for (m = 0; m < n_class; m++) {
	for (f = 0; f < n_feat; f++) {
	    if (s3write_3d((void ***)regl[m][f], sizeof(float32),
			   veclen[f], veclen[f]+1, veclen[f]+1, fp, &chksum) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	    if (s3write_2d((void **)regr[m][f], sizeof(float32),
			   veclen[f], veclen[f]+1, fp, &chksum) != S3_SUCCESS) {
		return S3_ERROR;
	    }
	}
    }

    if (s3write(&chksum, sizeof(uint32), 1, fp, &ignore) != 1) {
	s3close(fp);

	return S3_ERROR;
    }
	
    s3close(fp);

    E_INFO("Wrote %s %u*%u regl and regl arrays.\n",
	   fn, n_class, n_feat);

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
