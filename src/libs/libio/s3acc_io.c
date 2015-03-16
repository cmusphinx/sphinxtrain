/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights 
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
 * File: s3acc_io.c
 * 
 * Description: 
 *     Reading routines for the accumulators. 
 * Author: 
 *     Arthur Chan (archan@cs.cmu.edu)
 *********************************************************************/

#include <s3/s3acc_io.h>
#include <s3/s3tmat_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3gau_io.h>
#include <sphinxbase/matrix.h>
#include <s3/s3io.h>
#include <s3/gauden.h>

#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>

#include <sys_compat/file.h>
#include <sys_compat/misc.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
rdacc_tmat(const char *dir,
           float32 **** inout_tmat_acc,
           uint32 * inout_n_tmat, uint32 * inout_n_state_pm)
{
    char fn[MAXPATHLEN + 1];
    float32 ***in_tmat_acc;
    float32 ***tmat_acc;
    uint32 n_tmat;
    uint32 n_state_pm;

    sprintf(fn, "%s/tmat_counts", dir);

    if (s3tmat_read(fn,
                    &in_tmat_acc,
                    &n_tmat, &n_state_pm) != S3_SUCCESS) {
        return S3_ERROR;
    }
    tmat_acc = *inout_tmat_acc;

    if (tmat_acc == NULL) {
        *inout_tmat_acc = tmat_acc = in_tmat_acc;
        *inout_n_tmat = n_tmat;
        *inout_n_state_pm = n_state_pm;
    }
    else {
        int err = FALSE;

        if (*inout_n_tmat != n_tmat) {
            E_ERROR("# tmat in, %u != prior # tmat, %u\n",
                    n_tmat, *inout_n_tmat);
            err = TRUE;
        }
        if (*inout_n_state_pm != n_state_pm) {
            E_ERROR("# tmat in, %u != prior # tmat, %u\n",
                    n_state_pm, *inout_n_state_pm);
            err = TRUE;
        }

        if (err)
            return S3_ERROR;

        accum_3d(tmat_acc, in_tmat_acc,
                 n_tmat, n_state_pm - 1, n_state_pm);

        ckd_free_3d((void ***) in_tmat_acc);
    }

    return S3_SUCCESS;
}

int
rdacc_mixw(const char *dir,
           float32 **** inout_mixw_acc,
           uint32 * inout_n_mixw,
           uint32 * inout_n_stream, uint32 * inout_n_density)
{
    char fn[MAXPATHLEN + 1];
    float32 ***in_mixw_acc;
    float32 ***mixw_acc;
    uint32 n_mixw;
    uint32 n_stream;
    uint32 n_density;

    sprintf(fn, "%s/mixw_counts", dir);

    if (s3mixw_read(fn,
                    &in_mixw_acc,
                    &n_mixw, &n_stream, &n_density) != S3_SUCCESS) {
        return S3_ERROR;
    }

    mixw_acc = *inout_mixw_acc;

    if (mixw_acc == NULL) {
        *inout_mixw_acc = mixw_acc = in_mixw_acc;
        *inout_n_mixw = n_mixw;
        *inout_n_stream = n_stream;
        *inout_n_density = n_density;
    }
    else {
        if (*inout_n_mixw != n_mixw) {
            E_ERROR
                ("# mixw in file %s (== %u) != prior # mixw (== %u)\n",
                 fn, n_mixw, *inout_n_mixw);
            return S3_ERROR;
        }

        if (*inout_n_stream != n_stream) {
            E_ERROR
                ("# stream in file %s (== %u) != prior # stream (== %u)\n",
                 fn, n_stream, *inout_n_stream);
            return S3_ERROR;
        }

        if (*inout_n_density != n_density) {
            E_ERROR
                ("# density comp/mix in file %s (== %u) != prior # density (== %u)\n",
                 fn, n_density, *inout_n_density);
            return S3_ERROR;
        }

        accum_3d(mixw_acc, in_mixw_acc, n_mixw, n_stream, n_density);

        ckd_free_3d((void ***) in_mixw_acc);
    }

    return S3_SUCCESS;
}

