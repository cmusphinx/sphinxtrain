/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * ms_mllr.c -- Application of MLLR regression matrices to codebook means
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
 * Revision 1.5  2006/02/22  17:21:35  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Added ms_mllr.[ch]
 * 
 * Revision 1.4.4.1  2005/07/20 19:39:01  arthchan2003
 * Added licences in ms_* series of code.
 *
 * Revision 1.4  2005/06/21 18:56:13  arthchan2003
 * 1, Fixed doxygen documentation. 2, Added $ keyword.
 *
 * Revision 1.4  2005/06/19 04:50:02  archan
 * Sphinx3 to s3.generic: allow multiple classes for MLLR
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
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

/* #include <s3.h>*/

#include "ms_mllr.h"


int32
ms_mllr_read_regmat(const char *regmatfile,
                    float32 ***** A,
                    float32 **** B,
                    float32 **** H,
                    int32 * streamlen, int32 n_stream, int32 * nclass)
{
    int32 i, j, k, m, n, lnclass;
    FILE *fp;
    float32 ****lA, ***lB, ***lH;

    if ((fp = fopen(regmatfile, "r")) == NULL) {
        E_ERROR("fopen(%s,r) failed\n", regmatfile);
        return -1;
    }
    else
        E_INFO("Reading MLLR transformation file %s\n", regmatfile);

    if ((fscanf(fp, "%d", &n) != 1) || (n < 1))
        return -1;
    lnclass = n;

    if ((fscanf(fp, "%d", &n) != 1) || (n != n_stream))
        return -1;

    lA = (float32 ****) ckd_calloc(n_stream, sizeof(float32 **));
    lB = (float32 ***) ckd_calloc(n_stream, sizeof(float32 *));
    lH = (float32 ***) ckd_calloc(n_stream, sizeof(float32 *));

    for (i = 0; i < n_stream; ++i) {
        lA[i] =
            (float32 ***) ckd_calloc_3d(lnclass, streamlen[i],
                                        streamlen[i], sizeof(float32));
        lB[i] =
            (float32 **) ckd_calloc_2d(lnclass, streamlen[i],
                                       sizeof(float32));
        lH[i] =
            (float32 **) ckd_calloc_2d(lnclass, streamlen[i],
                                       sizeof(float32));
    }

    for (i = 0; i < n_stream; ++i) {
        if ((fscanf(fp, "%d", &n) != 1) || (streamlen[i] != n))
            goto readerror;

        for (m = 0; m < lnclass; ++m) {
            for (j = 0; j < streamlen[i]; ++j) {
                for (k = 0; k < streamlen[i]; ++k) {
                    if (fscanf(fp, "%f ", &lA[i][m][j][k]) != 1)
                        goto readerror;
                }
            }
            for (j = 0; j < streamlen[i]; ++j) {
                if (fscanf(fp, "%f ", &lB[i][m][j]) != 1)
                    goto readerror;
            }
            for (j = 0; j < streamlen[i]; ++j) {
                if (fscanf(fp, "%f ", &lH[i][m][j]) != 1)
                    goto readerror;
            }
        }
    }

    *A = lA;
    *B = lB;
    *H = lH;
    *nclass = lnclass;

    fclose(fp);

    return 0;

  readerror:
    E_ERROR("Error reading MLLR file %s\n", regmatfile);
    for (i = 0; i < n_stream; ++i) {
        ckd_free_3d((void ***) lA[i]);
        ckd_free_2d((void **) lB[i]);
        ckd_free_2d((void **) lH[i]);
    }
    ckd_free(lA);
    ckd_free(lB);
    ckd_free(lH);
    fclose(fp);

    *A = NULL;
    *B = NULL;
    *H = NULL;

    return -1;
}


int32
ms_mllr_free_regmat(float32 **** A, float32 *** B, float32 *** H, int32 n_stream)
{
    int32 i;

    for (i = 0; i < n_stream; i++) {
        ckd_free_3d((void ***) A[i]);
        ckd_free_2d((void **) B[i]);
        ckd_free_2d((void **) H[i]);
    }

    ckd_free(A);
    ckd_free(B);
    ckd_free(H);

    return 0;
}


int32
ms_mllr_norm_mgau(float32 *** mean,
		  float32 *** var,
                  int32 n_density,
                  float32 **** A,
                  float32 *** B,
                  float32 *** H,
                  int32 * streamlen, int32 n_stream, int32 class)
{
    int32 s, d, l, m;
    float64 *temp;

    /* Transform codebook for each stream s */
    for (s = 0; s < n_stream; s++) {
        temp = (float64 *) ckd_calloc(streamlen[s], sizeof(float64));

        /* Transform each density d in selected codebook */
        for (d = 0; d < n_density; d++) {
            for (l = 0; l < streamlen[s]; l++) {
                temp[l] = 0.0;
                for (m = 0; m < streamlen[s]; m++) {
                    temp[l] += A[s][class][l][m] * mean[s][d][m];
                }
                temp[l] += B[s][class][l];
            }

            for (l = 0; l < streamlen[s]; l++) {
                mean[s][d][l] = (float32) temp[l];
		if (H)
			var[s][d][l] *= H[s][class][l];
            }
        }

        ckd_free(temp);
    }

    return 0;
}
