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
 * File: s3cb2mllr_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/s3cb2mllr_io.h>

#include <s3/s3map_io.h>

#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>

int
s3cb2mllr_read(const char *fn,
	       uint32 **out_cb2mllr,
	       uint32 *out_n_cb,
	       uint32 *out_n_mllr)
{
    int ret, i, n_d, n_r;
    int *did_map, incomplete;
    uint32 *cb2mllr;

    ret = s3map_read(fn,
		     (void **)out_cb2mllr,
		     out_n_cb,
		     out_n_mllr,
		     sizeof(uint32));

    
    if (ret == S3_SUCCESS) {
	n_d = *out_n_cb;
	n_r = *out_n_mllr;
	cb2mllr = *out_cb2mllr;
	
	did_map = ckd_calloc(n_r, sizeof(int));

	for (i = 0; i < n_d; i++) {
	    did_map[cb2mllr[i]] = TRUE;
	}
	
	for (i = 0, incomplete = FALSE; i < n_r; i++) {
	    if (!did_map[i])
		incomplete = TRUE;
	}
	if (incomplete) {
	    E_WARN("%s maps into [0..%u] rather than onto it\n",
		   fn, n_r-1);
	}

	ckd_free(did_map);
    }

    return ret;
}

int
s3cb2mllr_write(const char *fn,
		uint32 *cb2mllr,
		uint32 n_cb,
		uint32 n_mllr)
{
    return s3map_write(fn,
		       (void *)cb2mllr,
		       n_cb,
		       n_mllr,
		       sizeof(uint32));
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
