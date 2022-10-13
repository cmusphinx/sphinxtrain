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
 * cont_mgau.h -- Mixture Gaussians for continuous HMM models.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.15  2006/02/22 16:32:40  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Dave's change in 1.18 for removing temp_hack is followed 2, The logic of uninit_compact is changed, by default the code will remove a Gaussian if it has zero mean AND zero variance. The old behavior (removal if Gaussian has zero mean.) could be retained if specifying -remove_zero_var_gau=1, 3, Fix issue in  .
 *
 *
 * Revision 1.13.4.3  2005/07/05 21:28:57  arthchan2003
 * 1, Merged from HEAD. 2, Remove redundant keyword in cont_mgau.
 *
 * Revision 1.13.4.2  2005/07/05 06:48:54  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.14  2005/07/04 20:57:53  dhdfu
 * Finally remove the "temporary hack" for the endpointer, and do
 * everything in logs3 domain.  Should make it faster and less likely to
 * crash on Alphas.
 *
 * Actually it kind of duplicates the existing GMM computation functions,
 * but it is slightly different (see the comment in classify.c).  I don't
 * know the rationale for this.
 *
 * Revision 1.13.4.1  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 *
 * Revision 1.13  2005/06/21 18:06:45  arthchan2003
 *
 * Log. 1, Fixed Doxygen documentation. 2, Added $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Log. 1, Fixed Doxygen documentation. 2, Added Revision 1.15  2006/02/22 16:32:40  arthchan2003
 * Log. 1, Fixed Doxygen documentation. 2, Added Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Dave's change in 1.18 for removing temp_hack is followed 2, The logic of uninit_compact is changed, by default the code will remove a Gaussian if it has zero mean AND zero variance. The old behavior (removal if Gaussian has zero mean.) could be retained if specifying -remove_zero_var_gau=1, 3, Fix issue in  .
 * Log. 1, Fixed Doxygen documentation. 2, Added
 *
 * Revision 1.4  2005/06/13 04:02:55  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added mgau_free to free memory allocated by mgau_init()
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added mgau_model_t.{frm_sen_eval,frm_gau_eval}.
 * 		Added mgau_var_nzvec_floor().

 * 
 * 28-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _S3_CONT_MGAU_H_
#define _S3_CONT_MGAU_H_

#include <s3types.h>

/** \file cont_mgau.h
 *  \brief Interface of full GMM computation with integer value of log likelihood. 
 */
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/**
 * Specification of sets of GMM 
 */

#define CONTHMM  10001 /**(NOT USED, for backward compatibility only)  */
#define SEMIHMM  10002 /**(NOT USED, for backward compatibllity only)  */

#define FULL_INT_COMP 20001  /** (NOT USED) Use full integer computation */
#define FULL_FLOAT_COMP 20002 /** (NOT USED) Use full floating point computation */
#define MIX_INT_FLOAT_COMP 20003 /** (NOT USED) Use mixture of integer and floating point computation */

#define MGAU_MEAN		1 /** Constant specifying the mean is used */
#define MGAU_VAR		2 /** Constant specifying the variance is used */
#define MGAU_FULLVAR		3 /** Constant specifying the full covariance is used */

#define NO_BSTIDX               -1 /** When there is no best index */
#define NOT_UPDATED              -100 /** Constant defined that a gaussian is not updated. */

