/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
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
#include <string.h>
#include <math.h>
#include <s3/s3.h>
#include <s3/common.h>
#include <s3/mllr.h>
#include <s3/mllr_io.h>

int32
store_reg_mat (const char    *regmatfn,
	       const uint32  *veclen,
	       uint32  n_class,
	       uint32  n_stream,
	       float32 ****A,
	       float32 ***B)
{
    uint32 i,j,k,m;
    FILE  *fp;

    if ((fp = fopen(regmatfn,"w")) == NULL) {
	E_INFO("Unable to open %s to store MLLR matrices\n",regmatfn);
	return S3_ERROR;
    }

    fprintf(fp,"%d\n",n_class);
    fprintf(fp,"%d\n",n_stream); 
    for (m = 0; m < n_class; m++) {
	for (i = 0; i < n_stream; i++) {
	    fprintf(fp,"%d\n", veclen[i]); 
	    for (j = 0; j < veclen[i]; j++) {
		for (k = 0; k < veclen[i]; ++k) {
		    fprintf(fp,"%f ",A[m][i][j][k]);
		}
		fprintf(fp,"\n");
	    }
	    for (j = 0; j < veclen[i]; j++) {
		fprintf(fp,"%f ",B[m][i][j]);
	    }
	    fprintf(fp,"\n");
	}
    }
    fclose(fp);
    return S3_SUCCESS;
}


int32
read_reg_mat (
             const char   *regmatfn,
             const uint32  **veclen,
             uint32  *n_class,
             uint32  *n_stream,
             float32 *****A,
             float32 ****B
            )
{
    uint32 i,j,k,m,nstream,nclass;
    FILE  *fp;
    uint32 *vlen;
    float32 ****lA,***lB;

    if ((fp = fopen(regmatfn,"r")) == NULL) {
	E_INFO("Unable to open %s to read MLLR matrices\n",regmatfn);
	return S3_ERROR;
    }

    fscanf(fp,"%d",&nclass);
    fscanf(fp,"%d",&nstream);
    vlen = (uint32 *)ckd_calloc(nstream,sizeof(uint32));
    lA = (float32 ****)ckd_calloc_2d (nclass,nstream,sizeof (float32 **));
    lB = (float32 ***)ckd_calloc_2d (nclass,nstream,sizeof (float32 *));
    for (m = 0; m < nclass; ++m) {
	for (i = 0; i < nstream; ++i) {
	    fscanf(fp,"%d", &vlen[i]);
	    lA[m][i] = (float32 **) ckd_calloc_2d(vlen[i],vlen[i],sizeof(float32));
	    lB[m][i] = (float32 *) ckd_calloc(vlen[i],sizeof(float32));
	    for (j = 0; j < vlen[i]; j++) {
		for (k = 0; k < vlen[i]; ++k) {
		    fscanf(fp,"%f ",&lA[m][i][j][k]);
		}
	    }
	    for (j = 0; j < vlen[i]; j++) {
		fscanf(fp,"%f ",&lB[m][i][j]);
	    }
	}
    }

    *n_class = nclass;
    *n_stream = nstream;
    *veclen = vlen;
    *A = lA;
    *B = lB;

    fclose(fp);
    return S3_SUCCESS;
}
