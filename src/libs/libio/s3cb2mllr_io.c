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
 * File: s3cb2mllr_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <sphinxbase/err.h>
#include <sphinxbase/ckd_alloc.h>

#include <s3/s3cb2mllr_io.h>
#include <s3/s3map_io.h>
#include <s3/s3.h>

#define IS_MAPPED	1
#define NOT_MAPPED	0

int
s3cb2mllr_read(const char *fn,
	       int32 **out_cb2mllr,
	       uint32 *out_n_cb,
	       uint32 *out_n_mllr)
{
    int ret, i, n_d, n_r;
    int *did_map;
    int32 *cb2mllr;
    int in_hole = FALSE;
    int beg_hole = 0;

    ret = s3map_read(fn,
		     (void **)out_cb2mllr,
		     out_n_cb,
		     out_n_mllr,
		     sizeof(int32));

    
    if (ret == S3_SUCCESS) {
	n_d = *out_n_cb;
	n_r = *out_n_mllr;
	cb2mllr = *out_cb2mllr;
	
	did_map = ckd_calloc(n_r, sizeof(int));

	for (i = 0; i < n_d; i++) {
	    if (cb2mllr[i] >= (int32)*out_n_mllr) {
		E_FATAL("%s cb2mllr[%d] -> %d which is >= n_mllr_class (%d)\n",
			fn, i, cb2mllr[i], *out_n_mllr);
	    }
	    else if (cb2mllr[i] >= 0)
		did_map[cb2mllr[i]] = IS_MAPPED;
	}
	
	for (i = 0; i < n_d; i++) {
	    if (cb2mllr[i] < 0) continue; /* skipped */
	    if (!in_hole && (did_map[cb2mllr[i]] == NOT_MAPPED)) {
		beg_hole = i;
		in_hole = TRUE;
	    }
	    if (in_hole && (did_map[cb2mllr[i]] == IS_MAPPED)) {
		E_FATAL("cb unmapped in region [%u %u]\n", beg_hole, i-1);
		in_hole = FALSE;
	    }
	}
	if (in_hole) {
	    E_FATAL("cb unmapped in region [%u %u]\n", beg_hole, i-1);
	}

	ckd_free(did_map);
    }

    return ret;
}

int
s3cb2mllr_write(const char *fn,
		int32 *cb2mllr,
		uint32 n_cb,
		uint32 n_mllr)
{
    return s3map_write(fn,
		       (void *)cb2mllr,
		       n_cb,
		       n_mllr,
		       sizeof(int32));
}
