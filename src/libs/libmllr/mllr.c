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
#include <s3/mllr.h>
#include <sphinxbase/matrix.h>


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
    float32 *ABloc=NULL;

    Aloc = (float32 ****) ckd_calloc_2d(nclass, nfeat, sizeof(float32 ***));
    Bloc = (float32 ***) ckd_calloc_2d(nclass, nfeat, sizeof(float32 **));

#if 0
    dump_regmat_statistics(regl,
			   regr,
			   nclass,
			   nfeat
			   veclen);

#endif
    /* Here, regl is the same as Legetter's G, while regr is Z.  The
     * original papers use the formula w_i^T = G(i)^{-1} z_i^T to
     * calculate the rows of W (the transformation matrix).  However
     * what they really mean by this is that we solve for w_i^T in the
     * equation G(i) w_i^T = z_i^T.  That is what this code does,
     * optionally omitting the bias or rotation parts of W. */
    for (m = 0; m < nclass; m++) {
	for (i = 0; i < nfeat; i++) {
	    len = veclen[i];

	    Aloc[m][i] = (float32 **) ckd_calloc_2d(len, len, sizeof(float32));
	    Bloc[m][i] = (float32 *) ckd_calloc(len, sizeof(float32));
	    ABloc = (float32 *) ckd_calloc(len+1, sizeof(float64));
 
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


/* Transform means using MLLR. */
int32
mllr_transform_mean(vector_t ***mean,
		    vector_t ***var, /* NOT USED */
		    uint32 gau_begin,
		    uint32 n_mgau,
		    uint32 n_feat,
		    uint32 n_density,
		    const uint32 *veclen,
		    float32 ****A,
		    float32 ***B,
		    int32 *cb2mllr,
		    uint32 n_mllr_class)
{
    uint32 i, j, k, l, m;
    float32 *tmean;

    for (i = gau_begin; i < n_mgau; i++) {
	int32 mc;

	if (cb2mllr)
	    mc = cb2mllr[i];
	else
	    mc = 0;
	if (mc < 0) continue;	/* skip */

	for (j = 0; j < n_feat; j++) {
	    tmean = (float32 *)ckd_calloc(veclen[j],sizeof(float32));

	    for (k = 0; k < n_density; k++) {
		for (l = 0; l < veclen[j]; l++) {
		    tmean[l] = 0;
		    for (m = 0; m < veclen[j]; m++) {
			tmean[l] += A[mc][j][l][m] * mean[i][j][k][m];

		    }
		    tmean[l] += B[mc][j][l];
		}

		/* Write back the transformed mean vector */
		for (l = 0; l < veclen[j]; l++) 
		    mean[i][j][k][l] = tmean[l];
		
	    }
	    ckd_free(tmean);
	}
    }
    return S3_SUCCESS;
}
