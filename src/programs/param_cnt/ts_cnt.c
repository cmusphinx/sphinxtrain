/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
 * Revision 1.1  2000/09/24  21:38:32  awb
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