/**
 * Mixture Gaussians: Weighted set of Gaussian densities, each with its own mean vector and
 * diagonal covariance matrix.  Specialized for continuous HMMs to improve speed performance.
 * So, a separate mixture Gaussian, with its own mixture weights, for each HMM state.  Also,
 * a single feature stream assumed.  (In other words, the mgau_t structure below represents
 * a senone in a fully continuous HMM model.)
 * 
 * Given a Gaussian density with mean vector m and diagonal variance vector v, and some
 * input vector x, all of length n, the Mahalanobis distance of x from the Gaussian mean m
 * is given by:
 *     {1/sqrt((2pi)^n * det(v))} * exp{-Sum((x[i] - m[i])^2 / (2v[i]))}
 * To speed up this evaluation, the first sub-expression ({1/sqrt...}) can be precomputed at
 * initialization, and so can 1/2v[i] in the second sub-expression.  Secondly, recognition
 * systems work with log-likelihood values, so these distances or likelihood values are
 * computed in log-domain.  Finally, float32 operations are costlier than int32 ones, so
 * the log-values are converted to logs3 domain (see libmisc/logs3.h) (but before the mixing
 * weights are applied).  Thus, to reiterate, the final scores are (int32) logs3 values.
 */

/* 20040826 ARCHAN: 
 * Introduced hook to the GMM definition to allow gaussian computation using full float 
 * operations. Also added another hook that allows potential use of full covariance matrix. 
 * At this point, full covariance matrix computation was not fully implemented. 
 * If comp_type = MIX_INT_FLOAT_COMP, then the interger pointer will be used to store the 
 * mixture weight.  It is also important to initialize the logs3 routine separately.
 * If comp_type = FULL_FLOAT_COMP, then the floating point pointer will be used to store the mixture
 * weights. 
 */

/**
 * \struct mgau_t
 * \brief A single mixture-Gaussian model for one senone (see above comment).
 */
typedef struct {
    int32 n_comp;	/**< \#Component Gaussians in this mixture.  NOTE: May be 0 (for the
			   untrained states). */

    int32 bstidx;         /**< Index for the most likely Gaussians in this mixture of component. It persists through time. */ 
    int32 bstscr;         /**< Scores for the most likely Gaussians in this mixture of component.*/
    int32 updatetime;     /**< Update time */

    /* Definition for mean */
    float32 **mean;	/**< The n_comp means of the Gaussians. The mean vector for a single mixture-Gaussian model for one senone. Dimension: n_comp * dimension */

    /* Definition for variances */
    float32 **var;	/**< The n_comp (diagonal) variances of the Gaussians.  Could be
			   converted to 1/(2*var) for faster computation (see above comment).  The diagonal variance vector for a single mixture-Gaussian model for one senone. Dimension: n_comp * dimension */

    float32 ***fullvar;   /* The n_comp (full) variances of the Gaussians. */
    /* A full co-variance matrix for a single mixture-Gaussian model for one senone */
    /* Dimension: n_comp * dimension * dimension */

    /* Definition for the log reciprocal terms */
    float32 *lrd;	        /**< Log(Reciprocal(Determinant (variance))).  (Then there is also a
				   (2pi)^(veclen) involved...) */

    /* Definitions for the mixture weights */
    int32 *mixw;	        /**< Mixture weights for the n_comp components (int32 instead of float32
				   because these values are in logs3 domain)*/
    float32 *mixw_f;      /**< (ONLY USED IN ENDPOINTER) mixture weights for the n_comp
                             components in float32 */
} mgau_t;



/**
 * \struct mgau_model_t
 * \brief The set of mixture-Gaussians in an acoustic model used in Sphinx 3.X family of tool
 */
typedef struct {
    int32 n_mgau;	/**< \#Mixture Gaussians in this model (i.e., \#senones) */
    int32 max_comp;	/**< Max components in any mixture */
    int32 veclen;	/**< Vector length of the Gaussian density means (and diagonal vars) */
    mgau_t *mgau;	/**< The n_mgau mixture Gaussians */
    float64 distfloor;	/**< Mahalanobis distances can underflow when finally converted to
                           logs3 values.  To prevent this, floor the log values first. */
    int32 comp_type;  /**< Type of computation used in this set of mixture-Gaussians*/		     
    int32 verbose;    /**< Whether to display information */

    /* Used only in the flat lexicon decoder, statistics */
    int32 frm_sen_eval;		/**< \#Senones evaluated in the most recent frame */
    int32 frm_gau_eval;		/**< \#Gaussian densities evaluated in the most recent frame */
    int32 frm_ci_sen_eval;        /**< \#CI Senones evaluated in most recent frame*/
    int32 frm_ci_gau_eval;        /**< \#CI Senones evaluated in most recent frame*/

    int32 gau_type; /**< gau_type=CONTHMM if it is fully continous HMM, 
                       gau_type=SEMIHMM if it is semi continous HMM. Currently SEMIHMM is not supported. */

    logmath_t *logmath;		/**< The logmath_t structure */
} mgau_model_t;


