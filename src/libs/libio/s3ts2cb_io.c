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
 * File: s3ts2cb_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/s3ts2cb_io.h>
#include <s3/s3map_io.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>

int
s3ts2cb_read(const char *fn,
	     uint32 **out_ts2cb,
	     uint32 *out_n_ts,
	     uint32 *out_n_cb)
{
    int ret, i, n_d, n_r;
    int *did_map, incomplete;
    uint32 *ts2cb;
    
    ret = s3map_read(fn,
		     (void **)out_ts2cb,
		     out_n_ts,
		     out_n_cb,
		     sizeof(uint32));

    if (ret == S3_SUCCESS) {
	n_d = *out_n_ts;
	n_r = *out_n_cb;
	ts2cb = *out_ts2cb;

	did_map = ckd_calloc(n_r, sizeof(int));

	for (i = 0; i < n_d; i++) {
	    did_map[ts2cb[i]] = TRUE;
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
s3ts2cb_write(const char *fn,
	      uint32 *ts2cb,
	      uint32 n_ts,
	      uint32 n_cb)
{
    return s3map_write(fn,
		      ts2cb,
		      n_ts,
		      n_cb,
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
