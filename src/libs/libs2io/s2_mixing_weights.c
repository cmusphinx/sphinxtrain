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
 * File: s2_mixing_weights.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s2_mixing_weights.h>

#include <s3/s2_param.h>
#include <s3/ckd_alloc.h>
#include <s3/int32_io.h>
#include <s3/s3.h>

#include <s2/log.h>
#include <s2/magic.h>

#include <stdio.h>
#include <assert.h>

static void
normalize_opdf(int32 *pdf, int n_events)
{
    int i;
    int32 sum = MIN_LOG;
    
    for (i = 0; i < n_events; i++) {
	sum = ADD(sum, pdf[i]);
    }

    for (i = 0; i < n_events; i++) {
	pdf[i] -= sum;
    }
}

static int32
ckd_read_int32(FILE *fp)
{
    if (feof(fp)) {
	fflush(stdout);
	fprintf(stderr, "%s(%d): ERROR premature EOF\n", __FILE__, __LINE__);
	fflush(stderr);

	exit(1);
    }

    return read_int32(fp);
}

static void
read_opdf(int32 *opdf, int32 n_cw, FILE *fp)
{
    int i;

    for (i = 0; i < n_cw; i++) {
	opdf[i] = ckd_read_int32(fp);
    }
}

static void
exp_pdf(float32 *opdf, int32 *ipdf, int n_codewords)
{
    int i;

    for (i = 0; i < n_codewords; i++) {
	opdf[i] = EXP(ipdf[i]);
    }
}

int32
s2_mixing_weights(float32 ***out_mixw,
		  const char *hmm_file_name)
{
    int magic;		/* SPHINX-II hmm file magic number */
    int n_cw;		/* number of codewords for the model */
    int n_omatrix;	/* number of output pdfs in the model */
    int32 *tmp_opdf;
    uint32 i;
    FILE *fp;
    
    fp = fopen(hmm_file_name, "rb");
    if (fp == NULL) {
	fflush(stdout);
	fprintf(stderr, "%s(%d): ERROR cannot open HMM file %s for reading\n",
		__FILE__, __LINE__, hmm_file_name);
	perror(hmm_file_name);
	fflush(stderr);
	
	return S3_ERROR;
    }

    magic = ckd_read_int32(fp);
    if (!IS_MAGIC(magic)) {
	fflush(stdout);
	fprintf(stderr, "%s(%d): ERROR invalid magic number found.  Byteorder?  Not an HMM?\n", __FILE__, __LINE__);
	fflush(stderr);

	return S3_ERROR;
    }

    n_cw = ckd_read_int32(fp);
    assert(n_cw == S2_N_CODEWORD);

    tmp_opdf = (int32 *)ckd_calloc(n_cw, sizeof(int32));

    n_omatrix = ckd_read_int32(fp);

    assert(n_omatrix == S2_N_STATE-1);

    if ((magic == COUNT_F) || (magic == PROB_F)) {
	for (i = 0; i < n_omatrix; i++) {
	    read_opdf(tmp_opdf, n_cw, fp);
	    if (COUNT_P(magic))
		normalize_opdf(tmp_opdf, n_cw);
	    exp_pdf(out_mixw[i][0], tmp_opdf, n_cw);
	}

	for (i = 0; i < n_omatrix; i++) {
	    read_opdf(tmp_opdf, n_cw, fp);
	    if (COUNT_P(magic))
		normalize_opdf(tmp_opdf, n_cw);
	    exp_pdf(out_mixw[i][1], tmp_opdf, n_cw);
	}

	for (i = 0; i < n_omatrix; i++) {
	    read_opdf(tmp_opdf, n_cw, fp);
	    if (COUNT_P(magic))
		normalize_opdf(tmp_opdf, n_cw);
	    exp_pdf(out_mixw[i][2], tmp_opdf, n_cw);
	}

	for (i = 0; i < n_omatrix; i++) {
	    read_opdf(tmp_opdf, n_cw, fp);
	    if (COUNT_P(magic))
		normalize_opdf(tmp_opdf, n_cw);
	    exp_pdf(out_mixw[i][3], tmp_opdf, n_cw);
	}
    }

    ckd_free(tmp_opdf);
    return 0;
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
 * Revision 1.8  97/03/07  09:09:13  eht
 * - changed open mode "r" to "rb" so that will work on PC's
 * 
 * Revision 1.7  1996/01/23  18:12:42  eht
 * Changes to remove either:
 * 	unused local variables
 * 	broken printf() format specifications
 * 	missing function prototypes in header files
 *
 * Revision 1.6  1995/10/12  17:42:40  eht
 * Get SPHINX-II header files from <s2/...>
 *
 * Revision 1.5  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.4  1995/10/09  15:40:24  eht
 * Included missing <s3/ckd_alloc.h>
 *
 * Revision 1.3  1995/10/09  15:08:24  eht
 * changed ckd_alloc interface to remove need for __FILE__, __LINE__
 * arguments
 *
 * Revision 1.2  1995/09/07  19:23:56  eht
 * Changed read_longs to read_int32 to try to be explicit
 * about int lengths.
 *
 * Revision 1.1  95/05/22  19:19:11  19:19:11  eht (Eric Thayer)
 * Initial revision
 * 
 *
 */
