/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * File: s3_feat_0.c
 * 
 * Description: 
 * 	Compute a unified feature stream (Ver 0)
 *
 *		f(t) = < s2_cep(t) s2_ddcep(t, 2) >
 *	
 *	Optionally does the following transformations on MFCC before computing the
 *	derived features above:
 *
 *		1. Cepstral mean normalization (based on current utt or prior
 *			utterances).
 *		2. Automatic gain control:
 *			- subtract utter max c[0] from all c[0]
 *			- subtract estimated utter max (based on prior utterances)
 *			  from all c[0] of current utterances.
 *		3. Silence deletion
 *			- based on c[0] histogram of current utterance
 *			- based on c[0] histogram of prior utterance
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$";*/

#include "v5_feat.h"

#include <s3/feat.h>

#include <s3/agc.h>
#include <s3/cmn.h>
#include <s3/silcomp.h>

#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/s3.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#define N_FEAT		1

static uint32 n_feat = N_FEAT;
static uint32 vecsize[1];
static uint32 mfcc_len;
static uint32 v5_delta_wsize = 2;
static uint32 v5_doubledelta_wsize = 1;

const char *
v5_feat_doc()
{
    return "1 stream :== < cep + ddcep >";
}

uint32
v5_feat_id()
{
    return FEAT_ID_V5;
}

uint32
v5_feat_n_stream()
{
    return n_feat;
}

uint32
v5_feat_blksize()
{
    return vecsize[0];
}

const uint32 *
v5_feat_vecsize()
{
    return vecsize;
}

void
v5_feat_set_in_veclen(uint32 veclen)
{
    mfcc_len = veclen;

    vecsize[0] = 2 * veclen;

    cmn_set_veclen(veclen);
    agc_set_veclen(veclen);
}

vector_t **
v5_feat_alloc(uint32 n_frames)
{
    vector_t **out;
    float *data;
    uint32 len;
    uint32 i, j, k;
    uint32 frame_size;

    out = (vector_t **)ckd_calloc_2d(n_frames, n_feat, sizeof(vector_t));
    
    for (i = 0, frame_size = 0; i < n_feat; i++)
	frame_size += vecsize[i];

    len = n_frames * frame_size;
    
    data = ckd_calloc(len, sizeof(float32));
    
    for (i = 0, k = 0; i < n_frames; i++) {
	
	assert((k % frame_size) == 0);
	
	for (j = 0; j < n_feat; j++) {
	    out[i][j] = &data[k];
	    k += vecsize[j];
	}
    }

    assert(k == len);

    return out;
}

void
v5_feat_free(vector_t **f)
{
    ckd_free(f[0][0]);		/* frees the data block */

    ckd_free_2d((void **)f);	/* frees the access overhead */
}


void
v5_doubledelta_frame(vector_t ddcep,
	          vector_t mfcc)
{
    int32 d_w;
    int32 dd_w;
    int32 mfcc_frame_len;
    int32 k;
    float32 d1;
    float32 d2;

    mfcc_frame_len = feat_mfcc_len();

    /* compute dcep window offset in terms of coefficients */
    d_w = v5_delta_wsize * mfcc_frame_len;

    /* compute ddcep window offset in terms of coefficients */
    dd_w = v5_doubledelta_wsize * mfcc_frame_len;

    for (k = 0; k < mfcc_frame_len; k++) {
	/* compute 2nd diff of mfcc[k] */

	/* dcep[k] of doubledelta_wsize frames in future */
	d1 = mfcc[k + d_w + dd_w] - mfcc[k - d_w + dd_w];

	/* dcep[k] of doubledelta_wsize frames in past */
	d2 = mfcc[k + d_w - dd_w] - mfcc[k - d_w - dd_w];

	ddcep[k] = d1 - d2;
    }
}

vector_t **
v5_feat_compute(vector_t *mfcc,
		uint32 *inout_n_frame)
{
    vector_t mfcc_frame;
    vector_t **out;
    vector_t out_frame;
    uint32 svd_n_frame;
    uint32 n_frame;
    const char *comp_type = cmd_ln_access("-silcomp");
    uint32 i, j;
    uint32 mfcc_len;
    uint32 cep_off;
    uint32 ddcep_off;

    uint32 dd_begin;
    uint32 dd_end;
    void v5_mfcc_print(vector_t *mfcc, uint32 n_frame);
    
    mfcc_len = feat_mfcc_len();	/* # of coefficients c[0..MFCC_LEN-1] per frame */

    cep_off    = 0;		/* cep feature is first */
    ddcep_off  = mfcc_len;	/* ddcep feature is mfcc_len long */
    
    n_frame = svd_n_frame = *inout_n_frame;

    n_frame = sil_compression(comp_type, mfcc, n_frame);

    /* set the begin and end frames for the 2nd diff cep */
    dd_begin = v5_doubledelta_wsize + v5_delta_wsize;
    dd_end   = n_frame - dd_begin;

    cmn(&mfcc[0][0], n_frame);
    agc(&mfcc[0][0], n_frame);

    
    out = v5_feat_alloc(n_frame);

    for (i = 0, j = 0; i < n_frame; i++, j += mfcc_len) {
	out_frame = out[i][0];
	mfcc_frame = mfcc[i];

        memcpy(out_frame, mfcc_frame, mfcc_len * sizeof(float32));

	if ((i >= dd_begin) && (i < dd_end)) {
	    v5_doubledelta_frame(out_frame + ddcep_off, mfcc_frame);
	}
    }

    /* Deal w/ 2nd diff boundaries */
    for (i = 0; i < dd_begin; i++) {
	memcpy(&out[i][0][ddcep_off],
	       &out[dd_begin][0][ddcep_off],
	       mfcc_len*sizeof(float32));
    }
    for (i = dd_end; i < n_frame; i++) {
	memcpy(&out[i][0][ddcep_off],
	       &out[dd_end-1][0][ddcep_off],
	       mfcc_len*sizeof(float32));
    }

    *inout_n_frame = n_frame;

    return out;
}

void
v5_mfcc_print(vector_t *mfcc, uint32 n_frame)
{
    uint32 i, k;
    uint32 mfcc_len;

    mfcc_len = feat_mfcc_len();

    for (i = 0; i < n_frame; i++) {
	printf("mfcc[%04u]: ", i);
	for (k = 0; k < mfcc_len; k++) {
	    printf("%6.3f ", mfcc[i][k]);
	}
	printf("\n");
    }
}


void
v5_feat_print(const char *label,
	      vector_t **f,
	      uint32 n_frames)
{
    uint32 i;
    int32 j;
    uint32 k;
    char *name[] = { "" };
    vector_t *frame;
    vector_t stream;

    for (i = 0; i < n_frames; i++) {
	frame = f[i];

	for (j = 0; j < n_feat; j++) {
	    stream = frame[j];

	    printf("%s%s[%04u]: ", label, name[j], i);
	    for (k = 0; k < vecsize[j]; k++) {
		printf("%6.3f ", stream[k]);
	    }
	    printf("\n");
	}
	printf("\n");
    }
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
 * Revision 1.3  1996/03/25  15:36:31  eht
 * Changes to allow for settable input feature length
 *
 * Revision 1.2  1996/01/26  18:04:51  eht
 * *** empty log message ***
 *
 * Revision 1.1  1995/12/14  20:12:58  eht
 * Initial revision
 *
 */
