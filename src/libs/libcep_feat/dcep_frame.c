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
 * File: dcep_frame.c
 * 
 * Description: 
 * 
 * Author:
 * 
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */


#include "dcep_frame.h"

#include <s3/feat.h>

static uint32 s_wsize = 2;
static uint32 l_wsize = 4;

void
dcep_frame_set_long_window_size(uint32 l_ws)
{
    l_wsize = l_ws;
}

uint32
dcep_frame_long_window_size()
{
    return l_wsize;
}

void
dcep_frame_set_short_window_size(uint32 s_ws)
{
    s_wsize = s_ws;
}

uint32
dcep_frame_short_window_size()
{
    return s_wsize;
}

/* compute just the short dcep_frame */
void
short_dcep_frame(vector_t dcep,
		 vector_t power,
		 vector_t mfcc)
{
    int32 s_w;
    int32 k;
    int32 mfcc_len;
    
    mfcc_len = feat_mfcc_len();

    s_w = s_wsize * mfcc_len;

    power[1] = mfcc[s_w] - mfcc[-s_w];

    for (k = 1; k < mfcc_len; k++) {
	/* compute the short duration diff cep */
	dcep[k-1] = mfcc[k + s_w] - mfcc[k - s_w];
    }
}

void
dcep_frame(vector_t dcep,
	   vector_t power,
	   vector_t mfcc)
{
    int32 s_w;
    int32 l_w;
    int32 k;
    int32 mfcc_len;
    int32 l_offset;
    
    mfcc_len = feat_mfcc_len();
    l_offset = mfcc_len-1;

    s_w = s_wsize * mfcc_len;
    l_w = l_wsize * mfcc_len;

    power[1] = mfcc[s_w] - mfcc[-s_w];

    for (k = 1; k < mfcc_len; k++) {
	/* compute the short duration diff cep */
	dcep[k-1] = mfcc[k + s_w] - mfcc[k - s_w];

	/* compute the long duration diff cep */
	dcep[k-1 + l_offset] =
	    mfcc[k + l_w] - mfcc[k - l_w];
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:05:38  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  1996/08/05  12:55:03  eht
 * Include <s3/feat.h> for prototypes
 *
 * Revision 1.1  1996/01/26  18:04:51  eht
 * Initial revision
 *
 *
 */
