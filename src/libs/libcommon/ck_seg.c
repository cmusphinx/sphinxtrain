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
 * File: ck_seg.c
 * 
 * Description: 
 * 	Check to see whether a seg file agrees with a given dictionary
 *	and word transcription.
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/ck_seg.h>

#include <s3/s2_param.h>
#include <s3/acmod_set.h>
#include <s3/s3.h>

int
ck_seg(acmod_set_t *acmod_set,
       acmod_id_t *phone,
       uint32 n_phone,
       uint16 *seg,
       uint32 n_frame,
       const char *utt_name)
{
    uint32	phone_i;
    uint32	seg_ci;
    acmod_id_t	trans_ci;
    uint32 f;
    uint32 n_state = S2_N_STATE-1;	/* # of emitting states/model */

    if (n_phone == 0) {
	if (n_frame == 0) 
	    return S3_SUCCESS;
	else {
	    E_ERROR("utt %s: No phones, but non-empty seg exists\n");
	    return S3_ERROR;
	}
    }

    if ((seg[0] & 0x8000) == 0) {
	E_ERROR("utt %s: Expected beginning of phone indicator for at frame %u\n",
		utt_name, 0);

	return S3_ERROR;
    }

    phone_i = 0;
    f = 0;

    seg_ci = (seg[f++] & 0x7FFF) / n_state;
    trans_ci = acmod_set_base_phone(acmod_set, phone[phone_i++]);
    /* skip over non-begin frames */
    for (; ((f < n_frame) && ((seg[f] & 0x8000) == 0)); f++);

    while ((seg_ci == trans_ci) &&
	   (f < n_frame) &&
	   (phone_i < n_phone)) {
	
	seg_ci = (seg[f++] & 0x7FFF) / n_state;
	
	trans_ci = acmod_set_base_phone(acmod_set, phone[phone_i++]);
	
	/* skip over non-begin frames */
	for (; ((f < n_frame ) && ((seg[f] & 0x8000) == 0)); f++);
    }

    if (seg_ci != trans_ci) {
	E_ERROR("utt %s: phone %u, %s, in transcript does not match phone %s in seg at frame %u\n",
		utt_name,
		phone_i-1,
		acmod_set_id2name(acmod_set, trans_ci),
		acmod_set_id2name(acmod_set, seg_ci),
		f-1);
	
	return S3_ERROR;
    }

    if ((phone_i < n_phone) && (f == n_frame)) {
	/* i.e. got to end of seg sequence before end of phone seq */
	E_ERROR("utt_name %s: seg phone seq shorter than given phone seq\n",
		utt_name);
	
	return S3_ERROR;
    }

    if ((phone_i == n_phone) && (f < n_frame)) {
	/* i.e. got to end of phone sequence before end of seg seq */

	E_ERROR("utt_name %s: seg phone seq longer than given phone seq\n",
		utt_name);
	
	return S3_ERROR;
    }

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.4  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.3  96/06/17  14:32:40  eht
 * Fixed bug in error output for frame 0
 * 
 * Revision 1.2  1996/03/04  15:53:58  eht
 * Made into a common subroutine
 *
 * Revision 1.1  1996/02/27  16:41:19  eht
 * Initial revision
 *
 * Revision 1.1  1995/12/14  19:54:26  eht
 * Initial revision
 *
 *
 */
