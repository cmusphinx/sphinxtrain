/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * 4. Redistributions of any form whatsoever must retain the following
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
/*********************************************************************
 *
 * File: s2_read_cb.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/s2_read_cb.h>

#include <s3/gauden.h>
#include <s3/s2_param.h>
#include <s3/s2io.h>
#include <s3/s3.h>

#include <s3/common.h>

#include <sys_compat/file.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

static uint32 cb_vlen[S2_N_FEATURE] = {
    S2_CEPF_VECLEN+1,
    S2_DCEPF_VECLEN+1,
    S2_POWF_VECLEN,
    S2_2DCEPF_VECLEN+1
};

static uint32 f_vlen[S2_N_FEATURE] = {
    S2_CEPF_VECLEN,
    S2_DCEPF_VECLEN,
    S2_POWF_VECLEN,
    S2_2DCEPF_VECLEN
};

static int screwball_offset[S2_N_FEATURE] = {
    1, 1, 0, 1
};

int
s2_read_cb(gauden_t *g,
	   const char *in_dir_name,
	   const char **cb_base,
	   const char *mean_ext,
	   const char *var_ext,
	   int do_floor,
	   const int32 *do_fixvar)
{
    vector_t ***mean;
    vector_t ***var;
    float32 *raw;
    int32 cblen;
    uint32 i, j, f;
    static float pv[] = {
	1.0/0.05,
	1.0,
	1.0/0.125
    };
    char cb_filename[MAXPATHLEN+1];

    gauden_set_feat(g, S2_N_FEATURE, f_vlen);
    gauden_set_const_dim_mgau(g, 1, S2_N_CODEWORD);

    mean = gauden_mean(g);
    var = gauden_var(g);

    if (*do_fixvar) {
	fflush(stdout);
	fprintf(stderr, "%s(%d): constant power variance\n",
		__FILE__, __LINE__);
	fflush(stderr);
	
	/* power variance is a constant for S2 */
	for (i = 0; i < S2_N_CODEWORD; i++) {
	    for (j = 0; j < f_vlen[S2_POW_FEATURE]; j++) {
		var[0][S2_POW_FEATURE][i][j] = pv[j];
	    }
	}
    }
    
    fflush(stdout);
    fprintf(stderr, "%s(%d): reading gau den from (",
	    __FILE__, __LINE__);

    for (f = 0; f < S2_N_FEATURE; f++) {
	sprintf(cb_filename, "%s/%s.%s",
		in_dir_name, cb_base[f], mean_ext);

	fprintf(stderr, "\n\t%s ", cb_filename);
	fflush(stderr);
	
	areadfloat(cb_filename, &raw, &cblen);
	assert(cblen == S2_N_CODEWORD * cb_vlen[f]);
	
	for (j = 0; j < S2_N_CODEWORD; j++) {
	    memcpy(mean[0][f][j],
		   &raw[j * cb_vlen[f] + screwball_offset[f]],
		   f_vlen[f] * sizeof(float32));
	}

	/* not ckd_free since SPHINX-II areadfloat uses malloc() */
	free(raw);
	
	if (!(*do_fixvar && (f == S2_POW_FEATURE))) {

	    sprintf(cb_filename, "%s/%s.%s",
		    in_dir_name, cb_base[f], var_ext);

	    fprintf(stderr, "%s", cb_filename);
	    fflush(stderr);

	    if (areadfloat(cb_filename, &raw, &cblen) < 0) {
		continue;
	    }
	    assert(cblen == S2_N_CODEWORD * cb_vlen[f]);

	    for (j = 0; j < S2_N_CODEWORD; j++) {
		memcpy(var[0][f][j],
		       &raw[j * cb_vlen[f] + screwball_offset[f]],
		       f_vlen[f] * sizeof(float32));
	    }

	    /* not ckd_free since SPHINX-II areadfloat uses malloc() */
	    free(raw);
	}
    }

    fprintf(stderr, "\n)\n");
    fprintf(stderr, "%s(%d): computing floor(variance)\n",
	    __FILE__, __LINE__);
    fflush(stderr);

    gauden_floor_variance(g);

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.9  97/03/07  09:09:46  eht
 * - use prototypes for s2io routines
 * 
 * Revision 1.8  1996/01/23  18:12:42  eht
 * Changes to remove either:
 * 	unused local variables
 * 	broken printf() format specifications
 * 	missing function prototypes in header files
 *
 * Revision 1.7  1995/12/14  20:16:02  eht
 * Fix power variance reading
 *
 * Revision 1.6  1995/11/10  20:14:10  eht
 * Add a fixvar parameter
 * ls
 *
 * Revision 1.5  1995/10/17  14:03:23  eht
 * Changed to port to Windows NT
 *
 * Revision 1.4  1995/10/12  17:42:40  eht
 * Get SPHINX-II header files from <s2/...>
 *
 * Revision 1.3  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/10/09  15:08:24  eht
 * changed ckd_alloc interface to remove need for __FILE__, __LINE__
 * arguments
 *
 * Revision 1.1  1995/05/22  19:19:38  eht
 * Initial revision
 *
 *
 */
