/* -*- c-basic-offset: 4 -*- */
/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * File: inc_densities.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "inc_densities.h"

#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

static int
not_done(uint32 k, uint32 *did, uint32 n_did)
{
    uint32 i;

    for (i = 0; (i < n_did) && (did[i] != k); i++);

    if (i < n_did)
	return FALSE;
    else
	return TRUE;
}

int
inc_densities(float32 ***new_mixw,
	      vector_t ***new_mean,
	      vector_t ***new_var,
	      vector_t ****new_fullvar,

	      float32 ***mixw,
	      vector_t ***mean,
	      vector_t ***var,
	      vector_t ****fullvar,
	      float32 ***dnom,
	      
	      uint32 n_mixw,
	      uint32 n_mgau,
	      uint32 n_dnom,
	      uint32 n_feat,
	      uint32 n_density,
	      const uint32 *veclen,
	      uint32 n_inc)
{
    uint32 i, j, k, l, r;
    uint32 **did;
    float32 max_wt;
    uint32 max_wt_idx;
    float32 std;

    assert(n_mgau <= n_mixw);
    
    if (n_mgau < n_mixw) {
	E_FATAL("Splitting of the tied mixture gaussians is not yet implemented\n");
    }

    /* copy old parameters into new arrays */
    for (i = 0; i < n_mgau; i++) {
	for (j = 0; j < n_feat; j++) {
	    for (k = 0; k < n_density; k++) {
		memcpy(new_mean[i][j][k], mean[i][j][k], veclen[j]*sizeof(float32));
		if (fullvar)
		    memcpy(new_fullvar[i][j][k][0], fullvar[i][j][k][0],
			   veclen[j]*veclen[j]*sizeof(float32));
		else
		    memcpy(new_var[i][j][k], var[i][j][k], veclen[j]*sizeof(float32));
		new_mixw[i][j][k] = mixw[i][j][k];
	    }
	}
    }

    /* Over all mixtures:
     * 	- Find the largest unsplit component density.
     *		- Split it into two components where the new component
     *			mixw_a = mixw_b = 1/2 mixw
     *			mean_a = mean + 0.2 std
     *			mean_b = mean - 0.2 std
     *			var_a = var_b = var
     *
     * New parameters are placed beginning at index n_density
     */

    did = (uint32 **)ckd_calloc_2d(n_feat, n_inc, sizeof(uint32));

    for (i = 0; i < n_mgau; i++) {
	printf("%u:", i);
	fflush(stdout);
	
	for (r = 0; r < n_inc; r++) {
	    for (j = 0; j < n_feat; j++) {
		did[j][r] = n_density;

		/* find the density w/ the largest EM count not yet split (i.e. most probable, most occurances) */
		for (k = 0, max_wt = -1.0, max_wt_idx = n_density; k < n_density; k++) {
		    if ((max_wt < dnom[i][j][k]) && not_done(k, did[j], r)) {
			max_wt = dnom[i][j][k];
			max_wt_idx = k;
		    }
		}

		if ( dnom[i][j][max_wt_idx] < MIN_IEEE_NORM_POS_FLOAT32 ) {
		    E_WARN("(mgau= %u, feat= %u, density= %u) never observed skipping\n",
			   i, j, k);

		    new_mixw[i][j][n_density+r] = 0;

		    if (fullvar)
			memcpy(new_fullvar[i][j][n_density+r][0], fullvar[i][j][0][0],
			       veclen[j]*veclen[j]*sizeof(float32));
		    else 
			memcpy(new_var[i][j][n_density+r], var[i][j][0],
			       veclen[j]*sizeof(float32));

		    memcpy(new_mean[i][j][n_density+r], mean[i][j][0],
			   veclen[j]*sizeof(float32));

		    continue;
		}

		/* mixing weight of prior and new densities == 1/2 prior mixing weight */
		new_mixw[i][j][max_wt_idx] /= 2;
		new_mixw[i][j][n_density+r] = new_mixw[i][j][max_wt_idx];

		/* Keep variance of new class same as old */
		if (fullvar)
		    memcpy(new_fullvar[i][j][n_density+r][0], fullvar[i][j][max_wt_idx][0],
			   veclen[j]*veclen[j]*sizeof(float32));
		else
		    memcpy(new_var[i][j][n_density+r], var[i][j][max_wt_idx],
			   veclen[j]*sizeof(float32));

		/* mean_a = mean + 0.2 std */
		/* mean_b = mean - 0.2 std */
		for (l = 0; l < veclen[j]; l++) {
		    /* Use the stddev of mean[l] itself for full covariances. */
		    if (fullvar)
			std = (float32)sqrt(fullvar[i][j][max_wt_idx][l][l]);
		    else
			std = (float32)sqrt(var[i][j][max_wt_idx][l]);
		    
		    new_mean[i][j][max_wt_idx][l] = mean[i][j][max_wt_idx][l] + 0.2f * std;
		    new_mean[i][j][n_density+r][l]  = mean[i][j][max_wt_idx][l] - 0.2f * std;
		}

		did[j][r] = max_wt_idx;
		printf("%u(%.2e)", did[j][r], max_wt);
		fflush(stdout);
	    }
	}
	printf("\n");
    }
	    

    return S3_SUCCESS;
}

