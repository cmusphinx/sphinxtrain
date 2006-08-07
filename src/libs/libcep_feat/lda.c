/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All rights
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
/*
 * lda.c -- Read and apply LDA matrices to features.
 *
 * Author: David Huggins-Daines <dhuggins@cs.cmu.edu>
 */

#include "s3/feat.h"
#include "s3/ckd_alloc.h"
#include "s3/s3io.h"
#include "s3/err.h"

#include <assert.h>
#include <string.h>

#define MATRIX_FILE_VERSION "0.1"

float32 ***
lda_read(const char *ldafile, uint32 *out_n_lda, uint32 *out_m, uint32 *out_n)
{
    FILE *fh;
    uint32 byteswap;
    uint32 chksum, n_lda, m, n;
    const char *do_chk, *ver;
    float32 ***lda;

    if (feat_n_stream() != 1) {
        E_ERROR("LDA incompatible with multi-stream features (n_stream = %d)\n",
                feat_n_stream());
        return NULL;
    }

    if ((fh = s3open(ldafile, "rb", &byteswap)) == NULL) {
        E_ERROR_SYSTEM("s3open(%s, rb) failed", ldafile);
        return NULL;
    }

    /* check version id */
    ver = s3get_gvn_fattr("version");
    if (ver) {
	if (strcmp(ver, MATRIX_FILE_VERSION) != 0) {
	    E_FATAL("Version mismatch for %s, file ver: %s != reader ver: %s\n",
		    ldafile, ver, MATRIX_FILE_VERSION);
	}
    }
    else {
	E_FATAL("No version attribute for %s\n", ldafile);
    }
    
    /* if do_chk is non-NULL, there is a checksum after the data in the file */
    do_chk = s3get_gvn_fattr("chksum0");

    if (do_chk && !strcmp(do_chk, "no")) {
        do_chk = NULL;
    }

    chksum = 0;

    if (s3read_3d((void ****)&lda, sizeof(float32),
                  &n_lda, &m, &n,
                  fh, byteswap, &chksum) < 0) {
        E_ERROR_SYSTEM("%s: bio_fread_3d(lda) failed\n", ldafile);
        fclose(fh);
        return NULL;
    }

    if (out_n_lda) *out_n_lda = n_lda;
    if (out_m) *out_m = m;
    if (out_n) *out_n = n;

    return lda;
}

void
lda_transform(float32 ***inout_feat, uint32 nfr,
              float32 ***lda, uint32 veclen, uint32 dim)
{
    float32 *tmp;
    uint32 i, j, k;

    tmp = ckd_calloc(veclen, sizeof(float32));
    for (i = 0; i < nfr; ++i) {
        /* Do the matrix multiplication inline here since fcb->lda
         * is transposed (eigenvectors in rows not columns). */
        /* FIXME: In the future we ought to use the BLAS. */
        memset(tmp, 0, sizeof(float32)*veclen);
        for (j = 0; j < dim; ++j) {
            for (k = 0; k < veclen; ++k) {
                tmp[j] += inout_feat[i][0][k] * lda[0][j][k];
            }
        }
        memcpy(inout_feat[i][0], tmp, veclen * sizeof(float32));
    }
    ckd_free(tmp);
}
