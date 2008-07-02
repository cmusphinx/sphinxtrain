/* -*- c-basic-offset: 4 -*- */
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
#include <s3/ckd_alloc.h>
#include <s3/lda.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

static uint32 fid = FEAT_ID_NONE;
static unsigned int mfcc_len = 13;

/**
 * Variables related to LDA/MLLT.
 */
static struct {
    float32 ***lda;
    uint32 n_lda, lda_rows, lda_cols, lda_dim;
} lda;

/**
 * Variables related to subvector projection.
 */
static struct {
    uint32 *sv_len;
    int32 **subvecs;
    float32 *sv_buf;
    int32 sv_dim;
    int32 n_sv;
} sv;


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
    "d/0..L-1/",
    "dd/0..L-1/",
    NULL
};

static feat_conf_t feat_conf[FEAT_ID_MAX+1] = {
    { s2_feat_set_in_veclen,
      s2_feat_doc,
      s2_feat_id,
      s2_feat_n_stream,
      s2_feat_blksize,
      s2_feat_vecsize,
      s2_feat_compute,
      s2_feat_print },		/* FEAT_ID_SPHINX_II_STD */
    { v1_feat_set_in_veclen,
      v1_feat_doc,
      v1_feat_id,
      v1_feat_n_stream,
      v1_feat_blksize,
      v1_feat_vecsize,
      v1_feat_compute,
      v1_feat_print },		/* FEAT_ID_V1 */
    { v2_feat_set_in_veclen,
      v2_feat_doc,
      v2_feat_id,
      v2_feat_n_stream,
      v2_feat_blksize,
      v2_feat_vecsize,
      v2_feat_compute,
      v2_feat_print },		/* FEAT_ID_V2: 1s_c_d_dd */
    { v3_feat_set_in_veclen,
      v3_feat_doc,
      v3_feat_id,
      v3_feat_n_stream,
      v3_feat_blksize,
      v3_feat_vecsize,
      v3_feat_compute,
      v3_feat_print },		/* FEAT_ID_V3: 1s_c_d */
    { v4_feat_set_in_veclen,
      v4_feat_doc,
      v4_feat_id,
      v4_feat_n_stream,
      v4_feat_blksize,
      v4_feat_vecsize,
      v4_feat_compute,
      v4_feat_print },		/* FEAT_ID_V4: 1s_c */
    { v5_feat_set_in_veclen,
      v5_feat_doc,
      v5_feat_id,
      v5_feat_n_stream,
      v5_feat_blksize,
      v5_feat_vecsize,
      v5_feat_compute,
      v5_feat_print },		/* FEAT_ID_V5: 1s_c_dd */
    { v6_feat_set_in_veclen,
      v6_feat_doc,
      v6_feat_id,
      v6_feat_n_stream,
      v6_feat_blksize,
      v6_feat_vecsize,
      v6_feat_compute,
      v6_feat_print },		/* FEAT_ID_V5: 1s_d */
    { v7_feat_set_in_veclen,
      v7_feat_doc,
      v7_feat_id,
      v7_feat_n_stream,
      v7_feat_blksize,
      v7_feat_vecsize,
      v7_feat_compute,
      v7_feat_print },		/* FEAT_ID_V5: 1s_dd */
    { v8_feat_set_in_veclen,
      v8_feat_doc,
      v8_feat_id,
      v8_feat_n_stream,
      v8_feat_blksize,
      v8_feat_vecsize,
      v8_feat_compute,
      v8_feat_print }		/* FEAT_ID_V8: 1s_c_d_ld_dd */
};

