/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * gs.c -- gaussian density module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * $Log$
 * Revision 1.3  2005/06/21  18:28:05  arthchan2003
 * Log. 1, Fixed doxygen documentation. 2, Add $Log$ keyword.
 * 
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 29-Dec-03    A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University.
 *              Initial version created           
 */

#include <stdio.h>
#include <sphinxbase/bitvec.h>

#include "gs.h"

int gs_free(gs_t * gs);

int32
gs_fread_int32(gs_t * gs)
{
    int32 val;
    if (fread(&val, sizeof(int32), 1, gs->fp) != 1)
        E_FATAL("fread failed\n");
    return (val);
}

float32
gs_fread_float32(gs_t * gs)
{
    float32 val;
    if (fread(&val, sizeof(float32), 1, gs->fp) != 1)
        E_FATAL("fread failed\n");
    return (val);
}

void
gs_fread_bitvec_t(bitvec_t * val, gs_t * gs)
{
    if (fread(val, gs->n_mbyte, 1, gs->fp) != 1)
        E_FATAL("fread failed\n");
}

int
gs_display(char *file, gs_t * gs)
{
    int32 i;
    int32 code_id;
    int32 m_id, s_id, c_id;
    float32 tmp;
    bitvec_t *bv;

    E_INFO("Reading gaussian selector map: %s\n", file);
    gs = (gs_t *) ckd_calloc(1, sizeof(gs_t));

    if ((gs->fp = fopen(file, "rb")) == NULL)
        E_FATAL("fopen(%s,rb) failed\n", file);

    gs->n_mgau = gs_fread_int32(gs);

    E_INFO("The number of mixtures of gaussian: %d\n", gs->n_mgau);
    gs->n_feat = gs_fread_int32(gs);
    E_INFO("The number of features stream: %d\n", gs->n_feat);
    gs->n_density = gs_fread_int32(gs);
    E_INFO("The number of density: %d\n", gs->n_density);
    gs->n_code = gs_fread_int32(gs);
    E_INFO("The number of code word: %d\n", gs->n_code);
    gs->n_featlen = gs_fread_int32(gs);
    E_INFO("The feature length: %d\n", gs->n_featlen);
    gs->n_mbyte = bitvec_size(gs->n_density) * sizeof(bitvec_t);
    E_INFO("The number of byte to read: %d\n", gs->n_mbyte);

    /* allocate the bit vector here */
    bv = bitvec_alloc(gs->n_density);

    /*  for(i=0;i<gs->n_code;i++) */
    for (code_id = 0; code_id < gs->n_code; code_id++) {
        printf("Code idx: %d\n", code_id);
        for (c_id = 0; c_id < gs->n_featlen; c_id++) {
            tmp = gs_fread_float32(gs);
            printf("%f ", tmp);
        }
        printf("\n");
        for (m_id = 0; m_id < gs->n_mgau; m_id++) {
            for (s_id = 0; s_id < gs->n_feat; s_id++) {
                /*The writer currently doesn't support the byte order */
                gs_fread_bitvec_t(bv, gs);
                printf("%d %d ", m_id, s_id);

                for (i = 0; i < gs->n_density; i++) {
                    if (bitvec_is_set(bv, i)) {
                        printf("%d ", i);
                    }
                }
                printf("\n");
            }
        }
    }

    printf("\n");
    /*  bitvec_free(bv); */
    /* destroy the bit vector here */

    gs_free(gs);
    return 1;
}

