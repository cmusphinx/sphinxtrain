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

    if (s3read(out_n_rng,
	       sizeof(uint32),
	       1,
	       fp,
	       swap,
	       &rd_chksum) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    if (s3read_1d(out_map,
		  map_elem_size,
		  out_n_dom,
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

    if (s3write(&n_rng, sizeof(uint32), 1, fp, &chksum) != 1) {
	s3close(fp);

	return S3_ERROR;
    }

    if (s3write_1d(map,
		   map_elem_size,
		   n_dom,
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

    E_INFO("Wrote %s [%u mappings to %u]\n", fn, n_dom, n_rng);

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
