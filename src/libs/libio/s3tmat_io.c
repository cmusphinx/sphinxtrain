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
 * File: s3tmat_io.c
 * 
 * Description: 
 *     SPHINX-III transition matrix file I/O functions
 *
 * Author: 
 *     Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s3tmat_io.h>
#include <s3/matrix.h>
#include <s3/s3io.h>
#include <s3/s3.h>

#include <string.h>

int
s3tmat_read(const char *fn,
	    float32 ****out_tmat,
	    uint32 *out_n_tmat,
	    uint32 *out_n_state)
{
    uint32 rd_chksum = 0;
    uint32 sv_chksum;
    uint32 ignore;
    uint32 tmp;
    char *ver;
    char *do_chk;
    FILE *fp;
    uint32 swap;

    fp = s3open(fn, "rb", &swap);
    if (fp == NULL)
	return S3_ERROR;

    /* check version id */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, TMAT_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, TMAT_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    if (s3read_3d((void ****)out_tmat,
		  sizeof(float32),
		  out_n_tmat,
		  &tmp,
		  out_n_state,
		  fp,
		  swap,
		  &rd_chksum) != S3_SUCCESS) {
	s3close(fp);

	return S3_ERROR;
    }

    if (do_chk) {
	if (s3read(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) != 1) {
	    s3close(fp);
	    
	    return S3_ERROR;
	}

	if (sv_chksum != rd_chksum) {
	    E_FATAL("Checksum error; read corrupted data.\n");
	}
    }

    s3close(fp);

    E_INFO("Read %s [%ux%ux%u array]\n",
	   fn, *out_n_tmat, (*out_n_state)-1, *out_n_state);

    return S3_SUCCESS;
}

int
s3tmat_write(const char *fn,
	     float32 ***tmat,
	     uint32 n_tmat,
	     uint32 n_state)
{
    FILE *fp;
    uint32 chksum = 0;
    uint32 ignore = 0;
    int t, i, j;
    
    s3clr_fattr();
    s3add_fattr("version", TMAT_FILE_VERSION, TRUE);
    s3add_fattr("chksum0", "yes", TRUE);

    fp = s3open(fn, "wb", NULL);
    if (fp == NULL)
	return S3_ERROR;

    for (t = 0; t < n_tmat; t++) {
	for (i = 0; i < n_state-1; i++) {
	    for (j = 0; j < n_state; j++) {
		if (tmat[t][i][j] < 0) {
		    E_ERROR("tmat[%u][%u][%u] < 0 (%e)\n",
			    t, i, j, tmat[t][i][j]);
		}
	    }
	}
    }

    /* floor all non-zero entries to this value to make sure
       that results are compatible between machines */
    floor_nz_3d(tmat, n_tmat, n_state-1, n_state, MIN_POS_FLOAT32);

    if (s3write_3d((void ***)tmat,
		   sizeof(float32),
		   n_tmat,
		   n_state-1,
		   n_state,
		   fp,
		   &chksum) != S3_SUCCESS) {
	s3close(fp);

	return S3_ERROR;
    }
    if (s3write(&chksum, sizeof(uint32), 1, fp, &ignore) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    s3close(fp);

    E_INFO("Wrote %s [%ux%ux%u array]\n",
	   fn, n_tmat, n_state-1, n_state);

    return S3_SUCCESS;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