/** Access macros */
/** \def mgau_n_mgau
    Access number of GMMs
    \def mgau_max_comp
    Access the maximum number of components. It can be different across GMMs
    \def mgau_veclen
    Access an integer array contains the size of each stream
    \def mgau_n_comp
    Access the number of component for a particular mixture in a GMM
    \def mgau_mean 
    Access the mean
    \def mgau_var
    Access the variance
    \def mgau_lrd
    Access the floating point version of the Gaussian constant
    \def mgau_lrd
    Access the integer version of the Gaussian constant
    \def mgau_frm_sen_eval
    Number of senones evaluated in this frame
    \def mgau_frm_gau_eval
    Number of gaussians evaluated in this frame
    \def mgau_frm_cisen_eval
    Number of CI senones evaluated in this frame
    \def mgau_frm_cigau_eval
    Number of CI gaussians evaluated in this frame

*/
#define mgau_n_mgau(g)		((g)->n_mgau)
#define mgau_max_comp(g)	((g)->max_comp)
#define mgau_veclen(g)		((g)->veclen)
#define mgau_n_comp(g,m)	((g)->mgau[m].n_comp)
#define mgau_mean(g,m,c)	((g)->mgau[m].mean[c])
#define mgau_var(g,m,c)		((g)->mgau[m].var[c])
#define mgau_fullvar(g,m,c)	((g)->mgau[m].fullvar[c])
#define mgau_mixw(g,m,c)	((g)->mgau[m].mixw[c])
#define mgau_mixw_f(g,m,c)	((g)->mgau[m].mixw_f[c])
#define mgau_lrd(g,m,c)		((g)->mgau[m].lrd[c])
#define mgau_lrdi(g,m,c)	((g)->mgau[m].lrdi[c])
#define mgau_frm_sen_eval(g)	((g)->frm_sen_eval)
#define mgau_frm_gau_eval(g)	((g)->frm_gau_eval)
#define mgau_frm_cisen_eval(g)	((g)->frm_ci_sen_eval)
#define mgau_frm_cigau_eval(g)	((g)->frm_ci_gau_eval)

/**
 * Create a new mixture Gaussian model from the given files (Sphinx3 format).  Optionally,
 * apply the precomputations mentioned in the main comment above.
 * @see mgau_file_read
 * @see mgau_mixw_read
 * @see mgau_uninit_compact
 * @see mgau_var_floor
 * @see mgau_precomp
 * @return pointer to the model created if successful; NULL if error.
 */
S3DECODER_EXPORT
mgau_model_t *
mgau_init (const char *meanfile,	/**< In: File containing means of mixture gaussians */
	   const char *varfile,		/**< In: File containing variances of mixture gaussians */
	   float64 varfloor,		/**< In: Floor value applied to variances; e.g., 0.0001 */
	   const char *mixwfile,	/**< In: File containing mixture weights */
	   float64 mixwfloor,		/**< In: Floor value for mixture weights; e.g., 0.0000001 */
	   int32 precomp,		/**< In: If TRUE, create and precompute mgau_t.lrd and also
						   transform each var value to 1/(2*var).  (If FALSE, one
						   cannot use the evaluation routines provided here.) */
	   const char* senmgau,		/**< In: type of the gaussians distribution, .cont. or .semi. FIX 
						   me! This is confusing!*/
	   int32 comp_type,		/**< In: Type of computation in this set of gaussian mixtures. */
	   logmath_t *logmath);
				

