/* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights 
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
 * File: s3ts2cb_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <s3/s3ts2cb_io.h>
#include <s3/s3map_io.h>
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
