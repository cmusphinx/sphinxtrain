/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: mk_s2sendump.c
 * 
 * Description: 
 *    Make sendump file in converting S3 models to S2 models
 *
 * Author: 
 *    Alan W Black (awb@cs,cmu.edu)
 * Picked togeterh out off rkm's sen2s2.c
 *********************************************************************/

#include "parse_cmd_ln.h"
#include "senone.h"

#include <s3/model_def_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/byteorder.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/bio.h>
#include <sphinxbase/logmath.h>

#define NO_STATE	0xffffffff

static char *fmtdesc[] = {
    "BEGIN FILE FORMAT DESCRIPTION",
    "(int32) <length(string)> (including trailing 0)",
    "<string> (including trailing 0)",
    "... preceding 2 items repeated any number of times",
    "(int32) 0 (length(string)=0 terminates the header)",
    "(int32) <#codewords>",
    "(int32) <#pdfs>",
    "256 (int32) cluster-prob values for codebook-0 codeword-0",
    "#pdf (unsigned char) cluster-prob ids for codebook-0 codeword-0",
    "... preceding 2 items repeated for all codewords in codebook-0",
    "preceding 3 items repeated for codebooks 1, 2, 3.",
    "END FILE FORMAT DESCRIPTION",
    NULL,
};

static void fwrite_int32 (FILE *fp, int32 val)
{
    SWAP_LE_32(&val);
    fwrite (&val, sizeof(int), 1, fp);
}


