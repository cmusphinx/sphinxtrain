/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * File: s3map_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/s3map_io.h>
#include <s3/s3io.h>

#include <s3/s3.h>

#include <stdio.h>
#include <string.h>

int
s3map_read(const char *fn,
	   void **out_map,
	   uint32 *out_n_dom,
	   uint32 *out_n_rng,
	   size_t map_elem_size)
{
    uint32 rd_chksum = 0;
    uint32 sv_chksum;
    uint32 ignore;
    char *ver;
    char *do_chk;
    FILE *fp;
    uint32 swap;

    fp = s3open(fn, "rb", &swap);
    if (fp == NULL) {
	return S3_ERROR;
    }

    /* check version id */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, MAP_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    fn, ver, MAP_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", fn);
    }

    do_chk = s3get_gvn_fattr("chksum0");

    if (bio_fread(out_n_rng,
	       sizeof(uint32),
	       1,
	       fp,
	       swap,
	       &rd_chksum) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    if (bio_fread_1d(out_map,
		  map_elem_size,
		  out_n_dom,
		  fp,
		  swap,
		  &rd_chksum) < 0) {
	s3close(fp);

	return S3_ERROR;
    }

    if (do_chk) {
	if (bio_fread(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) != 1) {
	    s3close(fp);
	    
	    return S3_ERROR;
	}
	
	if (sv_chksum != rd_chksum) {
	    E_FATAL("Checksum error; read corrupted data.\n");
	}
    }

    E_INFO("Read %s [%u mappings to %u]\n", fn, *out_n_dom, *out_n_rng);

    return S3_SUCCESS;
}

int
s3map_write(const char *fn,
	    void *map,
	    uint32 n_dom,
	    uint32 n_rng,
	    size_t map_elem_size)
{
    FILE *fp;
    uint32 chksum = 0;
    uint32 ignore = 0;

    s3clr_fattr();
    s3add_fattr("version", MAP_FILE_VERSION, TRUE);
    s3add_fattr("chksum0", "yes", TRUE);

    fp = s3open(fn, "wb", NULL);
    if (fp == NULL)
	return S3_ERROR;

    if (bio_fwrite(&n_rng, sizeof(uint32), 1, fp, 0, &chksum) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    if (bio_fwrite_1d(map,
		   map_elem_size,
		   n_dom,
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

    E_INFO("Wrote %s [%u mappings to %u]\n", fn, n_dom, n_rng);

    return S3_SUCCESS;
}
