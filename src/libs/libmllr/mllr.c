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
#include <s3/s3regmat_io.h>
#include <s3/mllr.h>

int32
regmat_read (const char    *accumdir,
	     float32 ******regl,
	     float32 *****regr,
             const uint32  **veclen,
	     uint32  *n_mllr_class,
	     uint32  *n_stream,
	     uint32  *mllr_mult,
	     uint32  *mllr_add)
{
    char    accbuffer[1024];
    uint32  mllrmult,mllradd,nclass,nfeat;
    const uint32  *len;
    float32 ****lregr=NULL, *****lregl=NULL;
    float32 ****lregrloc=NULL, *****lreglloc=NULL;

    uint32  i,j,k,l,m;

    static int32 FIRSTBUFF = 1;
    static uint32 oldmllrmult, oldmllradd, oldnfeat;
    static const uint32 *in_veclen;

    E_INFO("Reading MLLR accumulator from %s..",accumdir);
    sprintf(accbuffer,"%s/regmat_counts",accumdir);

    if (s3regmatcnt_read (accbuffer, &lregr, &lregl, &nclass, &nfeat, &len, 
			  &mllrmult, &mllradd) != S3_SUCCESS) {
        E_FATAL("Error reading MLLR regression matrix buffer\n");
    }

    if (FIRSTBUFF) {
        *regl = lregl;
        *regr = lregr;
        *veclen = len;
        *mllr_mult = mllrmult;
        *mllr_add = mllradd;
        *n_mllr_class = nclass;
        *n_stream = nfeat;

        oldmllrmult = mllrmult;
        oldmllradd = mllradd;
        oldnfeat   = nfeat;

        FIRSTBUFF = 0;

        return S3_SUCCESS;
    }

    if ((oldmllrmult != mllrmult) || (oldmllradd != mllradd)) {
	E_FATAL("Mismatch in mllrmult/mllradd flags in buffer %s\n",accumdir);
    }
    if (oldnfeat != nfeat) {
	E_FATAL("Mismatch in number of features in mllr buffer %s\n",accumdir);
    }

    in_veclen = *veclen;

    lregrloc = *regr;
    lreglloc = *regl;
    for (m = 0; m < nclass; m++) {
	for (i = 0; i < nfeat; ++i) {
	    if (in_veclen[i] != len[i]) {
		E_FATAL("Mismatch in %d featlen in mllr buffer %s\n",i,accumdir);
	    }
	    for (j = 0; j < len[i]; j++) {
		for (k = 0; k <= len[i]; k++) {
		    lregrloc[m][i][j][k] += lregr[m][i][j][k];
		    for (l = 0; l <= len[i]; l++) {
			lreglloc[m][i][j][k][l] += lregl[m][i][j][k][l];
		    }
		}
	    }
	}
    }

    ckd_free ((void *)len);

    for (m = 0; m < nclass; m++) {
	for (i = 0; i < nfeat; ++i) {
	    ckd_free_2d ((void **)lregr[m][i]);
	    ckd_free_3d ((void ***)lregl[m][i]);
	}
    }
    ckd_free_2d ((void **)lregr);
    ckd_free_2d ((void **)lregl);

    return S3_SUCCESS;
}

