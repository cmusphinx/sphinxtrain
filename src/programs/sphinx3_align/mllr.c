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
/*
 * mllr.c -- Application of MLLR regression matrices to codebook means
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
 * Revision 1.4  2005/06/21  18:50:45  arthchan2003
 * Log. 1, Fixed doxygen documentation, 2, Added $Log$ keyword.
 * 
 * Revision 1.4  2005/06/19 19:41:21  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 24-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon University
 *              First checked in from sphinx 3.0 to sphinx 3.5
 * 
 * 02-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added reading of MLLR classes in transformation file.  Currently must
 * 		be 1.
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started (copied from Vipul Parikh's implementation).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mllr.h"

void
mllr_dump(float32 *** A, float32 ** B, float32 **H, 
	  int32 veclen, int32 nclass)
{
    int32 i, j, k;
    char *tmpstr;
    assert(A != NULL);
    assert(B != NULL);

    tmpstr = ckd_calloc((veclen * (veclen + 2) * 20), sizeof(char));

    for (i = 0; i < nclass; i++) {
        E_INFO("%d:\n", i);
        for (j = 0; j < veclen; j++) {
            sprintf(tmpstr, "A %d ", j);
            for (k = 0; k < veclen; k++) {
                sprintf(tmpstr, "%s %f ", tmpstr, A[i][j][k]);
            }
            sprintf(tmpstr, "%s\n", tmpstr);
            E_INFO("%s\n", tmpstr);
        }

        sprintf(tmpstr, "B\n");
        for (j = 0; j < veclen; j++) {
            sprintf(tmpstr, "%s %f ", tmpstr, B[i][j]);
        }
        sprintf(tmpstr, "%s \n", tmpstr);

        sprintf(tmpstr, "H\n");
        for (j = 0; j < veclen; j++) {
            sprintf(tmpstr, "%s %f ", tmpstr, H[i][j]);
        }
        sprintf(tmpstr, "%s \n", tmpstr);

        E_INFO("%s\n", tmpstr);
    }

    ckd_free(tmpstr);
}

int32
mllr_read_regmat(const char *regmatfile,
                 float32 **** A,
                 float32 *** B,
		 float32 *** H,
		 int32 * nclass, int32 ceplen)
{
    int32 i, j, k, n, lnclass;
    FILE *fp;
    float32 ***lA, **lB, **lH;

    if ((fp = fopen(regmatfile, "r")) == NULL) {
        E_ERROR("fopen(%s,r) failed\n", regmatfile);
        return -1;
    }
    else
        E_INFO("Reading MLLR transformation file %s\n", regmatfile);

    lA = NULL;
    lB = NULL;
    lH = NULL;

    if ((fscanf(fp, "%d", &n) != 1) || (n < 1))
        goto readerror;
    lnclass = n;

    /* The number of stream must be 1 for now (archan@cs.cmu.edu 24-Jul-2004) */
    if ((fscanf(fp, "%d", &n) != 1) || (n != 1))
        goto readerror;

    lA = (float32 ***) ckd_calloc_3d(lnclass, ceplen, ceplen,
                                     sizeof(float32));
    lB = (float32 **) ckd_calloc_2d(lnclass, ceplen, sizeof(float32));
    lH = (float32 **) ckd_calloc_2d(lnclass, ceplen, sizeof(float32));

    for (i = 0; i < lnclass; i++) {
        /* We definitely do not allow different classes to have different
           feature vector lengths! (that would be silly) */
        if ((fscanf(fp, "%d", &n) != 1) || (ceplen != n))
            goto readerror;
        for (j = 0; j < ceplen; j++) {
            for (k = 0; k < ceplen; ++k) {
                if (fscanf(fp, "%f ", &lA[i][j][k]) != 1)
                    goto readerror;
            }
        }
        for (j = 0; j < ceplen; j++) {
            if (fscanf(fp, "%f ", &lB[i][j]) != 1)
                goto readerror;
        }
        for (j = 0; j < ceplen; j++) {
            if (fscanf(fp, "%f ", &lH[i][j]) != 1)
                goto readerror;
        }
    }

    *A = lA;
    *B = lB;
    *H = lH;
    if (nclass)
        *nclass = lnclass;

    fclose(fp);

    return 0;

  readerror:
    E_ERROR("Error reading MLLR file %s\n", regmatfile);
    ckd_free_3d((void ***) lA);
    ckd_free_2d((void **) lB);
    ckd_free_2d((void **) lH);

    fclose(fp);

    *A = NULL;
    *B = NULL;
    *H = NULL;

    return -1;
}


int32
mllr_free_regmat(float32 *** A, float32 ** B, float32 ** H)
{
    ckd_free_3d((void ***) A);
    ckd_free_2d((void **) B);
    ckd_free_2d((void **) H);
    return 0;
}



int32
mllr_norm_mgau(mgau_model_t * mgauset,
               float32 *** A, float32 ** B,
	       float32 ** H, int32 nclass, int32 * cb2mllr)
{
    int32 d, c, l, m;
    int32 class;
    float32 *temp;

    int32 n_density = mgauset->n_mgau;
    int32 n_mix = mgauset->max_comp;
    int32 ceplen = mgauset->veclen;
    mgau_t *mgau;

    /* Transform codebook for each stream s */
    temp = (float32 *) ckd_calloc(ceplen, sizeof(float32));

    /* Transform each density d in selected codebook */
    mgau = mgauset->mgau;

    for (d = 0; d < n_density; d++) {
        if (cb2mllr)
            class = cb2mllr[d];
        else
            class = 0;
        if (class == -1)
            continue;

        for (c = 0; c < n_mix; c++) {
            for (l = 0; l < ceplen; l++) {
                temp[l] = 0.0;
                for (m = 0; m < ceplen; m++) {
                    temp[l] += A[class][l][m] * mgau[d].mean[c][m];
                }
                temp[l] += B[class][l];
            }

            for (l = 0; l < ceplen; l++) {
                mgau[d].mean[c][l] = temp[l];
		mgau[d].var[c][l] *= H[class][l];
            }
        }
    }

    ckd_free(temp);

    return 0;
}