int
rdacc_den(const char *dir,
          vector_t **** inout_wt_mean,
          vector_t **** inout_wt_var,
          int32 * inout_pass2var,
          float32 **** inout_dnom,
          uint32 * inout_n_mgau,
          uint32 * inout_n_stream,
          uint32 * inout_n_density, uint32 ** inout_veclen)
{
    char fn[MAXPATHLEN + 1];
    vector_t ***in_wt_mean;
    vector_t ***wt_mean;
    vector_t ***in_wt_var;
    vector_t ***wt_var;
    float32 ***in_dnom;
    float32 ***dnom;
    uint32 n_mgau;
    uint32 n_stream;
    uint32 n_density;
    int32 pass2var;
    int i;

    uint32 *in_veclen;

    sprintf(fn, "%s/gauden_counts", dir);

    if (s3gaucnt_read(fn,
                      &in_wt_mean,
                      &in_wt_var,
                      &pass2var,
                      &in_dnom,
                      &n_mgau,
                      &n_stream, &n_density, &in_veclen) != S3_SUCCESS) {
        fflush(stdout);
        perror(fn);

        return S3_ERROR;
    }

    wt_mean = *inout_wt_mean;
    wt_var = *inout_wt_var;
    dnom = *inout_dnom;

    if (wt_mean == NULL) {

        /* if a gauden_counts file exists, it will have reestimated means */

        *inout_wt_mean = wt_mean = in_wt_mean;
        *inout_dnom = dnom = in_dnom;
        *inout_n_mgau = n_mgau;
        *inout_n_stream = n_stream;
        *inout_n_density = n_density;
        *inout_veclen = in_veclen;
        *inout_pass2var = pass2var;

        if (wt_var == NULL && in_wt_var != NULL) {
            *inout_wt_var = wt_var = in_wt_var;
        }
    }
    else {
        int err = FALSE;

        /* check if matrices are able to be added */
        if (*inout_n_mgau != n_mgau) {
            E_ERROR
                ("# mix. Gau. for file %s (== %u) != prior # mix. Gau. (== %u)\n",
                 fn, n_mgau, *inout_n_mgau);
            err = TRUE;
        }

        if (*inout_n_stream != n_stream) {
            E_ERROR
                ("# stream for file %s (== %u) != prior # stream (== %u)\n",
                 fn, n_stream, *inout_n_stream);
            err = TRUE;
        }

        if (*inout_n_density != n_density) {
            E_ERROR
                ("# density comp/mix for file %s (== %u) != prior # density, %u\n",
                 fn, n_density, *inout_n_density);
            err = TRUE;
        }

        if (*inout_pass2var != pass2var) {
            E_ERROR("2 pass var %s in %s, but %s in others.\n",
                    fn, (pass2var ? "true" : "false"),
                    (*inout_pass2var ? "true" : "false"));
            err = TRUE;
        }

        for (i = 0; i < n_stream; i++) {
            if ((*inout_veclen)[i] != in_veclen[i]) {
                E_ERROR
                    ("vector length of stream %u (== %u) != prior length (== %u)\n",
                     i, in_veclen[i], (*inout_veclen)[i]);
                err = TRUE;
            }
        }

        ckd_free(in_veclen);

        if (err)
            return S3_ERROR;

        /* accumulate values */
        accum_3d(dnom, in_dnom, n_mgau, n_stream, n_density);

        gauden_accum_param(wt_mean, in_wt_mean,
                           n_mgau, n_stream, n_density, *inout_veclen);
        gauden_free_param(in_wt_mean);

        if (wt_var) {
            assert(in_wt_var);

            gauden_accum_param(wt_var, in_wt_var,
                               n_mgau, n_stream, n_density, *inout_veclen);
            gauden_free_param(in_wt_var);
        }
    }

    return S3_SUCCESS;
}

int
rdacc_den_full(const char *dir,
               vector_t **** inout_wt_mean,
               vector_t ***** inout_wt_var,
               int32 * inout_pass2var,
               float32 **** inout_dnom,
               uint32 * inout_n_mgau,
               uint32 * inout_n_stream,
               uint32 * inout_n_density, uint32 ** inout_veclen)
{
    char fn[MAXPATHLEN + 1];
    vector_t ***in_wt_mean;
    vector_t ***wt_mean;
    vector_t ****in_wt_var;
    vector_t ****wt_var;
    float32 ***in_dnom;
    float32 ***dnom;
    uint32 n_mgau;
    uint32 n_stream;
    uint32 n_density;
    uint32 *in_veclen;
    int32 pass2var;
    int i;

    sprintf(fn, "%s/gauden_counts", dir);

    if (s3gaucnt_read_full(fn,
                           &in_wt_mean,
                           &in_wt_var,
                           &pass2var,
                           &in_dnom,
                           &n_mgau,
                           &n_stream,
                           &n_density, &in_veclen) != S3_SUCCESS) {
        fflush(stdout);
        perror(fn);

        return S3_ERROR;
    }

    wt_mean = *inout_wt_mean;
    wt_var = *inout_wt_var;
    dnom = *inout_dnom;

    if (wt_mean == NULL) {
        /* if a gauden_counts file exists, it will have reestimated means */
        *inout_wt_mean = wt_mean = in_wt_mean;
        *inout_dnom = dnom = in_dnom;
        *inout_n_mgau = n_mgau;
        *inout_n_stream = n_stream;
        *inout_n_density = n_density;
        *inout_veclen = in_veclen;
        *inout_pass2var = pass2var;

        if (wt_var == NULL && in_wt_var != NULL) {
            *inout_wt_var = wt_var = in_wt_var;
        }
    }
    else {
        int err = FALSE;

        /* check if matrices are able to be added */
        if (*inout_n_mgau != n_mgau) {
            E_ERROR
                ("# mix. Gau. for file %s (== %u) != prior # mix. Gau. (== %u)\n",
                 fn, n_mgau, *inout_n_mgau);
            err = TRUE;
        }

        if (*inout_n_stream != n_stream) {
            E_ERROR
                ("# stream for file %s (== %u) != prior # stream (== %u)\n",
                 fn, n_stream, *inout_n_stream);
            err = TRUE;
        }

        if (*inout_n_density != n_density) {
            E_ERROR
                ("# density comp/mix for file %s (== %u) != prior # density, %u\n",
                 fn, n_density, *inout_n_density);
            err = TRUE;
        }

        if (*inout_pass2var != pass2var) {
            E_ERROR("2 pass var %s in %s, but %s in others.\n",
                    fn, (pass2var ? "true" : "false"),
                    (*inout_pass2var ? "true" : "false"));
            err = TRUE;
        }

        for (i = 0; i < n_stream; i++) {
            if ((*inout_veclen)[i] != in_veclen[i]) {
                E_ERROR
                    ("vector length of stream %u (== %u) != prior length (== %u)\n",
                     i, in_veclen[i], (*inout_veclen)[i]);
                err = TRUE;
            }
        }

        ckd_free(in_veclen);

        if (err)
            return S3_ERROR;

        /* accumulate values */
        accum_3d(dnom, in_dnom, n_mgau, n_stream, n_density);

        gauden_accum_param(wt_mean, in_wt_mean,
                           n_mgau, n_stream, n_density, *inout_veclen);
        gauden_free_param(in_wt_mean);

        if (wt_var) {
            assert(in_wt_var);

            gauden_accum_param_full(wt_var, in_wt_var,
                                    n_mgau, n_stream, n_density,
                                    *inout_veclen);
            gauden_free_param_full(in_wt_var);
        }
    }

    return S3_SUCCESS;
}