int32
compute_mllr (
             float32 *****regl,
             float32 ****regr,
             const uint32  *veclen,
             uint32  nclass,
             uint32  nfeat,
             uint32  mllrmult,
             uint32  mllradd,
             float32 *****A,
             float32 ****B
            )
{
    uint32  i,j,k,m,len;
    float32 ****Aloc, ***Bloc;
    float64 *ABloc=NULL;

    Aloc = (float32 ****) ckd_calloc_2d(nclass, nfeat, sizeof(float32 ***));
    Bloc = (float32 ***) ckd_calloc_2d(nclass, nfeat, sizeof(float32 **));

    for (m = 0; m < nclass; m++) {
	for (i = 0; i < nfeat; i++) {
	    len = veclen[i];

	    Aloc[m][i] = (float32 **) ckd_calloc_2d(len, len, sizeof(float32));
	    Bloc[m][i] = (float32 *) ckd_calloc(len, sizeof(float32));
	    ABloc = (float64 *) ckd_calloc(len+1, sizeof(float64));
 
	    if (mllrmult && !mllradd) {
		/* Compute only multiplicative part of MLLR*/
		E_INFO("Computing only multiplicative part of MLLR\n"); 
		for (j = 0; j < len; j++) {
		    Bloc[m][i][j] = 0.0;
		}
		for (j = 0; j < len; ++j) {
		    /** If estimation of rotation not possible, dont rotate **/
		    if (solve(regl[m][i][j],regr[m][i][j],len,ABloc) != S3_SUCCESS){
			E_INFO("Estimation of %d th multiplicative",
			       " term in MLLR regression failed\n",j);
			for (k = 0;k < len; k++) {
			    Aloc[m][i][j][k] = 0.0;
			}
			Aloc[m][i][j][j] = 1.0;
		    }
		    else {
			for (k = 0; k < len; k++) {
			    Aloc[m][i][j][k] = ABloc[k];
			}
		    }
		}
	    }
	    else if(!mllrmult && mllradd){
		/* Compute only additive part of MLLR*/
		E_INFO("Computing only additive part of MLLR\n"); 
		for (j = 0; j < len; j++) {
		    for (k = 0; k < len; k++) {
			Aloc[m][i][j][k] = 0.0;
		    }
		    Aloc[m][i][j][j] = 1.0;
		}
		for (j = 0; j < len; j++) {
		    /** If estimation of shift not possible, dont shift **/
		    if (regl[m][i][j][len][len] > MIN_IEEE_NORM_POS_FLOAT32) {
			Bloc[m][i][j] = (regr[m][i][j][len]-regl[m][i][j][len][j])/
			    regl[m][i][j][len][len];
		    }
		    else {
			E_INFO("%th dimension of addition term of %dth MLLR adaptation underflow!\nSet to zero...\n",j);
			Bloc[m][i][j] = 0.0;
		    }
		}
	    }
	    else if (mllrmult && mllradd) {
		/* Compute both multiplicative and additive part of MLLR*/
		E_INFO("Computing both multiplicative and additive part of MLLR\n");
		for (j = 0; j < len; ++j) {
		    /** If estimation of regression not possible, dont do anything **/
		    if (solve(regl[m][i][j],regr[m][i][j],len+1,ABloc) != S3_SUCCESS) {
			E_INFO("Estimation of %d th regression in MLLR failed\n",j);
			for (k = 0;k < len; k++) {
			    Aloc[m][i][j][k] = 0.0;
			}
			Aloc[m][i][j][j] = 1.0;
			Bloc[m][i][j] = 0.0;
		    }
		    else {
			for (k = 0; k < len; ++k) {
			    Aloc[m][i][j][k] = ABloc[k];
			}
			Bloc[m][i][j] = ABloc[len];
		    }
		}
	    }
	    else if(!mllrmult && !mllradd){
		/*MLLR flags are not set - Error*/
		E_FATAL("ERROR: MLLR flags are not set\n"); 
	    }
	    ckd_free(ABloc);
	}
    }
    *A = Aloc;
    *B = Bloc;

    return S3_SUCCESS;
}



int32
invert(float32 **ainv,
       float32 **a,
       int32 len)
{
    int32 i, j;
    int32 *indx;
    float64 d;
    float64 *col;
    float64 **adcmp;

    indx = ckd_calloc(len, sizeof(int32));
    col = ckd_calloc(len, sizeof(float64));
    adcmp = (float64 **)ckd_calloc_2d(len, len, sizeof(float64));

    for (i = 0; i < len; i++) {
	for (j = 0; j < len; j++) {
	    adcmp[i][j] = a[i][j];
	}
    }

    ludcmp(adcmp, len, indx, &d);
    for (j = 0; j < len; j++) {
	for (i = 0; i < len; i++)
	    col[i] = 0;
	col[j] = 1;

	lubksb(adcmp, len, indx, col, col);
	for (i = 0; i < len; i++) {
	    ainv[i][j] = col[i];
	}
    }

    ckd_free(indx);
    ckd_free(col);
    ckd_free_2d((void **)adcmp);

    return S3_SUCCESS;
}

