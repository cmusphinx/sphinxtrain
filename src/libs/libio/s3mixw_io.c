/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: s3mixw_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/s3mixw_io.h>
#include <sphinxbase/matrix.h>
#include <s3/s3io.h>
#include <s3/s3.h>

#include <string.h>

int
s3mixw_read(const char *fn,
	    float32 ****out_mixw,
	    uint32 *out_n_mixw,
	    uint32 *out_n_feat,
	    uint32 *out_n_density)
{
    uint32 rd_chksum = 0;
    uint32 sv_chksum;
    uint32 swap;
    uint32 ignore;
    char *ver;
    char *do_chk;
    FILE *fp;

    fp = s3open(fn, "rb", &swap);
    if (fp == NULL)
	return S3_ERROR;

    /* check version id */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, MIXW_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, MIXW_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    /* Read the mixing weight array */
    if (bio_fread_3d((void ****)out_mixw,
		  sizeof(float32),
		  out_n_mixw,
		  out_n_feat,
		  out_n_density,
		  fp,
		  swap,
		  &rd_chksum) < 0) {
	s3close(fp);
	return S3_ERROR;
    }

    if (do_chk) {
	/* See if the checksum in the file matches that which
	   was computed from the read data */

	if (bio_fread(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) != 1) {
	    s3close(fp);
	    return S3_ERROR;
	}

	if (sv_chksum != rd_chksum) {
	    E_FATAL("Checksum error; read corrupt data.\n");
	}
    }

    s3close(fp);

    E_INFO("Read %s [%ux%ux%u array]\n",
	   fn, *out_n_mixw, *out_n_feat, *out_n_density);

    return S3_SUCCESS;
}

int
s3mixw_intv_read(const char *fn,
		 uint32  mixw_s,
		 uint32  mixw_e,
		 float32 ****out_mixw,
		 uint32 *out_n_mixw,
		 uint32 *out_n_feat,
		 uint32 *out_n_density)
{
    uint32 ignore;
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
	if (strcmp(ver, MIXW_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, MIXW_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    /* Read the mixing weight array */
    if (bio_fread_intv_3d((void ****)out_mixw,
		       sizeof(float32),
		       mixw_s,
		       mixw_e,
		       out_n_mixw,
		       out_n_feat,
		       out_n_density,
		       fp,
		       swap,
		       &ignore) != S3_SUCCESS) {
	s3close(fp);
	return S3_ERROR;
    }
    s3close(fp);

    E_INFO("Read %s [%ux%ux%u array]\n",
	   fn, *out_n_mixw, *out_n_feat, *out_n_density);

    return S3_SUCCESS;
}

int
s3mixw_write(const char *fn,
	     float32 ***mixw,
	     uint32 n_mixw,
	     uint32 n_feat,
	     uint32 n_density)
{
    FILE *fp;
    uint32 chksum = 0;
    uint32 ignore = 0;
    uint32 i, j, k;

    s3clr_fattr();
    s3add_fattr("version", MIXW_FILE_VERSION, TRUE);
    s3add_fattr("chksum0", "yes", TRUE);

    fp = s3open(fn, "wb", NULL);
    if (fp == NULL)
	return S3_ERROR;

    for (i = 0; i < n_mixw; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		if (mixw[i][j][k] < 0) {
		    E_ERROR("mixw[%u][%u][%u] < 0 (%e)\n",
			    i,j,k,mixw[i][j][k]);
		}
	    }
	}
    }

    /* floor all non-zero entries to this value to make sure
       that results are compatible between machines */
    floor_nz_3d(mixw, n_mixw, n_feat, n_density, MIN_POS_FLOAT32);

    if (bio_fwrite_3d((void ***)mixw,
		   sizeof(float32),
		   n_mixw,
		   n_feat,
		   n_density,
		   fp,
		   &chksum) < 0) {
	s3close(fp);
	return S3_ERROR;
    }
	
    if (bio_fwrite(&chksum, sizeof(uint32), 1, fp, 0, &ignore) != 1) {
	s3close(fp);

	return S3_ERROR;
    }
	
    s3close(fp);

    E_INFO("Wrote %s [%ux%ux%u array]\n",
	   fn, n_mixw, n_feat, n_density);

    return S3_SUCCESS;
}