int
rdacc_mmie_den(const char *dir,
               const char *lat_name,
               vector_t **** inout_wt_mean,
               vector_t **** inout_wt_var,
               float32 **** inout_dnom,
               uint32 * inout_n_mgau,
               uint32 * inout_n_stream,
               uint32 * inout_n_density, uint32 ** inout_veclen)
{
    char fn[MAXPATHLEN + 1];
    vector_t ***in_wt_mean;
    vector_t ***wt_mean;
    vector_t ***in_wt_var;
    vector_t ***wt_var;
    float32 ***in_dnom;
    float32 ***dnom;
    uint32 n_mgau;
    uint32 n_density;
    uint32 n_stream;
    uint32 *in_veclen;
    int32 pass2var;
    int i;

    sprintf(fn, "%s/%s_gauden_counts", dir, lat_name);

    if (s3gaucnt_read(fn,
                      &in_wt_mean,
                      &in_wt_var,
                      &pass2var,
                      &in_dnom,
                      &n_mgau,
                      &n_stream, &n_density, &in_veclen) != S3_SUCCESS) {
        fflush(stdout);
        perror(fn);

        return S3_ERROR;
    }

    wt_mean = *inout_wt_mean;
    wt_var = *inout_wt_var;
    dnom = *inout_dnom;

    if (wt_mean == NULL) {

        /* if a gauden_counts file exists, it will have reestimated means */

        *inout_wt_mean = wt_mean = in_wt_mean;
        *inout_dnom = dnom = in_dnom;
        *inout_n_mgau = n_mgau;
        *inout_n_stream = n_stream;
        *inout_n_density = n_density;
        *inout_veclen = in_veclen;

        if (wt_var == NULL && in_wt_var != NULL) {
            *inout_wt_var = wt_var = in_wt_var;
        }
    }
    else {
        int err = FALSE;

        /* check if matrices are able to be added */
        if (*inout_n_mgau != n_mgau) {
            E_ERROR
                ("# mix. Gau. for file %s (== %u) != prior # mix. Gau. (== %u)\n",
                 fn, n_mgau, *inout_n_mgau);
            err = TRUE;
        }

        if (*inout_n_stream != n_stream) {
            E_ERROR
                ("# stream for file %s (== %u) != prior # stream (== %u)\n",
                 fn, n_stream, *inout_n_stream);
            err = TRUE;
        }

        if (*inout_n_density != n_density) {
            E_ERROR
                ("# density comp/mix for file %s (== %u) != prior # density, %u\n",
                 fn, n_density, *inout_n_density);
            err = TRUE;
        }

        for (i = 0; i < n_stream; i++) {
            if ((*inout_veclen)[i] != in_veclen[i]) {
                E_ERROR
                    ("vector length of stream %u (== %u) != prior length (== %u)\n",
                     i, in_veclen[i], (*inout_veclen)[i]);
                err = TRUE;
            }
        }

        ckd_free((void *) in_veclen);

        if (err)
            return S3_ERROR;

        /* accumulate values */
        accum_3d(dnom, in_dnom, n_mgau, n_stream, n_density);

        gauden_accum_param(wt_mean, in_wt_mean,
                           n_mgau, n_stream, n_density, *inout_veclen);
        gauden_free_param(in_wt_mean);

        if (wt_var) {
            assert(in_wt_var);

            gauden_accum_param(wt_var, in_wt_var,
                               n_mgau, n_stream, n_density, *inout_veclen);
            gauden_free_param(in_wt_var);
        }
    }

    return S3_SUCCESS;
}