static void senone_dump (const model_def_t *mdef, const senone_t *s, const char *file)
{
    int32 i, j, k, c, m, f, n, p, sb, se;
    mixw_t *fw;
    FILE *fpout;
    int32 lut[256];
    senprob_t *sp;
    int n_emit_state,d,d2,pmax,lpmax;
    
    E_INFO("Writing S2 format sendump file: %s\n", file);
    if ((fpout = fopen(file, "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", file);
    
    /* Write format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	n = strlen(fmtdesc[i])+1;
	fwrite_int32 (fpout, n);
	fwrite (fmtdesc[i], sizeof(char), n, fpout);
    }

    /* Terminate header */
    fwrite_int32 (fpout, 0);

    /* Write #codewords, #pdfs */
    if (s->n_mgau != 1)
	E_FATAL("#codebooks(%d) != 1\n", s->n_mgau);
    if (s->mgau2sen[0].n_sen != s->n_sen)
	E_FATAL("#senones for mgau[0](%d) != total #senones(%d)\n",
		s->mgau2sen[0].n_sen, s->n_sen);
    if (s->n_feat != 4)
	E_FATAL("#Features(%d) != 4\n", s->n_feat);
    for (i = 0; i < s->n_feat; i++) {
	if (s->mgau2sen[0].feat_mixw[i].n_wt != s->mgau2sen[0].feat_mixw[0].n_wt)
	    E_FATAL("#Wts not same for all features\n");
    }
    if (s->mgau2sen[0].feat_mixw[0].n_wt != 256)
	E_FATAL("#Wts(%d) != 256\n", s->mgau2sen[0].feat_mixw[0].n_wt);
    
    fwrite_int32 (fpout, s->mgau2sen[0].feat_mixw[0].n_wt);
    fwrite_int32 (fpout, s->n_sen);

    /* Create lut */
    for (i = 0; i < 256; i++)
	lut[i] = -(i << s->shift);
    
    sp = (senprob_t *) ckd_calloc (s->n_sen, sizeof(senprob_t));

    /* Write PDFs (#feat x #wt x #sen) */
    if (mdef->max_n_state - 1 != 5)

	E_FATAL("#States(%d) != 5\n", mdef->max_n_state - 1);
    n_emit_state = mdef->max_n_state - 1;
    for (f = 0; f < s->n_feat; f++) {
	fw = s->mgau2sen[0].feat_mixw;

	for (c = 0; c < fw[f].n_wt; c++) {
	    /*
	     * In S3, all CI-senones (for all CI-phones) come first.  CD-senones later.
	     * But in S2, for each CI-phone, CD-senones come first and then CI-senones.
	     */
	    k = 0, d=mdef->acmod_set->n_ci;
	    lpmax = mdef->acmod_set->n_ci * n_emit_state - 1;
	    for (p = 0; p < mdef->acmod_set->n_ci; p++) {
		/* CD senones first; find start and end points in S3 data */
		for (pmax = lpmax ; mdef->defn[d].tmat == p; d++)
		{
		    for (d2=0; d2 < n_emit_state; d2++)
			if (mdef->defn[d].state[d2] > pmax)
			    pmax = mdef->defn[d].state[d2];
		}
		sb = lpmax + 1;
		se = pmax;
		lpmax = pmax;
		
		for (i = sb; i <= se; i++) {
		    m = s->sen2mgau[i];
		    assert (m == 0);
		    j = s->mgau2sen_idx[i];
		    assert (j == i);
		    
		    sp[k++] = fw[f].prob[j][c];
		}

		/* CI senones next */
		sb = p * n_emit_state;
		se = sb + n_emit_state - 1;
		
		for (i = sb; i <= se; i++) {
		    m = s->sen2mgau[i];
		    assert (m == 0);
		    j = s->mgau2sen_idx[i];
		    assert (j == i);
		    
		    sp[k++] = fw[f].prob[j][c];
		}
	    }
	    assert (k == mdef->n_tied_state);
	    
	    /* Write lut for feat f, codeword c */
	    for (i = 0; i < 256; i++)
		fwrite_int32 (fpout, lut[i]);

	    /* Write data for feat f, codeword c */
	    fwrite (sp, sizeof(uint8), s->n_sen, fpout);
	}
    }

    fclose (fpout);
}

static void pocketsphinx_senone_dump(const model_def_t *mdef,
				     const senone_t *s, const char *file)
{
    FILE *fpout;
    char pshdr[256];
    int32 i, n, k, c, d, f;
    
    E_INFO("Writing PocketSphinx format sendump file: %s\n", file);
    if ((fpout = fopen(file, "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", file);
    
    /* Write format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	n = strlen(fmtdesc[i])+1;
	fwrite_int32 (fpout, n);
	fwrite (fmtdesc[i], sizeof(char), n, fpout);
    }
    /* Now write the cluster, feature and codebook counts */
    sprintf(pshdr, "cluster_count %d", 0);
    fwrite_int32(fpout, strlen(pshdr)+1);
    fwrite(pshdr, sizeof(char), strlen(pshdr)+1, fpout);
    sprintf(pshdr, "codebook_count %d", s->n_mgau);
    fwrite_int32(fpout, strlen(pshdr)+1);
    fwrite(pshdr, sizeof(char), strlen(pshdr)+1, fpout);
    sprintf(pshdr, "feature_count %d", s->n_feat);
    fwrite_int32(fpout, strlen(pshdr)+1);
    fwrite(pshdr, sizeof(char), strlen(pshdr)+1, fpout);

    /* Pad the header for alignment purposes */
    k = ftell(fpout) & 3;
    if (k > 0) {
        k = 4 - k;
        fwrite_int32(fpout, k);
        fwrite("!!!!", 1, k, fpout);
    }

    /* Terminate header */
    fwrite_int32(fpout, 0);

    /* For each codebook, write #codewords, #pdfs */
    for (c = 0; c < s->n_mgau; ++c) {
	/* Write #densities, #senones (indicates that they are transposed) */
	fwrite_int32 (fpout, s->mgau2sen[c].feat_mixw[0].n_wt);
	fwrite_int32 (fpout, s->mgau2sen[c].n_sen);

	/* Now write out transposed, quantized senones. */
	/* Note!  PocketSphinx puts them in Sphinx3 order. */
	for (f = 0; f < s->n_feat; f++) {
	    for (d = 0; d < s->mgau2sen[c].feat_mixw[f].n_wt; ++d) {
		for (i = 0; i < s->mgau2sen[c].n_sen; ++i) {
		    fputc(s->mgau2sen[c].feat_mixw[f].prob[i][d], fpout);
		}
	    }
	}
    }

    fclose (fpout);
}

int main (int32 argc, char **argv)
{
    model_def_t *m;
    float64 wtflr;
    const char *mdeffile, *senfile, *mgaumap, *outfile;
    senone_t *s;
    logmath_t *logmath;
    uint32 shift;
    
    parse_cmd_ln(argc, argv);

    mdeffile = cmd_ln_str("-moddeffn");
    mgaumap = ".semi.";
    senfile = cmd_ln_str("-mixwfn");
    wtflr = (float64)(cmd_ln_float32("-mwfloor"));
    outfile = cmd_ln_str("-sendumpfn");
    
    shift = cmd_ln_boolean("-pocketsphinx") ? 10 : 0;
    logmath = logmath_init(1.0001, shift, TRUE);

    model_def_read(&m, mdeffile);
    s = senone_init (logmath, senfile, mgaumap, wtflr);

    if (m->n_tied_state != s->n_sen)
	E_FATAL("#senones different in mdef(%d) and mixw(%d) files\n", m->n_tied_state, s->n_sen);
    
    if (cmd_ln_boolean("-pocketsphinx")) {
	pocketsphinx_senone_dump(m, s, outfile);
    }
    else {
	senone_dump(m, s, outfile);
    }

    return 0;
}

