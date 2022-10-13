/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
/*
 * subvq.h
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.14  2006/02/22 17:43:32  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH:
 * 1, vector_gautbl_free is not appropiate to be used in this case because it will free a certain piece of memory twice.
 * 2, Fixed dox-doc.
 *
 * Revision 1.13.4.1  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 * Revision 1.13  2005/06/21 19:01:33  arthchan2003
 * Added $ keyword.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Updated subvq_free () to free more allocated memory
 * 
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Moved subvq_t.{frm_sen_eval,frm_gau_eval} to cont_mgau.h.
 * 
 * 14-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_t.{frm_sen_eval,frm_gau_eval}.  Changed subvq_frame_eval to
 * 		return the normalization factor.
 * 
 * 06-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_subvec_eval_logs3().
 * 
 * 14-Oct-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed ci_active flags input to sen_active in subvq_frame_eval().
 * 
 * 20-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_gautbl_eval_logs3().
 * 
 * 12-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_SUBVQ_H_
#define _S3_SUBVQ_H_

#include "s3types.h"
#include "cont_mgau.h"
#include "vector.h"

/** \file subvq.h
    \brief Implementation of Sub-vector quantization.
*/
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** \struct subvq_t
 * \brief Wrapper structures of sub-vector quantization
 */
typedef struct {
    arraysize_t origsize;	/**< origsize.r = \#codebooks (or states) in original model;
				   origsize.c = max \#codewords/codebook in original model. */
    int32 n_sv;			/**< \#Subvectors */
    int32 vqsize;		/**< \#Codewords in each subvector quantized mean/var table */
    int32 **featdim;		/**< featdim[s] = Original feature dimensions in subvector s */
    vector_gautbl_t *gautbl;	/**< Vector-quantized Gaussians table for each sub-vector */
    int32 ***map;		/**< map[i][j] = map from original codebook(i)/codeword(j) to
				   sequence of nearest vector quantized subvector codewords;
				   so, each map[i][j] is of length n_sv.  Finally, map is
				   LINEARIZED, so that it indexes into a 1-D array of scores
				   rather than a 2-D array (for faster access). */ 
    /* Working space used during evaluation. */
    float32 *subvec;		/**< Subvector extracted from feature vector */
    int32 **vqdist;		/**< vqdist[i][j] = score (distance) for i-th subvector compared
				   to j-th subvector-codeword */
    int32 *gauscore;		/**< Subvq-based approx. Gaussian density scores for one mixture */
    int32 *mgau_sl;		/**< Shortlist for one mixture (based on gauscore[]) */

    /* ARCHAN, 1111, 04, move the static global variables to the structure again. */
    /* RAH, 5.8.01, VQ_EVAL determines how many vectors are used to
     * compute the shortlist, for now this value is only relevant when n_sv =3.
     * Setting it to 1 means that only the CEP values are estimated, 2 means that 
     * CEP and delta values are estimated, 3 means all three are estimated.
     * Note, we must adjust the beam widths as we muck around with these.
     */

    int32 VQ_EVAL;              /** Number of sub-vector to be computed */
} subvq_t;


/**
 * SubVQ file format:
 *   VQParam \#Original-Codebooks \#Original-Codewords/codebook(max) -> \#Subvectors \#VQ-codewords
 *   Subvector 0 length \<length\> \<feature-dim\> \<feature-dim\> \<feature-dim\> ...
 *   Subvector 1 length \<length\> \<feature-dim\> \<feature-dim\> \<feature-dim\> ...
 *   ...
 *   Codebook 0
 *   Row 0 of mean/var values (interleaved) for subvector 0 codebook (in 1 line)
 *   Row 1 of above
 *   Row 2 of above
 *   ...
 *   Map 0
 *   Mappings for state 0 codewords (in original model) to codewords of this subvector codebook
 *   Mappings for state 1 codewords (in original model) to codewords of this subvector codebook
 *   Mappings for state 2 codewords (in original model) to codewords of this subvector codebook
 *   ...
 *   Repeated for each subvector codebook 1
 *   Repeated for each subvector codebook 2
 *   ...
 *   End
 *   @return initialized sub-vq
 */
subvq_t *subvq_init (const char *file,	/**< In: Subvector model file */
		     float64 varfloor,	/**< In: Floor to be applied to variance values */
		     int32 max_sv,	/**< In: Use the first so many subvectors instead of all;
					   if <0, use all */
		     mgau_model_t *g,	/**< In: Original model from
					   which this subvq model was
					   built, for
					   cross-validation; optional */
		     cmd_ln_t *config,
		     logmath_t *logmath
    );	



/** Deallocate sub-vector quantization */
void subvq_free (subvq_t *vq /**< In: A sub-vector model */
    );


/**
 * Evaluate senone scores for one frame.  If subvq model is available, for each senone, first
 * get approximate Gaussian density scores using it; obtain a shortlist of Gaussians using
 * these scores, then evaluate the shortlist exactly.  If no subvq model, evaluate senones
 * using all Gaussian densities.  Finally, scale senone scores by subtracting the best.
 * @return The normalization factor (best senone absolute score).
 */
int32 subvq_frame_eval (subvq_t *vq,	/**< In: A sub-vector model */
			mgau_model_t *g,/**< In: Exact mixture Gaussian model */
			int32 beam,	/**< In: (Logs3) threshold for selecting shortlist;
					   range = [-infinity(widest beam), 0(narrowest)] */
			float32 *feat,	/**< In: Input feature vector for this frame */
			int32 *sen_active,	/**< In: Active flags for each senone (optional).
						   If not NULL, only active ones evaluated */
			int32 *senscr /**< Out: Normalized senone scores */
    );	

/**
 * Evaluate the Mahalanobis distances between the given feature vector and each entry in the
 * given subvq codebook.  Save results, as logs3 values, in vq->vqdist[][].
 */
void subvq_gautbl_eval_logs3 (subvq_t *vq,	/**< In/Out: Reference subvq structure */
			      float32 *feat,	/**< In: Subvectors
						   extracted from this, and compared to
						   relevant subvq codewords */
			      logmath_t *logmath
    );

/**
 * Evaluate the codewords for a single given subvector sv, wrt the input feature vector.
 * Save results, as logs3 values, in vq->vqdist[sv][].
 * (Basically, like subvq_gautbl_eval_logs3, but for a single given subvector instead of all.)
 */
void subvq_subvec_eval_logs3 (subvq_t *vq,	/**< In/Out: Reference subvq structure */
			      float32 *feat,	/**< In: Input feature
						 ** subvector
						 ** extracted from
						 ** this, and compared
						 ** to relevant
						 ** codewords */

			      int32 sv,	        /**< In: ID of
						   subvector being
						   evaluated */
			      logmath_t *logmath
    );

/*
 * Based on previously computed subvq scores (Mahalanobis distances), determine the active
 * components in the given mixture (using the vq->map).
 * @return Number of candidates in the returned shortlist.
 */
int32 subvq_mgau_shortlist (subvq_t *vq,        /**< In subvq */
			    int32 m,	/**< In: GMM index */
			    int32 n,	/**< In: \#Components in specified mixture */
			    int32 beam	/**< In: Threshold to select active components */
    );


/**
 * Compute the scores of a gaussian using only sum of the sub-vector scores. 
 * @return best senone score. 
 */

int32 subvq_mgau_eval (mgau_model_t *g, /**< In: Gaussian */
		       subvq_t *vq, /**< In: the SVQ */
		       int32 m, /**< In: GMM Index */
		       int32 n, /**< In :\#Components in a specified mixture */
		       int32 *active /**< In:Active list of mixture */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