int
feat_set(const char *id_name)
{
    uint32 i;

    /* HACK: "s2_4x" is an alias for sphinx-II features. */
    if (strcmp(id_name, "s2_4x") == 0) {
	fid = FEAT_ID_SPHINX_II_STD;
	return S3_SUCCESS;
    }

    /* HACK, continued: "s3_1x39" is an alias for one of the sphinx-III variants. */
    if (strcmp(id_name, "s3_1x39") == 0) {
	fid = FEAT_ID_V1;
	return S3_SUCCESS;
    }

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

int32
feat_read_lda(const char *ldafile, uint32 dim)
{
    if (lda.lda != NULL)
	ckd_free_3d((void ***)lda.lda);
    lda.lda = lda_read(ldafile, &lda.n_lda, &lda.lda_rows, &lda.lda_cols);
    if (lda.lda == NULL)
	return S3_ERROR;
    lda.lda_dim = dim;

    assert(lda.lda_cols == feat_conf[fid].blksize());
    return S3_SUCCESS;
}

/**
 * Project feature components to subvectors (if any).
 */
static void
feat_subvec_project(vector_t **inout_feat, uint32 nfr)
{
    uint32 i;

    for (i = 0; i < nfr; ++i) {
        float32 *out;
        int32 j;

        out = sv.sv_buf;
        for (j = 0; j < sv.n_sv; ++j) {
            int32 *d;
            for (d = sv.subvecs[j]; d && *d != -1; ++d) {
                *out++ = inout_feat[i][0][*d];
            }
        }
        memcpy(inout_feat[i][0], sv.sv_buf, sv.sv_dim * sizeof(*sv.sv_buf));
    }
}

static void
subvecs_free(int32 **subvecs)
{
    int32 **sv;

    for (sv = subvecs; sv && *sv; ++sv)
        ckd_free(*sv);
    ckd_free(subvecs);
}

static int32 **
feat_set_subvecs_internal(int32 **subvecs)
{
    int32 **s;
    int32 n_sv, n_dim, i;
    uint32 n_stream = feat_conf[fid].n_stream();
    uint32 feat_n_dim = feat_conf[fid].blksize();

    if (subvecs == NULL) {
        subvecs_free(sv.subvecs);
        ckd_free(sv.sv_buf);
        ckd_free(sv.sv_len);
        sv.n_sv = 0;
        sv.subvecs = NULL;
        sv.sv_len = NULL;
        sv.sv_buf = NULL;
        sv.sv_dim = 0;
        return NULL;
    }

    if (n_stream != 1) {
        E_ERROR("Subvector specifications require single-stream features!");
        return NULL;
    }

    n_sv = 0;
    n_dim = 0;
    for (s = subvecs; s && *s; ++s) {
        int32 *d;

        for (d = *s; d && *d != -1; ++d) {
            ++n_dim;
        }
        ++n_sv;
    }
    if (n_dim > feat_n_dim) {
        E_ERROR("Total dimensionality of subvector specification %d "
                "> feature dimensionality %d\n", n_dim, feat_n_dim);
        return NULL;
    }

    sv.n_sv = n_sv;
    sv.subvecs = subvecs;
    sv.sv_len = ckd_calloc(n_sv, sizeof(*sv.sv_len));
    sv.sv_buf = ckd_calloc(n_dim, sizeof(*sv.sv_buf));
    sv.sv_dim = n_dim;
    for (i = 0; i < n_sv; ++i) {
        int32 *d;
        for (d = subvecs[i]; d && *d != -1; ++d) {
            ++sv.sv_len[i];
        }
    }

    return sv.subvecs;
}

int32 **
feat_set_subvecs(char const *str)
{
    char const *strp;
    int32 n, n2, l;
    /* Grumble. */
    struct int_list {
	int32 x;
	struct int_list *next;
    };
    struct int_list *dimlist;            /* List of dimensions in one subvector */
    struct int_list *il;
    struct int_list_list {
	struct int_list *x;
	struct int_list_list *next;
    };
    struct int_list_list *veclist;            /* List of dimlists (subvectors) */
    struct int_list_list *ill;
    int32 **subvec;
    int32 n_sv;

    if (str == NULL)
      return feat_set_subvecs_internal(NULL);

    veclist = NULL;

    strp = str;
    n_sv = 0;
    for (;;) {
        dimlist = NULL;

        for (;;) {
            if (sscanf(strp, "%d%n", &n, &l) != 1)
                E_FATAL("'%s': Couldn't read int32 @pos %d\n", str,
                        strp - str);
            strp += l;

            if (*strp == '-') {
                strp++;

                if (sscanf(strp, "%d%n", &n2, &l) != 1)
                    E_FATAL("'%s': Couldn't read int32 @pos %d\n", str,
                            strp - str);
                strp += l;
            }
            else
                n2 = n;

            if ((n < 0) || (n > n2))
                E_FATAL("'%s': Bad subrange spec ending @pos %d\n", str,
                        strp - str);

            for (; n <= n2; n++) {
		for (il = dimlist; il; il = il->next)
		    if (il->x == n)
			break;
		if (il != NULL)
                    E_FATAL("'%s': Duplicate dimension ending @pos %d\n",
                            str, strp - str);

                il = ckd_calloc(1, sizeof(*il));
		il->x = n;
		il->next = dimlist;
		dimlist = il;
            }

            if ((*strp == '\0') || (*strp == '/'))
                break;

            if (*strp != ',')
                E_FATAL("'%s': Bad delimiter @pos %d\n", str, strp - str);

            strp++;
        }

	ill = ckd_calloc(1, sizeof(*ill));
	ill->x = dimlist;
	ill->next = veclist;
	veclist = ill;
	++n_sv;

        if (*strp == '\0')
            break;

        assert(*strp == '/');
        strp++;
    }

    /* Convert the glists to arrays; remember the glists are in reverse order of the input! */
    subvec = (int32 **) ckd_calloc(n_sv + 1, sizeof(int32 *));     /* +1 for sentinel */
    subvec[n_sv] = NULL;           /* sentinel */
    n = n_sv;
    for (--n, ill = veclist; (n >= 0) && ill; ill = ill->next, --n) {
	n2 = 0;
	for (il = ill->x; il; il = il->next)
	    ++n2;

        if (n2 <= 0)
            E_FATAL("'%s': 0-length subvector\n", str);

        subvec[n] = (int32 *) ckd_calloc(n2 + 1, sizeof(int32));        /* +1 for sentinel */
        subvec[n][n2] = -1;     /* sentinel */

	il = ill->x;
        for (--n2; (n2 >= 0) && il; il = il->next, --n2)
            subvec[n][n2] = il->x;
        assert((n2 < 0) && (il == NULL));
    }
    assert((n < 0) && (ill == NULL));

    /* Free the glists */
    {
	struct int_list_list *illn;
	struct int_list *iln;
	
	for (ill = veclist; ill; ill = illn) {
	    illn = ill->next;
	    for (il = ill->x; il; il = iln) {
		iln = il->next;
		ckd_free(il);
	    }
	    ckd_free(ill);
	}
    }

    if (feat_set_subvecs_internal(subvec) == NULL) {
	    subvecs_free(subvec);
	    return NULL;
    }
    return subvec;
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
    if (sv.subvecs != NULL) {
	return sv.n_sv;
    }
    else if (fid <= FEAT_ID_MAX) {
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
    if (sv.subvecs != NULL) {
	return sv.sv_dim;
    }
    else if (lda.lda != NULL) {
	return lda.lda_dim;
    }
    else if (fid <= FEAT_ID_MAX) {
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
    if (sv.subvecs != NULL) {
	return sv.sv_len;
    }
    else if (lda.lda != NULL) {
	return &lda.lda_dim;
    }
    else if (fid <= FEAT_ID_MAX) {
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

static vector_t **
feat_alloc_internal(uint32 n_frames, uint32 n_feat, uint32 const *vecsize)
{
    vector_t **out;
    float *data, *d;
    uint32 len;
    uint32 i, j;
    uint32 frame_size;

    out = (vector_t **)ckd_calloc_2d(n_frames, n_feat, sizeof(vector_t));

    /* Take the maximum of the original block size (do NOT call
       feat_blksize() because it will give the LDA output
       dimensionality) and the subvector block size. */
    frame_size = feat_conf[fid].blksize();
    if (sv.subvecs != NULL) {
	if (sv.sv_dim > frame_size)
	    frame_size = sv.sv_dim;
    }

    len = n_frames * frame_size;
    
    data = ckd_calloc(len, sizeof(float32));
    
    for (i = 0; i < n_frames; i++) {
	d = data + i * frame_size;
	for (j = 0; j < n_feat; j++) {
	    out[i][j] = d;
	    d += vecsize[j];
	}
    }

    return out;
}

vector_t **
feat_alloc(uint32 n_frames)
{
    if (fid <= FEAT_ID_MAX) {
	return feat_alloc_internal(n_frames, feat_n_stream(), feat_vecsize());
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
	ckd_free(f[0][0]);		/* frees the data block */
	ckd_free_2d((void **)f);	/* frees the access overhead */
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
    vector_t **feat;

    if (fid <= FEAT_ID_MAX) {
	feat = feat_conf[fid].compute(mfcc, inout_n_frame);
	if (lda.lda)
	    lda_transform(feat, *inout_n_frame, lda.lda, lda.lda_cols, lda.lda_dim);
	if (sv.subvecs)
	    feat_subvec_project(feat, *inout_n_frame);
	return feat;
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
