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
 * File: mk_seg.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "mk_seg.h"

#include <s3/s2_param.h>
#include <s3/s3.h>

#include <assert.h>

int
mk_seg(acmod_set_t *acmod_set,
       uint16 *seg,
       uint32 n_frame,
       acmod_id_t *phone,
       uint32 *start,
       uint32 *len,
       uint32 n_phone)
{
    uint32 phone_i;
    uint32 f;

    /*
     * BEWARE: This routine does not check the consistency of
     *		phone[] with seg[].  See ck_seg() for this test.
     */
       

    for (phone_i = 0, f = 0; phone_i < n_phone; phone_i++) {
	assert(seg[f] & 0x8000);	/* beginning of phone bit */
	
	start[phone_i] = f;
	
	/* skip over the remaining frames in the segment */
	for (++f; (f < n_frame) && !(seg[f] & 0x8000); f++);
	
	len[phone_i] = f - start[phone_i];
    }
	
    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.2  96/03/25  15:45:23  eht
 * Development version
 * 
 * Revision 1.1  1996/03/04  16:01:06  eht
 * Initial revision
 *
 *
 */