int32
solve  (float32 **regl,
        float32 *regr,
	int32   len,
	float64 *A)
{
    float64 d;
    float64 **tmp_regl;
    float64 *tmp_regr;
    int32 *indx;
    int i, j;

    tmp_regl = (float64 **)ckd_calloc_2d(len, len, sizeof(float64));
    for (i = 0; i < len; i++) {
	for (j = 0; j < len; j++) {
	    tmp_regl[i][j] = regl[i][j];
	}
    }
    indx = (int32 *) ckd_calloc(len, sizeof(int32));

    if (ludcmp(tmp_regl, len, indx, &d) != S3_SUCCESS) {
	ckd_free (indx);
	ckd_free_2d ((void **)tmp_regl);

	return S3_ERROR;
    }

    tmp_regr = ckd_calloc(len, sizeof(float64));
    for (i = 0; i < len; i++)
	tmp_regr[i] = regr[i];

    lubksb(tmp_regl, len, indx, tmp_regr, A);

    ckd_free (indx);
    ckd_free_2d ((void **)tmp_regl);
    ckd_free ((void *)tmp_regr);

    return S3_SUCCESS;
}


int32 ludcmp(float64 **a,
             int32   n,
             int32   *indx,
             float64 *d)
{
    int32    i,imax=0,j,k;
    float64  big,dum,sum,t1;
    float64  *vv;  /* vv stores the implicit scaling of each row */

    if((vv=(float64 *) ckd_calloc(n,sizeof(float64))) == NULL)
	E_INFO("Unable to allocate space for vv\n");

    *d=1.0;           /* No row interchanges yet */
    for (i = 0; i < n; i++) { /*Loop over the rows to get the implicit scaling */
	big = 0.0;    /*information */
	for (j = 0; j < n; j++) {
	    if((t1 = fabs(a[i][j])) > big)
		big=t1;
	}
	if (big == 0.0) {
	    /* No nonzero largest element. */
	    E_ERROR("Singular matrix in routine ludcmp\n");

	    return S3_ERROR;
	}
	vv[i] = 1.0/big; /* Save the scaling */
    }
    for (j = 0; j < n; j++) {
	for (i = 0; i < j; i++) {
	    sum = a[i][j];
	    for (k = 0; k < i; k++)
		sum -= a[i][k]*a[k][j];
	    a[i][j] = sum;
	}
	big = 0.0;
	for (i = j; i < n;i++) {
	    sum = a[i][j];
	    for (k = 0; k < j; k++)
		sum -= a[i][k] * a[k][j];
	    a[i][j] = sum;
	    if ( (dum = vv[i] * fabs(sum)) >= big) {
		big = dum;
		imax = i;
	    }
	}
	if (j != imax) {
	    for (k = 0; k < n; k++) {
		dum = a[imax][k];
		a[imax][k] = a[j][k];
		a[j][k] = dum;
	    }
	    *d = -(*d);
	    vv[imax] = vv[j];
	}
	indx[j] = imax;
	if (a[j][j] == 0.0)
	    a[j][j]=TINY;

	if (j != n-1) {
	    dum = 1.0 / a[j][j];
	    for (i = j+1; i < n; i++)
		a[i][j] *= dum;
	}
    }
    
    ckd_free(vv);

    return S3_SUCCESS;
}


int32 lubksb(
       float64 **a, 
       int32   n, 
       int32   *indx, 
       float64 b[],
       float64 x[]
      )
{
    int32   i,ii=0,ip,j,done=0;
    float64 sum;

    for (i = 0; i < n; i++) {
	ip = indx[i];
	sum = b[ip];
	b[ip] = b[i];
	if (done) {
	    for (j = ii; j < i; j++)
		sum -= a[i][j] * b[j];
	}
	else if (sum) {
	    ii=i;
	    done=1;
	}
	b[i]=sum;
    }
    for (i=0;i<n;i++) {
	x[i] = b[i];
    }
    
    for (i = n-1; i >= 0; i--) {
	sum = x[i];
	
	for (j = n-1; j > i; j--)
	    sum -= a[i][j]*x[j];
	
	x[i]=sum/a[i][i];
    }

    return S3_SUCCESS;
}
