/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: s2_feat.c
 * 
 * Description: 
 * 	Compute standard SPHINX-II derived cepstrum features.
 *	Takes the standard 13 component MFCC vector, MFCC, and computes 4
 *	independent feature streams:
 *
 *		  cep: MFCC[1..12]
 *		 dcep: < (cep[t + 2] - cep[t - 2]), (cep[t + 4] - cep[t - 4]) >
 *		  pow: < MFCC0[t], MFCC0[t+2] - MFCC0[t-2], pow[t+1][1] - pow[t-1][1]>
 *		ddcep: < dcep[t+1] - dcep[t-1] >
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

/* static char rcsid[] = "@(#)$Id$"; */

#include "s2_feat.h"

#include <s3/feat.h>

#include "s2_cep.h"
#include "s2_dcep.h"
#include "s2_ddcep.h"

#include <s3/agc.h>
#include <s3/cmn.h>
#include <s3/silcomp.h>

#include <s3/s2_param.h>

#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/s3.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

static uint32 n_feat = S2_N_FEATURE;
static uint32 in_veclen = 0;

static uint32 vecsize[S2_N_FEATURE] = {
    0,
    0,
    0,
    0
};

const char *
s2_feat_doc()
{
    return "SPHINX-II 4 stream (12 cep, 24 dcep, 3 pow, 12 ddcep)";
}

uint32
s2_feat_id()
{
    return FEAT_ID_SPHINX_II_STD;
}

uint32
s2_feat_n_stream()
{
    return S2_N_FEATURE;
}

void
s2_feat_set_in_veclen(uint32 veclen)
{
    in_veclen = veclen;

    vecsize[0] = in_veclen - 1;
    vecsize[1] = 2*vecsize[0];
    vecsize[2] = 3;
    vecsize[3] = vecsize[0];

    cmn_set_veclen(veclen);
    agc_set_veclen(veclen);
    s2_cep_set_veclen(veclen);
    s2_dcep_set_veclen(veclen);
    s2_ddcep_set_veclen(veclen);
}
uint32
s2_feat_blksize()
{
    int i;
    uint32 size;

    for (i = 0, size = 0; i < s2_feat_n_stream(); i++) {
	size += vecsize[i];
    }

    return size;
}

const uint32 *
s2_feat_vecsize()
{
    return vecsize;
}

vector_t **
s2_feat_alloc(uint32 n_frames)
{
    vector_t **out;
    float *data;
    uint32 len;
    uint32 i, j, k;
    uint32 frame_size;

    out = (vector_t **)ckd_calloc_2d(n_frames, S2_N_FEATURE, sizeof(vector_t));

    if (vecsize[0] == 0) {
	E_FATAL("Call s2_feat_set_in_veclen() before s2_feat_alloc()\n");
    }
    
    for (i = 0, frame_size = 0; i < n_feat; i++)
	frame_size += vecsize[i];

    len = n_frames * frame_size;
    
    data = ckd_calloc(len, sizeof(float32));
    
    for (i = 0, k = 0; i < n_frames; i++) {

	assert((k % frame_size) == 0);

	for (j = 0; j < S2_N_FEATURE; j++) {
	    out[i][j] = &data[k];
	    k += vecsize[j];
	}
    }

    assert(k == len);

    return out;
}

void
s2_feat_free(vector_t **f)
{
    ckd_free(f[0][0]);		/* frees the data block */

    ckd_free_2d((void **)f);	/* frees the access overhead */
}

vector_t **
s2_feat_compute(vector_t *mfcc,
		uint32 *inout_n_frame)
{
    vector_t **out;
    uint32 svd_n_frame, n_frame;
    const char *comp_type = cmd_ln_access("-silcomp");
    
    
    n_frame = svd_n_frame = *inout_n_frame;

    if (strcmp(comp_type, "none") != 0) {
	n_frame = sil_compression(comp_type, mfcc, n_frame);
    }
    cmn(&mfcc[0][0], n_frame);
    agc(&mfcc[0][0], n_frame);

    if (svd_n_frame - n_frame > 0) {
	fprintf(stdout, "(del %d frames) ", svd_n_frame - n_frame);
	fflush(stdout);
    }
    
    out = s2_feat_alloc(n_frame);
    
    s2_cep_feat(out, mfcc, n_frame);		/* see s2_cep.c */
    s2_short_dcep_feat(out, mfcc, n_frame);	/* see s2_dcep.c */
    s2_long_dcep_feat(out, mfcc, n_frame);	/* see s2_dcep.c */
    s2_sec_ord_dcep_feat(out, mfcc, n_frame);	/* see s2_ddcep.c */

    *inout_n_frame = n_frame;

    return out;
}

void
s2_feat_print(const char *label,
	      vector_t **f,
	      uint32 n_frames)
{
    uint32 i;
    int32 j;
    uint32 k;
    char *name[] = {
	"  cep",
	" dcep",
	"  pow",
	"ddcep"
    };
    vector_t *frame;
    vector_t stream;

    for (i = 0; i < n_frames; i++) {
	frame = f[i];

	for (j = 0; j < S2_N_FEATURE; j++) {
	    stream = frame[j];

	    printf("%s%s[%04u]: ", label, name[j], i);
	    for (k = 0; k < vecsize[j]; k++) {
		printf("%.3e ", stream[k]);
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
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.6  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.5  1996/03/25  15:36:31  eht
 * Changes to allow for settable input feature length
 *
 * Revision 1.4  1996/01/26  18:04:51  eht
 * *** empty log message ***
 *
 * Revision 1.3  1995/12/14  20:12:58  eht
 * SPHINX-II feature extractor
 *
 * Revision 1.2  1995/12/07  19:44:22  eht
 * Added some comments and changed the interface name to s2_*()
 *
 * Revision 1.1  1995/12/07  19:29:27  eht
 * Initial revision
 *
 * Revision 1.8  1995/12/07  19:20:09  eht
 * Added a function to return the total size of a frames worth of
 * features
 *
 * Revision 1.7  1995/12/04  14:59:55  eht
 * Added a feat_n_stream() function so that callers of this
 * module can know how many feature streams it computes.
 *
 * Revision 1.6  1995/10/12  17:39:31  eht
 * Use memcpy rather than bcopy since memcpy is a part
 * of the ANSI-C specification.
 *
 * Revision 1.5  1995/10/10  17:38:24  eht
 * Include some prototypes for called functions
 * Make some unsigned values consistent
 *
 * Revision 1.4  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.3  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.2  1995/09/07  19:04:37  eht
 * Fixed latent bug in argument passing to the silence
 * compression stuff.
 *
 * Revision 1.1  95/06/02  14:52:54  14:52:54  eht (Eric Thayer)
 * Initial revision
 * 
 *
 */
