/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
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
 * File: ts_cnt.c
 * 
 * Description: 
 * 	Initialize one Gaussian mixtures for each context independent
 *	phone state.
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include "ts_cnt.h"
#include "mk_sseq.h"

#include <s3/prim_type.h>
#include <s3/cvt2triphone.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <stdio.h>

static int32 did_warn = 0;


int
ts_cnt(uint32 *cnt,		/* observation counts */

       model_def_t *mdef,	/* model definitions */

       uint16 *seg,		/* Viterbi (CI phone,state) pairs for all frames */
       uint32 n_frame,

       acmod_id_t *phone,	/* CI phone sequence (already validated) */
       char *btw_mark,
       uint32 n_phone)
{
    acmod_set_t *acmod_set;
    uint32 *ci_sseq = NULL;
    uint32 *sseq = NULL;
    uint32 i;

    acmod_set = mdef->acmod_set;

    /* make a tied state id sequence from the state segmentation and the
       phone list */
    ci_sseq = mk_sseq(seg, n_frame, phone, n_phone, mdef);
    for (i = 0; i < n_frame; i++) {
	cnt[ci_sseq[i]]++;
    }

    if (cvt2triphone(acmod_set, phone, btw_mark, n_phone) != S3_SUCCESS) {
	if (!did_warn) {
	    E_WARN("Conversion from CI phones to triphones failed\n");
	}
	
	return S3_SUCCESS;
    }

    /* make a tied state id sequence from the state segmentation and the
       phone list */
    sseq = mk_sseq(seg, n_frame, phone, n_phone, mdef);

    for (i = 0; i < n_frame; i++) {
	if (ci_sseq[i] != sseq[i])
	    cnt[sseq[i]]++;
    }
    
    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:14  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:32  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/07  08:42:28  eht
 * Initial revision
 * 
 * Revision 1.1  1996/03/25  15:21:20  eht
 * Initial revision
 *
 *
 */
