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
 * File: s2_read_tmat.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s2_read_tmat.h>

#include <s3/s2_param.h>
#include <s3/cmd_ln.h>
#include <s3/vector.h>
#include <s3/ckd_alloc.h>
#include <s3/acmod_set.h>
#include <s3/int32_io.h>
#include <s3/s3.h>

#include <s2/log.h>
#include <s2/magic.h>

#include <sys_compat/file.h>

#include <assert.h>

#include <stdio.h>


static int
premature_eof(FILE *fp)
{
    if (feof(fp)) {
	E_ERROR("Premature eof found while extracting trans mat\n");

	return TRUE;
    }
    else
	return FALSE;
}

static int
read_transition(int32 *from, int32 *to, int32 *prob, int32 *opdf_index, FILE *fp)
{
    if (premature_eof(fp))
	return S3_ERROR;
    *from = read_int32(fp);

    if (premature_eof(fp))
	return S3_ERROR;
    *to = read_int32(fp);

    if (premature_eof(fp))
	return S3_ERROR;
    *prob = read_int32(fp);

    if (premature_eof(fp))
	return S3_ERROR;
    *opdf_index = read_int32(fp);

    return S3_SUCCESS;
}

static int
rummage_hmm(float **tmat, FILE *fp)
{
    int32 opdf_index;
    int32 prob;
    int32 from, to;
    int32 magic;
    int32 n_cw;
    int32 n_omatrix;
    int32 n_state;
    int32 n_initial;
    int32 n_final;
    int32 n_arc;
    int32 gbg;
    uint32 i;
    
    if (premature_eof(fp)) return S3_ERROR;
    magic = read_int32(fp);

    if (!IS_MAGIC(magic)) {
	E_ERROR("Invalid HMM magic number found.  Byteorder problem?  Not an HMM?\n");

	return S3_ERROR;
    }

    if (premature_eof(fp)) return S3_ERROR;
    n_cw = read_int32(fp);
    assert(n_cw == S2_N_CODEWORD);

    if (premature_eof(fp)) return S3_ERROR;
    n_omatrix = read_int32(fp);
    assert(n_omatrix == S2_N_STATE-1);

    if ((magic == COUNT_F) || (magic == PROB_F)) {
	/* skip past stored output pdf's */
	fseek(fp, (long)(4 * n_omatrix * n_cw * sizeof(int32)), SEEK_CUR);
    }

    if (premature_eof(fp)) return S3_ERROR;
    n_state = read_int32(fp);
    assert(n_state == S2_N_STATE);

    if (premature_eof(fp)) return S3_ERROR;
    n_initial = read_int32(fp);
    assert(n_initial == 1);

    if (premature_eof(fp)) return S3_ERROR;
    gbg = read_int32(fp);

    assert(gbg == 0);

    if (premature_eof(fp)) return S3_ERROR;
    n_final = read_int32(fp);

    assert(n_final == 1);

    if (premature_eof(fp)) return S3_ERROR;
    gbg = read_int32(fp);

    /* last state is final */
    assert(gbg == (S2_N_STATE-1));

    if (premature_eof(fp)) return S3_ERROR;
    n_arc = read_int32(fp);
    assert(n_arc == 14);

    for (i = 0; i < n_arc; i++) {
	read_transition(&from, &to, &prob, &opdf_index, fp);
	
	tmat[from][to] = EXP(prob);
    }

    return S3_SUCCESS;
}

static int
extract_tmat(float **tmat, const char *in_dir_name, const char *ci_name)
{
    const char *hmm_ext;
    char ci_hmm_filename[MAXPATHLEN];
    FILE *fp;

    hmm_ext = cmd_ln_access("-hmmext");

    sprintf(ci_hmm_filename, "%s/%s.%s",
	    in_dir_name, ci_name, hmm_ext);
    
    fp = fopen(ci_hmm_filename, "rb");
    if (fp == NULL) {
	fflush(stdout);
	fprintf(stderr, "%s(%d): can't open %s for reading to extract tmat\n",
		__FILE__, __LINE__, ci_hmm_filename);
	fflush(stderr);

	return S3_ERROR;
    }
    
    if (rummage_hmm(tmat, fp) != S3_SUCCESS)
	return S3_ERROR;

    fclose(fp);

    return S3_SUCCESS;
}

int
normalize_floor_tmat(float **tmat, float32 tprob_floor, int n_state)
{
    uint32 i;
    int32 r, ret_val;

    ret_val = S3_SUCCESS;

    for (i = 0; i < n_state-1; i++) {
	vector_normalize(tmat[i], n_state);

	/* set non-zero (but "small") tprobs to floor value */
	vector_nz_floor(tmat[i], n_state, tprob_floor);
	r = vector_normalize(tmat[i], n_state);

	if (r != S3_SUCCESS)
	    ret_val = r;
    }

    return ret_val;
}

void
print_tmat(FILE *fp, float **tmat, int n_state)
{
    uint32 i, j;

    for (i = 0; i < n_state-1; i++) {
	for (j = 0; j < n_state; j++) {
	    fprintf(fp, "(%u %u) == %f\n", i, j, tmat[i][j]);
	}
    }

}

float32 ***
s2_read_tmat(const char *in_dir_name,
	     acmod_set_t *acmod_set,
	     float32 tprob_floor)
{ 
    float32 ***tmat;
    uint32 n_ci;
    uint32 i, j, k;
    int err;

    n_ci = acmod_set_n_ci(acmod_set);

    fflush(stdout);
    fprintf(stderr, "%s(%d): reading %d tied CI transition matrices from %s\n",
	    __FILE__, __LINE__,
	    n_ci,
	    in_dir_name);

    tmat = (float ***)ckd_calloc_3d(n_ci, S2_N_STATE-1, S2_N_STATE,
				    sizeof(float));

    fflush(stdout);
    fprintf(stderr, "%s(%d): applying floor %e\n",
	    __FILE__, __LINE__, tprob_floor);
    fflush(stderr);

    err = 0;
    for (i = 0; i < n_ci; i++) {
	for (j = 0; j < S2_N_STATE-1; j++) {
	    for (k = 0; k < S2_N_STATE; k++) {
		tmat[i][j][k] = 0.0;
	    }
	}

	if (extract_tmat(tmat[i],
			 in_dir_name,
			 acmod_set_id2name(acmod_set, i)) != S3_SUCCESS)
	    err = 1;
	
#ifdef S2_READ_TMAT_VERBOSE
	if (!err) {
	    print_tmat(stdout, tmat[i], S2_N_STATE);
	    fflush(stdout);
	}
#endif
    }
    
    if (!err) {
	return tmat;
    }
    else {
	return NULL;
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:30:32  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.10  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.9  1996/01/23  18:12:42  eht
 * Changes to remove either:
 * 	unused local variables
 * 	broken printf() format specifications
 * 	missing function prototypes in header files
 *
 * Revision 1.8  1995/10/17  14:03:23  eht
 * Changed to port to Windows NT
 *
 * Revision 1.7  1995/10/12  17:42:40  eht
 * Get SPHINX-II header files from <s2/...>
 *
 * Revision 1.6  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.5  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.4  1995/09/07  19:25:05  eht
 * Change read_long to read_int32 for machines where sizeof(long) != sizeof(int)
 *
 * Revision 1.3  95/08/09  20:32:05  20:32:05  eht (Eric Thayer)
 * No normalization of counts.
 * 
 * Revision 1.2  1995/06/02  20:30:30  eht
 * Add PWP's error reporting stuff
 *
 * Revision 1.1  1995/05/22  19:19:38  eht
 * Initial revision
 *
 *
 */