gs_t *
gs_read(const char *file, logmath_t *logmath)
{

    int32 code_id;
    int32 m_id, s_id, c_id;
    bitvec_t *bv;
    gs_t *gs;

    E_INFO("Reading gaussian selector map: %s\n", file);
    gs = (gs_t *) ckd_calloc(1, sizeof(gs_t));
    if (gs == NULL)
        E_FATAL("Cannot allocate gs\n");

    if ((gs->fp = fopen(file, "rb")) == NULL)
        E_FATAL("gs_read(%s,rb) failed\n", file);

    gs->logmath = logmath;
    gs->n_mgau = gs_fread_int32(gs);

    E_INFO("The number of mixtures of gaussian: %d\n", gs->n_mgau);
    gs->n_feat = gs_fread_int32(gs);
    E_INFO("The number of features stream: %d\n", gs->n_feat);
    gs->n_density = gs_fread_int32(gs);
    E_INFO("The number of density: %d\n", gs->n_density);
    gs->n_code = gs_fread_int32(gs);
    E_INFO("The number of code word: %d\n", gs->n_code);
    gs->n_featlen = gs_fread_int32(gs);
    E_INFO("The feature length: %d\n", gs->n_featlen);

    gs->n_mbyte = bitvec_size(gs->n_density) * sizeof(bitvec_t);
    E_INFO("The number of byte to read: %d\n", gs->n_mbyte);

    /* allocate the bit vector here */
    bv = bitvec_alloc(gs->n_density);


    /* allocate memory for the data structure */
    /* n_code * n_featlen */
    gs->codeword =
        (float32 **) ckd_calloc_2d(gs->n_code, gs->n_featlen,
                                   sizeof(float32));
    /* n_mgau * n_feat * n_code */
    /*Hack ! assume feature stream to be only 1 */
    gs->codemap =
        (uint32 ***) ckd_calloc_3d(gs->n_mgau, gs->n_feat, gs->n_code,
                                   sizeof(uint32));
    gs->mgau_sl = (int32 *) ckd_calloc(gs->n_density + 1, sizeof(int32));

    for (code_id = 0; code_id < gs->n_code; code_id++) {
        for (c_id = 0; c_id < gs->n_featlen; c_id++) {
            gs->codeword[code_id][c_id] = gs_fread_float32(gs);
        }
        for (m_id = 0; m_id < gs->n_mgau; m_id++) {
            for (s_id = 0; s_id < gs->n_feat; s_id++) {
                /*The writer currently doesn't support the byte order */
                gs_fread_bitvec_t(bv, gs);
                gs->codemap[m_id][s_id][code_id] = *bv;
            }
        }
    }
    return gs;
}


int32
gc_compute_closest_cw(gs_t * gs, float32 * feat)
{
    int32 codeid, bst_codeid;
    float64 diff1, diff2, tmp1, tmp2, min;
    int32 cid;
    float64 min_density;
    /*E_INFO("Compute the closest Code word\n"); */
    min_density = logmath_log_to_ln(gs->logmath, S3_LOGPROB_ZERO);

    bst_codeid = 0;
    min = MAX_POS_FLOAT64;

    for (codeid = 0; codeid < gs->n_code; codeid += 2) {
        tmp1 = 0;
        tmp2 = 0;

        for (cid = 0; cid < gs->n_featlen; cid++) {
            diff1 = feat[cid] - (gs->codeword[codeid][cid]);
            tmp1 += diff1 * diff1;

            diff2 = feat[cid] - (gs->codeword[codeid + 1][cid]);
            tmp2 += diff2 * diff2;
        }

        if (tmp1 < min) {
            min = tmp1;
            bst_codeid = codeid;
        }

        if (tmp2 < min) {
            min = tmp2;
            bst_codeid = codeid + 1;
        }
    }
    return bst_codeid;
}


/* Return the number of gaussians computed */
int32
gs_mgau_shortlist(gs_t * gs, int m, int n, float32 * feat, int bst_codeid)
{
    uint32 map;
    int32 bit_id;
    int32 nc;
    nc = 0;

    map = 0;
    map = gs->codemap[m][0][bst_codeid];

#if 0
    E_INFO("The map is %u, the length of gaussian %d\n", map, n);
#endif

    for (bit_id = 0; bit_id < n; bit_id++) {
        if (map & (1 << bit_id))
            gs->mgau_sl[nc++] = bit_id;
    }
    gs->mgau_sl[nc] = -1;

    if (nc == 0) {
        /* 20040222 ARCHAN : added to safe guard Gaussian Selection anomality */
        /* Special case when the number of components is smaller than the max. */
        /* This happens when zero vectors were removed from the codebook. */

        for (bit_id = 0; bit_id < n; bit_id++) {
            gs->mgau_sl[nc++] = bit_id;
        }
        gs->mgau_sl[nc] = -1;
    }

    if (nc == 0) {
        E_INFO
            ("No active gaussian found in senone %d, with num. component = %d\n",
             m, n);
    }
    return nc;

}

int32
gs_delete(char *file)
{
    return 1;
}

int
gs_free(gs_t * gs)
{
    fclose(gs->fp);
    free(gs);
    return 1;
}
