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
 * File: feat.c
 * 
 * Description: 
 * 	Allows a caller to select an acoustic feature set and
 *	dispatches the functions to implement the selected set.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$";*/

#include "s2_feat.h"
#include "v1_feat.h"
#include "v2_feat.h"
#include "v3_feat.h"
#include "v4_feat.h"
#include "v5_feat.h"
#include "v6_feat.h"
#include "v7_feat.h"
#include "v8_feat.h"

#include "cep_frame.h"

#include <s3/feat.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

static uint32 fid = FEAT_ID_NONE;
static int mfcc_len = 13;

static char *__name2id[] = {
    "4s_12c_24d_3p_12dd",
    "1s_12c_12d_3p_12dd",
    "1s_c_d_dd",
    "1s_c_d",
    "1s_c",
    "1s_c_dd",
    "1s_d",
    "1s_dd",
    "1s_c_d_ld_dd",
    NULL
};

static char *name2id[] = {
    "c/1..L-1/,d/1..L-1/,c/0/d/0/dd/0/,dd/1..L-1/",
    "c/1..L-1/d/1..L-1/c/0/d/0/dd/0/dd/1..L-1/",
    "c/0..L-1/d/0..L-1/dd/0..L-1/",
    "c/0..L-1/d/0..L-1/",
    "c/0..L-1/",
    "c/0..L-1/dd/0..L-1/",
    NULL
};

static feat_conf_t feat_conf[FEAT_ID_MAX+1] = {
    { s2_feat_set_in_veclen,
      s2_feat_doc,
      s2_feat_id,
      s2_feat_n_stream,
      s2_feat_blksize,
      s2_feat_vecsize,
      s2_feat_alloc,
      s2_feat_free,
      s2_feat_compute,
      s2_feat_print },		/* FEAT_ID_SPHINX_II_STD */
    { v1_feat_set_in_veclen,
      v1_feat_doc,
      v1_feat_id,
      v1_feat_n_stream,
      v1_feat_blksize,
      v1_feat_vecsize,
      v1_feat_alloc,
      v1_feat_free,
      v1_feat_compute,
      v1_feat_print },		/* FEAT_ID_V1 */
    { v2_feat_set_in_veclen,
      v2_feat_doc,
      v2_feat_id,
      v2_feat_n_stream,
      v2_feat_blksize,
      v2_feat_vecsize,
      v2_feat_alloc,
      v2_feat_free,
      v2_feat_compute,
      v2_feat_print },		/* FEAT_ID_V2: 1s_c_d_dd */
    { v3_feat_set_in_veclen,
      v3_feat_doc,
      v3_feat_id,
      v3_feat_n_stream,
      v3_feat_blksize,
      v3_feat_vecsize,
      v3_feat_alloc,
      v3_feat_free,
      v3_feat_compute,
      v3_feat_print },		/* FEAT_ID_V3: 1s_c_d */
    { v4_feat_set_in_veclen,
      v4_feat_doc,
      v4_feat_id,
      v4_feat_n_stream,
      v4_feat_blksize,
      v4_feat_vecsize,
      v4_feat_alloc,
      v4_feat_free,
      v4_feat_compute,
      v4_feat_print },		/* FEAT_ID_V4: 1s_c */
    { v5_feat_set_in_veclen,
      v5_feat_doc,
      v5_feat_id,
      v5_feat_n_stream,
      v5_feat_blksize,
      v5_feat_vecsize,
      v5_feat_alloc,
      v5_feat_free,
      v5_feat_compute,
      v5_feat_print },		/* FEAT_ID_V5: 1s_c_dd */
    { v6_feat_set_in_veclen,
      v6_feat_doc,
      v6_feat_id,
      v6_feat_n_stream,
      v6_feat_blksize,
      v6_feat_vecsize,
      v6_feat_alloc,
      v6_feat_free,
      v6_feat_compute,
      v6_feat_print },		/* FEAT_ID_V5: 1s_d */
    { v7_feat_set_in_veclen,
      v7_feat_doc,
      v7_feat_id,
      v7_feat_n_stream,
      v7_feat_blksize,
      v7_feat_vecsize,
      v7_feat_alloc,
      v7_feat_free,
      v7_feat_compute,
      v7_feat_print },		/* FEAT_ID_V5: 1s_dd */
    { v8_feat_set_in_veclen,
      v8_feat_doc,
      v8_feat_id,
      v8_feat_n_stream,
      v8_feat_blksize,
      v8_feat_vecsize,
      v8_feat_alloc,
      v8_feat_free,
      v8_feat_compute,
      v8_feat_print }		/* FEAT_ID_V8: 1s_c_d_ld_dd */
};

