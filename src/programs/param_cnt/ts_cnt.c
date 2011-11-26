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

#include <sphinxbase/prim_type.h>
#include <sphinxbase/ckd_alloc.h>

#include <s3/cvt2triphone.h>
#include <s3/mk_sseq.h>
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
