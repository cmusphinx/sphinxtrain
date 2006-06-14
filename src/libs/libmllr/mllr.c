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
#include <string.h>
#include <math.h>
#include <s3/s3.h>
#include <s3/common.h>
#include <s3/s3regmat_io.h>
#include <s3/mllr.h>
#include <s3/clapack_lite.h>

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


void dump_regmat_statistics(float64 *****regl,
		       float64 ****regr,
		       uint32 nclass,
		       uint32 nfeat,
		       const uint32  *veclen)

{
  uint32  i,j,k,l,m,len;

  printf("regl\n");
  for(m=0; m <nclass ; m++){
    for(i=0; i < nfeat ;i++){
      len=veclen[i];
      for(l=0; l < len ; l++){
	printf("class: %d, stream: %d, mixture %d\n",m,i,l);
	for(j=0;j<len;j++){
	  printf("%d ", j);
	  for(k=0;k<len;k++){
	    printf("%f ",regl[m][i][l][j][k]);
	  }
	    printf("\n");
	}
	}
    }
  }
  
  printf("regr\n");
  for(m=0; m <nclass ; m++){
    for(i=0; i < nfeat ;i++){
      len=veclen[i];
      for(l=0; l < len ; l++){
	printf("class: %d, stream: %d, mixture %d\n",m,i,l);
	for(j=0;j<len;j++){
	  printf("%f ",regr[m][i][l][j]);
	}
	printf("\n");
      }
    }
  }
    
}

/* 
20040726 : ARCHAN : What is this routine doing?  This routine can do
the conventional Legetter's method of maximum likelihood linear
regression.  For every regression class, every stream (nfeat is the
number of stream).  
*/


int32
compute_mllr (
             float64 *****regl,
             float64 ****regr,
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

#if 0
    dump_regmat_statistics(regl,
			   regr,
			   nclass,
			   nfeat
			   veclen);

#endif

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
		    if (solve(regl[m][i][j],regr[m][i][j],ABloc,len) != S3_SUCCESS){
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
		    if (solve(regl[m][i][j],regr[m][i][j],ABloc,len+1) != S3_SUCCESS) {
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

/* Solve x for equations Ax=b */
int32
solve  (float64 **A, /*Input : an n*n matrix A */
        float64 *b,  /*Input : a n dimesion vector b */
	float64 *x,  /*Output : a n dimesion vector x */
	int32   n)

{
  float64 *tmp_l;
  float64 *tmp_r;
  int i, j;
  int32 N, NRHS, LDA, LDB, INFO;
  int32 *IPIV;

  N=n;
  NRHS=1;    
  LDA=n;    
  LDB=n;

    /* don't know whether we HAVE to do this to get the f2c routine
       running. */

    tmp_l = (float64 *)ckd_calloc(N * N, sizeof(float64));

    /*To use the f2c lapack function, row/column ordering of the
      arrays need to be changed. */

    for (i = 0; i < N; i++) 
      for (j = 0; j < N; j++) 
	tmp_l[j+N*i] = A[i][j]; 

    tmp_r = (float64*) ckd_calloc(N, sizeof(float64));

    for (i = 0; i < N; i++) 
      tmp_r[i] = b[i];

    IPIV = (int32 *)ckd_calloc(N, sizeof(int32));

    /* Beware ! all arguments of lapack have to be a pointer */

    dgesv_(&N, &NRHS, tmp_l,&LDA,IPIV,tmp_r, &LDB, &INFO);

    if( INFO==0 ){ /*fprintf( stderr, "OK\n" );*/ }
    else{ return S3_ERROR; }

    for(i= 0 ; i< n ; i++){
      x[i] = tmp_r[i]; 
    }
    
    ckd_free ((void *)tmp_l);
    ckd_free ((void *)tmp_r);
    ckd_free ((void *)IPIV);


    return S3_SUCCESS;
}

/* Find inverse by solving AX=I. */
int32
invert(float32 **ainv,
       float32 **a,
       int32 n)
{
    float64 *tmp_a;
    float64 *tmp_i;
    int i, j;
    int32 N, NRHS, LDA, LDB, INFO;
    int32 *IPIV;

    N=n;
    NRHS=n;
    LDA=n;    
    LDB=n;

    /*To use the f2c lapack function, row/column ordering of the
      arrays need to be changed. */
    tmp_a = (float64 *)ckd_calloc(N * N, sizeof(float64));
    for (i = 0; i < N; i++) 
	for (j = 0; j < N; j++) 
	    tmp_a[j+N*i] = a[i][j]; 

    /* Construct an identity matrix. */
    tmp_i = (float64*) ckd_calloc(N * N, sizeof(float64));
    for (i = 0; i < N; i++) 
	tmp_i[i+N*i] = 1.0;

    IPIV = (int32 *)ckd_calloc(N, sizeof(int32));

    /* Beware ! all arguments of lapack have to be a pointer */
    dgesv_(&N, &NRHS, tmp_a, &LDA, IPIV, tmp_i, &LDB, &INFO);

    if (INFO != 0)
	return S3_ERROR;

    for (i = 0; i < n; ++i)
	for (j = 0; j < n; ++j)
	    ainv[i][j] = tmp_i[j+N*i];
    
    ckd_free ((void *)tmp_a);
    ckd_free ((void *)tmp_i);
    ckd_free ((void *)IPIV);

    return S3_SUCCESS;
}
