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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
