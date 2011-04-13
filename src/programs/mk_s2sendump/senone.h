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

#ifndef _LIBMAIN_SENONE_H_
#define _LIBMAIN_SENONE_H_

#include <sphinxbase/prim_type.h>
#include <sphinxbase/logmath.h>



typedef int16   	s3senid_t;	/** Senone id */
typedef uint8 senprob_t;	/* Senone logprob, truncated to 8 bits */

/*
 * Mixture weights for a group of senones that share a single mixture Gaussian.  Split
 * into two structures: mixw_t and mgau2sen_t.  mgau2sen_t identifies the set of senones
 * that share each mixture Gaussian codebook.  But since a codebook consists of multiple
 * feature streams, with a separate mixture Gaussian of independent size for each stream,
 * the actual mixing weights for each stream are given by mixw_t.
 */

typedef struct {
    int32 n_wt;		/* #mixture weights (== parent mixture Gaussian mgau_t.n_mean) */
    senprob_t **prob;	/* prob[senone][mixture-component]; #row: parent mgau2sen_t.n_sen
			   and #col: n_wt */
} mixw_t;

typedef struct {
    int32 n_sen;	/* #Senones sharing this codebook */
    s3senid_t *sen;	/* List of senones sharing this codebook */
    mixw_t *feat_mixw;	/* Mixture weights for these senones for each feature */
} mgau2sen_t;

/* The main senones data structure */
typedef struct {
    uint32 n_sen;	/* Total #senones */
    uint32 n_feat;	/* #Features */
    uint32 n_mgau;	/* #Parent codebooks (mixture Gaussians for multiple features
			   count as one codebook) */
    uint32 shift;	/* LSB bits truncated from original logs3 value */
    mgau2sen_t *mgau2sen;	/* Set of senones for each parent codebook */
    uint32 *sen2mgau;		/* Unique parent codebook for each senone */
    uint32 *mgau2sen_idx;	/* mgau2sen_idx[s] = index into mgau2sen.sen[]
				   for senone s */
} senone_t;


/* Access macros */
#define senone_n_sen(s)		((s)->n_sen)
#define senone_n_mgau(s)	((s)->n_mgau)
#define senone_n_stream(s)	((s)->n_feat)

/*
 * Load a set of senones (mixing weights and mixture gaussian codebook mappings) from
 * the given files.  Normalize weights for each codebook, apply the given floor, convert
 * PDF values to logs3 domain and quantize to 8-bits.
 * Return value: pointer to senone structure created.  Caller MUST NOT change its contents.
 */
senone_t *senone_init (logmath_t *logmath,		/* In: logmath */
		       const char *mixwfile,		/* In: mixing weights file */
		       const char *mgau_mapfile,	/* In: file specifying mapping from each
						   senone to mixture gaussian codebook.
						   If NULL all senones map to codebook 0 */
		       float64 mixwfloor);		/* In: Floor value for senone weights */

#endif

