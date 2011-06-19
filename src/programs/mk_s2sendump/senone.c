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

#include "senone.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3/vector.h>

#include <sphinxbase/err.h>
#include <sphinxbase/bio.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/logmath.h>


#define MAX_SENID	((int32)0x7fff)
#define MAX_MGAUID	((int16)0x7fff)
#define NOT_MGAUID(m)	((m)<0)
#define MIXW_PARAM_VERSION	"1.0"
#define SPDEF_PARAM_VERSION	"1.2"
/* For Pocketsphinx ONLY: Allocate 0..159 for negated quantized
 * mixture weights and 0..96 for negated normalized acoustic scores,
 * so that the combination of the two (for a single mixture) can never
 * exceed 255. */
#define MAX_NEG_MIXW 159 /**< Maximum negated mixture weight value. */
#define MAX_NEG_ASCR 96  /**< Maximum negated acoustic score value. */

static int32 senone_mgau_map_read (senone_t *s, const char *file_name)
{
    FILE *fp;
    int32 byteswap, chksum_present, n_mgau_present;
    uint32 chksum;
    int32 i;
    char eofchk;
    char **argname, **argval;
    float32 v;
    
    E_INFO("Reading senone-codebook map file: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    n_mgau_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], SPDEF_PARAM_VERSION) != 0) {
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
		       file_name, argval[i], SPDEF_PARAM_VERSION);
	    }
	    
	    /* HACK!! Convert version# to float32 and take appropriate action */
	    if (sscanf (argval[i], "%f", &v) != 1)
		E_FATAL("%s: Bad version no. string: %s\n", file_name, argval[i]);

	    n_mgau_present = (v > 1.1) ? 1 : 0;
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;
    
    /* Read #gauden (if version matches) */
    if (n_mgau_present) {
	if (bio_fread (&(s->n_mgau), sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	    E_FATAL("fread(%s) (#gauden) failed\n", file_name);
    }
    
    /* Read 1d array data; s->sen2mgau allocated by called function */
    if (bio_fread_1d ((void **)(&s->sen2mgau), sizeof(int32), &(s->n_sen), fp,
		      byteswap, &chksum) < 0) {
	E_FATAL("bio_fread_1d(%s) failed\n", file_name);
    }
    
    /* Infer n_mgau if not present in this version */
    if (! n_mgau_present) {
	s->n_mgau = 1;
	for (i = 0; i < s->n_sen; i++) {
	    if (s->sen2mgau[i] >= s->n_mgau)
		s->n_mgau = s->sen2mgau[i]+1;
	}
    }
    
    if (s->n_sen >= MAX_SENID)
	E_FATAL("%s: #senones (%d) exceeds limit (%d)\n", file_name, s->n_sen, MAX_SENID);
    if (s->n_mgau >= MAX_MGAUID)
	E_FATAL("%s: #gauden (%d) exceeds limit (%d)\n", file_name, s->n_mgau, MAX_MGAUID);

    /* Check for validity of mappings */
    for (i = 0; i < s->n_sen; i++) {
	if ((s->sen2mgau[i] >= s->n_mgau) || NOT_MGAUID(s->sen2mgau[i]))
	    E_FATAL("Bad sen2mgau[%d]= %d, out of range [0, %d)\n",
		    i, s->sen2mgau[i], s->n_mgau);
    }
    
    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read %d->%d senone-codebook mappings\n", s->n_sen, s->n_mgau);

    return 0;
}


/* In the old S3 files, all senones have the same "shape" (#codewords/senone/feat) */
static void build_mgau2sen (senone_t *s, int32 n_cw)
{
    int32 i, j, m, f;
    s3senid_t *sen;
    mixw_t *fw;
    
    /* Create mgau2sen map from sen2mgau */
    s->mgau2sen = (mgau2sen_t *) ckd_calloc (s->n_mgau, sizeof(mgau2sen_t));
    s->mgau2sen_idx = (uint32 *) ckd_calloc (s->n_sen, sizeof(int32));
    for (i = 0; i < s->n_sen; i++) {
	m = s->sen2mgau[i];
	assert ((m < s->n_mgau) && (m >= 0));
	(s->mgau2sen[m].n_sen)++;
    }
    
    sen = (s3senid_t *) ckd_calloc (s->n_sen, sizeof(s3senid_t));
    for (m = 0; m < s->n_mgau; m++) {
	s->mgau2sen[m].sen = sen;
	sen += s->mgau2sen[m].n_sen;
	s->mgau2sen[m].n_sen = 0;
    }

    for (i = 0; i < s->n_sen; i++) {
	m = s->sen2mgau[i];
	j = s->mgau2sen[m].n_sen;
	s->mgau2sen[m].sen[j] = i;
	s->mgau2sen_idx[i] = j;
	(s->mgau2sen[m].n_sen)++;
    }
    
    /* Allocate space for the weights */
    for (m = 0; m < s->n_mgau; m++) {
	fw = (mixw_t *) ckd_calloc (s->n_feat, sizeof(mixw_t));
	s->mgau2sen[m].feat_mixw = fw;

	for (f = 0; f < s->n_feat; f++) {
	    fw[f].n_wt = n_cw;
	    fw[f].prob = (senprob_t **) ckd_calloc_2d (s->mgau2sen[m].n_sen, n_cw,
						       sizeof(senprob_t));
	}
    }
}


static int32 senone_mixw_read(logmath_t * logmath, senone_t *s, const char *file_name, float64 mixwfloor)
{
    FILE *fp;
    char **argname, **argval;
    int32 byteswap, chksum_present;
    uint32 chksum;
    float32 *pdf;
    int32 i, j, f, m, c, p, n_sen, n_err, n_cw, nval;
    char eofchk;
    mixw_t *fw;
    
    E_INFO("Reading senone mixture weights: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], MIXW_PARAM_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
			file_name, argval[i], MIXW_PARAM_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #senones, #features, #codewords, arraysize */
    n_sen = s->n_sen;
    if ((bio_fread (&(s->n_sen),  sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(s->n_feat), sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(n_cw),   sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&nval,   sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
	E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if ((n_sen != 0) && (s->n_sen != n_sen))
	E_FATAL("#senones(%d) conflict with mapping file(%d)\n", s->n_sen, n_sen);
    if (s->n_sen >= MAX_SENID)
	E_FATAL("%s: #senones (%d) exceeds limit (%d)\n", file_name, s->n_sen, MAX_SENID);
    if (s->n_feat <= 0)
	E_FATAL("Bad #features: %d\n", s->n_feat);
    if (n_cw <= 0)
	E_FATAL("Bad #mixing-wts/senone: %d\n", n_cw);
    
    /* Allocate sen2mgau map if not yet done so (i.e. no explicit mapping file given */
    if (! s->sen2mgau) {
	assert ((s->n_mgau == 0) || (s->n_mgau == 1));
	
	s->sen2mgau = (uint32 *) ckd_calloc (s->n_sen, sizeof(int32));
	
	if (s->n_mgau == 1) {
	    /* Semicontinuous mode; all senones map to single, shared gaussian: 0 */
	    for (i = 0; i < s->n_sen; i++)
		s->sen2mgau[i] = 0;
	} else {
	    /* Fully continuous mode; each senone maps to own parent gaussian */
	    s->n_mgau = s->n_sen;
	    for (i = 0; i < s->n_sen; i++)
		s->sen2mgau[i] = i;
	}
    } else
	assert (s->n_mgau != 0);
    if (s->n_mgau >= MAX_MGAUID)
	E_FATAL("%s: #gauden (%d) exceeds limit (%d)\n", file_name, s->n_mgau, MAX_MGAUID);
    
    if (nval != s->n_sen * s->n_feat * n_cw) {
	E_FATAL("%s: #float32 values(%d) doesn't match dimensions: %d x %d x %d\n",
		file_name, nval, s->n_sen, s->n_feat, n_cw);
    }
    
    /*
     * Compute #LSB bits to be dropped to represent mixwfloor with 8 bits.
     * All PDF values will be truncated (in the LSB positions) by these many bits.
     */
    if ((mixwfloor <= 0.0) || (mixwfloor >= 1.0))
	E_FATAL("mixwfloor (%e) not in range (0, 1)\n", mixwfloor);

    /* Allocate memory for s->mgau2sen and senone PDF data */
    build_mgau2sen (s, n_cw);
    
    /* Temporary structure to read in floats */
    pdf = (float32 *) ckd_calloc (n_cw, sizeof(float32));

    /* Read senone probs data, normalize, floor, convert to logs3, truncate to 8 bits */
    n_err = 0;
    for (i = 0; i < s->n_sen; i++) {
	m = s->sen2mgau[i];	/* Parent mgau */
	j = s->mgau2sen_idx[i];	/* Index of senone i within list of senones for mgau m */
	fw = s->mgau2sen[m].feat_mixw;

	for (f = 0; f < s->n_feat; f++) {
	    if (bio_fread((void *)pdf, sizeof(float32), n_cw, fp, byteswap, &chksum)
		!= n_cw) {
		E_FATAL("bio_fread(%s) (arraydata) failed\n", file_name);
	    }
	    
	    /* Normalize and floor */
	    if (vector_sum_norm (pdf, n_cw) == 0.0)
		n_err++;
	    vector_floor (pdf, n_cw, mixwfloor);
	    vector_sum_norm (pdf, n_cw);

	    /* Convert to logs3, truncate to 8 bits, and store in s->pdf */
	    for (c = 0; c < n_cw; c++) {
		p = -logmath_log(logmath, pdf[c]);
		fw[f].prob[j][c] = p;
	    }
	}
    }
    if (n_err > 0)
	E_WARN("Weight normalization failed for %d senones\n", n_err);

    ckd_free (pdf);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read mixture weights for %d senones: %d features x %d codewords\n",
	   s->n_sen, s->n_feat, n_cw);
    
    return 0;
}

senone_t *senone_init (logmath_t *logmath, const char *mixwfile, const char *sen2mgau_map, float64 mixwfloor)
{
    senone_t *s;
    
    s = (senone_t *) ckd_calloc (1, sizeof(senone_t));
    s->n_sen = 0;	/* As yet unknown */
    s->sen2mgau = NULL;

    assert (sen2mgau_map);

    if (strcmp (sen2mgau_map, ".semi.") == 0) {
	/* Not a file; map all senones to a single parent mgau */
	s->n_mgau = 1;	/* But we don't yet know the #senones */
    } else if (strcmp (sen2mgau_map, ".cont.") == 0) {
	/* Not a file; map each senone to its own distinct parent mgau */
	s->n_mgau = 0;	/* We don't yet know the #senones */
    } else {
	/* Read mapping file */
	senone_mgau_map_read (s, sen2mgau_map);	/* Fills in n_sen */
    }
    
    assert (mixwfile);
    senone_mixw_read (logmath, s, mixwfile, mixwfloor);
    
    return s;
}