/**
 * Floor any variance vector that is non-zero (vector).
 * @return No. of variance VALUES floored.
 */
int32 mgau_var_nzvec_floor (mgau_model_t *g,  /**< In: A mixture of Gaussian components */
			    float64 floor /**< In: The floor value */
    );


/**
 * Evaluate a single mixture Gaussian at the given vector x; i.e., compute the Mahalanobis
 * distance of x from each mean in the mixture, and combine them using the mixture weights.
 * Return value: The final score from this evaluation (a logs3 domain value).  NOTE: if the
 * specified mixture is empty, S3_LOGPROB_ZERO is returned (see libmisc/libmisc.h).
 * @return the senone score.
 */
/* The hybrid integer and floating point implementation of GMM computation */

S3DECODER_EXPORT
int32 mgau_eval (mgau_model_t *g,	/**< In: The entire mixture Gaussian model */
	   int32 m,		/**< In: The chosen mixture in the model (i.e., g->mgau[m]) */
	   int32 *active_comp,	/**< In: An optional, -1 terminated list of active component
				   indices; if non-NULL, only the specified components are
				   used in the evaluation. */
	   float32 *x, /**< In: Input observation vector (of length g->veclen). */
	   int32 fr,    /**< In: Frame number where GMM m is updated */
	   int32 bUpdBstIdx    /**< In: Whether the best index for the GMM will be updated or not */
    );		

/**
 * Like mgau_eval(), but return the scores of the individual components, instead of combining
 * them into a senone score.  
 * @return: Best component score.
 */
S3DECODER_EXPORT
int32 mgau_comp_eval (mgau_model_t *g,	/**< In: Set of mixture Gaussians */
		      int32 s,		/**< In: Mixture being considered */
		      float32 *x,	/**< In: Input vector being compared to the components */
		      int32 *score	/**< Out: Array of scores for each component */
    );

/**
 * A routine that dump all mean and variance parameters of a set of gaussian distribution.   
 * @return always 0
 */

int32 mgau_dump (mgau_model_t *g,  /**< In: Set of mixture Gaussians */
		 int32 type      /**< In: type of output, MGAU_MEAN
				    for mean or MGAU_VAR for
				    variance.  */
    );

/** RAH
 * Free memory allocated by mgau_init
 */
S3DECODER_EXPORT
void mgau_free (mgau_model_t *g /**< In: A set of model to free */
    );


/** 
 * Reloading the means. This is particularly useful for speaker adaptation. 
 */
S3DECODER_EXPORT
int32 mgau_mean_reload(mgau_model_t *g,  /**< In/Out : The mean which will be resetted*/
		       const char* mean_file_name /**< In: The mean files */
    );

/** 
 * Reload the variances without precomputation. This is particularly
 * useful for speaker adaptation.
 */
S3DECODER_EXPORT
int32 mgau_var_reload(mgau_model_t * g,      /**< In/Out: The GMM */
                      const char *var_file_name       /**< In: file name for the mean file. */
    );

/**
 * Precompute things from the variances.  Not needed unless you explicitly call
 * mgau_var_reload().
 */
S3DECODER_EXPORT
int32 mgau_precomp(mgau_model_t * g);

/**
 * Floor the value of gaussian mixture weight using the parameter floor
 * @param floor
 */

S3DECODER_EXPORT 
void mgau_var_floor(mgau_model_t * g, float64 floor);

/**
 * Compact each mixture Gaussian in the given model by removing any
 * uninitialized components.  A component is considered to be
 * uninitialized if its variance is the 0 vector or if its mean or
 * variance contain NaN (not-a-numbers).  Compact by copying the data
 * rather than moving pointers.  Otherwise, malloc pointers could get
 * corrupted.
 * 
 * The past behavior will remove a Gaussian if the mean and variance
 * vectors are 0-vector. This can be turned on by changing the #if
 * flag.
 */

S3DECODER_EXPORT
void mgau_uninit_compact(mgau_model_t * g);


#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif


#endif
