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
 * File: phone_cnt.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "phone_cnt.h"

#include <s3/cvt2triphone.h>

static int32 did_warn = 0;

int
phone_cnt(uint32 *cnt,

	  model_def_t *mdef,	/* model definitions */

	  uint16 *seg,		/* Viterbi (CI phone,state) pairs for all frames */
	  uint32 n_frame,

	  acmod_id_t *phone,	/* CI phone sequence (already validated) */
	  char *btw_mark,
	  uint32 n_phone)
{
    acmod_set_t *acmod_set;
    uint32 i;

    acmod_set = mdef->acmod_set;

    if (cvt2triphone(acmod_set, phone, btw_mark, n_phone) != S3_SUCCESS) {
	if (!did_warn) {
	    E_WARN("Conversion from CI phones to triphones failed\n");
	}
	
	return S3_SUCCESS;
    }

    for (i = 0; i < n_phone; i++) {
	cnt[phone[i]]++;
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
 * Revision 1.1  97/03/07  08:42:17  eht
 * Initial revision
 * 
 *
 */