int
feat_set(const char *id_name)
{
    uint32 i;

    for (i = 0; name2id[i]; i++) {
	if (strcmp(id_name, name2id[i]) == 0) {
	    fid = i;
	    break;
	}
    }

    if (name2id[i] == NULL) {
	for (i = 0; __name2id[i]; i++) {
	    if (strcmp(id_name, __name2id[i]) == 0) {
		fid = i;
		break;
	    }
	}
	if (__name2id[i] == NULL) {
	    E_ERROR("Unimplemented feature %s\n", id_name);
	    E_ERROR("Implemented features are:\n");
	    for (i = 0; name2id[i]; i++) {
		fprintf(stderr, "\t%s\n", name2id[i]);
	    }
	    fid = FEAT_ID_NONE;
	    
	    return S3_ERROR;
	}
    }

    return S3_SUCCESS;
}

void
feat_set_in_veclen(uint32 len)
{
    mfcc_len = len;

    if (fid <= FEAT_ID_MAX) {
	feat_conf[fid].set_in_veclen(len);
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }
}

uint32
feat_mfcc_len()
{
    return mfcc_len;
}

const char *
feat_doc()
{
    if (fid <= FEAT_ID_MAX) {
	return feat_conf[fid].doc();
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }

    return NULL;
}

uint32
feat_id()
{
    if (fid <= FEAT_ID_MAX) {
	assert(fid == feat_conf[fid].id());
	return fid;
    }
    else if (fid != FEAT_ID_NONE) {
	E_FATAL("feat module misconfigured with invalid fid %u\n", fid);
    }

    return FEAT_ID_NONE;
}

uint32
feat_n_stream()
{
    if (fid <= FEAT_ID_MAX) {
	return feat_conf[fid].n_stream();
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }

    return 0;
}

uint32
feat_blksize()
{
    if (fid <= FEAT_ID_MAX) {
	return feat_conf[fid].blksize();
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }

    return 0;
}

const uint32 *
feat_vecsize()
{
    if (fid <= FEAT_ID_MAX) {
	return feat_conf[fid].vecsize();
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }

    return NULL;
}

int
feat_ck_vecsize(const char *tag,
		const uint32 *vecsize,
		uint32 n_stream)
{
    uint32 f_n_s = feat_n_stream();
    uint32 i, e;
    const uint32 *f_vecsize = feat_vecsize();

    if (n_stream != f_n_s) {
	E_ERROR("%s: n_stream==%u not consistent w/ feature module (n_stream==%u)\n",
		n_stream, f_n_s);

	return S3_ERROR;
    }

    for (i = 0, e = 0; i < n_stream; i++) {
	if (vecsize[i] != f_vecsize[i]) {
	    e = 1;
	    E_ERROR("%s: given vecsize[%u]==%u != feature vecsize[%u]==%u\n",
		    i, vecsize[i], i, f_vecsize[i]);
	}
    }

    if (e)
	return S3_ERROR;
    else
	return S3_SUCCESS;
}

vector_t **
feat_alloc(uint32 n_frames)
{
    if (fid <= FEAT_ID_MAX) {
	return feat_conf[fid].alloc(n_frames);
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }

    return NULL;
}

void
feat_free(vector_t **f)
{
    if (fid <= FEAT_ID_MAX) {
	feat_conf[fid].free(f);
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }
}

vector_t **
feat_compute(vector_t *mfcc,
	     uint32 *inout_n_frame)
{
    if (fid <= FEAT_ID_MAX) {
	return feat_conf[fid].compute(mfcc, inout_n_frame);
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }

    return NULL;
}

void
feat_print_mfcc(vector_t *mfcc,
		uint32 n_frame)
{
    uint32 i, j;

    for (i = 0; i < n_frame; i++) {
	printf("mfcc[%u]:", i);
	for (j = 0; j < mfcc_len; j++) {
	    printf(" %.3e", mfcc[i][j]);
	}
	printf("\n");
    }
    printf("\n");
}

void
feat_print(const char *label,
	   vector_t **f,
	   uint32 n_frame)
{
    if (fid <= FEAT_ID_MAX) {
	feat_conf[fid].print(label, f, n_frame);
    }
    else if (fid == FEAT_ID_NONE) {
	E_FATAL("feat module must be configured w/ a valid ID\n");
    }
    else {
	E_FATAL("feat module misconfigured with invalid feat_id %u\n", fid);
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.12  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.11  1996/03/25  15:36:00  eht
 * Change feat_comp_id() to just feat_id().
 *
 * Revision 1.10  1996/01/26  18:04:51  eht
 * *** empty log message ***
 *
 * Revision 1.9  1995/12/14  20:05:37  eht
 * Make changes to allow multiple feature extractor definitions.
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
