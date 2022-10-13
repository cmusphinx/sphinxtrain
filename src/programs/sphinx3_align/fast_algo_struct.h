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
 * fast_algo_struct.h -- Various forms of pruning beam
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
 * Revision 1.10  2006/02/22 16:39:43  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Initialize beam->n_ciphone properly, 2, use ckd_free instead of free, use float64 for subvqbeam and cipbeam.  3, Add a proper free function for fast_gmm_free
 *
 *
 * Revision 1.7.4.5  2005/11/17 06:07:07  arthchan2003
 * Added fast_gmm_free in the header.
 *
 * Revision 1.7.4.4  2005/07/05 21:28:57  arthchan2003
 * 1, Merged from HEAD. 2, Remove redundant keyword in cont_mgau.
 *
 * Revision 1.7.4.3  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 * Revision 1.7.4.2  2005/07/04 02:44:25  arthchan2003
 * Changed float32 to float64 for ci_pbeam and svqbeam. Code now compiled.
 *
 * Revision 1.7.4.1  2005/06/27 05:22:19  arthchan2003
 * Merged from the HEAD.
 *
 * Revision 1.9  2005/06/30 13:08:45  egouvea
 * Beams in linear scale have to be float64, since they can be easily defined as < 1e-40
 *
 * Revision 1.8  2005/06/22 08:00:09  arthchan2003
 * Completed all doxygen documentation on file description for libs3decoder/libutil/libs3audio and programs.
 *
 * Revision 1.7  2005/06/21 18:26:38  arthchan2003
 * Log. fast_algo_struct.c go through major changes in the gentle
 * refactoring process. It is the location of several wrapper structures
 * that control fast search.  That includes beam_t for storing beams and
 * scores. pl_t for storing structure for phoneme lookahead, histprune_t
 * for storing structures for histogram pruning. Lastly
 * fast_algo_struct_t, for storing structures for fast GMM
 * computation.
 *
 * Log. General Remark All of them now has consistent inteface, _init,
 * _report and _free.  They are respectively used for allocation,
 * reporting and deallocation of the routine. Doxygen documentation are
 * fixed for all structures.
 *
 * Log. Individual changes; beam_t start to own bestscore, bestwordscore,
 * wordbestscores, wordbestexits. They were owned by kb_t. histprune_t
 * now wrapped up maxwpf, maxhmmpdf, maxhistpf and
 * hmm_hist_binsize. Currently, the beam size determination routine is
 * controlled by search implementation modules.  It is done because
 * wrapping that operation up means we may need to introduce a bridge
 * between beam_t and histprune_t.  pl_t is now owning heuristic type,
 * the phoneme lookahead beam size. It also wrapped up phoneme heuristic
 * computation.
 *
 * Revision 1.5  2005/04/20 03:33:54  archan
 * Remove pl_win and pl_win_strt, Now consider them as the parameters of the search abstraction in srch.c
 *
 * Revision 1.4  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 19-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

/** \file fast_algo_struct.h
 * \brief File that implement various structure for fast algorithms.
 * fast_algo_struct implement beam_t, histprune_t, pl_t and fast_algo_struct_t
 */


#ifndef _S3_BEAM_H_
#define _S3_BEAM_H_

#include <s3types.h>
#include <mdef.h>
#include <ascr.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif


/** 
 *  \struct beam_t fast_algo_struct.h "fast_algo_struct.h"
 *  \brief Structure that contains all beam parameters for beam pruning in Viterbi algorithm. 
 *  
 *  This function include the definition of beam in multiple level of pruning in Viterbi
 *  algorithm.  That includes hmm (state-level), ptrans (phone-level), word (word-level). 
 *  ptranskip is used to specify how often in the Viterbi algorithm that phoneme level word 
 *  beam will be replaced by a word-level beam. 
 */

/**
 * Structure containing various beamwidth parameters.  All logs3 values; -infinite is widest,
 * 0 is narrowest.
 */

typedef struct {
    int32 hmm;		   /**< For selecting active HMMs, relative to best */
    int32 ptrans;	   /**< For determining which HMMs transition to their successors */
    int32 word;		   /**< For selecting words exited, relative to best HMM score */
    int32 ptranskip;       /**< Intervals at which wbeam is used for phone transitions */
    int32 wordend;         /**< For selecting the number of word ends  */
    int32 n_ciphone;       /**< No. of ci phone used to initialized the word best and exits list*/
    
    int32 bestscore;	   /**< Temporary variable: Best HMM state score in current frame */
    int32 bestwordscore;   /**< Temporary variable: Best wordexit HMM state score in current frame. */
    int32 thres;           /**< Temporary variable: The current frame general threshold */
    int32 phone_thres;     /**< Temporary variable: The current frame phone threshold */
    int32 word_thres;      /**< Temporary variable: The current frame phone threshold */

    int32 *wordbestscores; /**< The word best score list */
    int32 *wordbestexits;  /**< The word best exits list */

} beam_t;

/** 
 *  \struct pl_t 
 *  \brief Structure that contains all parameters for phoneme lookahead 
 */

typedef struct{
    int32 pheurtype;       /**< For differnt phoneme lookahead heuristic type. */
    int32 pl_beam;         /**< Phoneme lookahead beams */

    int32 n_ciphone;       /**< Number of ciphones assumed */
    int32 *phn_heur_list;  /**< Cache of best the ci phoneme scores
                              in the next pl_windows, include this
                              frame*/

}pl_t;

/**
 *  \struct histprune_t 
 * \brief Structure containing various histogram pruning parameters and internal storage  All in integers.
 * 
 */

typedef struct {
    int32 maxwpf;          /**< Max words per frame*/
    int32 maxhistpf;       /**< Max histories per frame*/
    int32 maxhmmpf;        /**< Max active HMMs per frame*/
    int32 hmm_hist_binsize;/**< Hmm histogram bin size */
    int32 hmm_hist_bins;   /**< Number of histogram bins*/
    int32 *hmm_hist;	   /**< Histogram: \#frames in which a given no. of HMMs are active */
    
} histprune_t;

/**
 * \struct downsampling_t 
 * \brief Structure containing various parameters for downsampling.
 */

typedef struct{
    int32 ds_ratio;        /**< Ratio of down-sampling the frame computation */
    int32 cond_ds;         /**< Whether we want to use conditional DS, 
                              cond_ds=0, don't use,
                              cond_ds=1, store previous 1 frame
                           */
    
    int32 dist_ds;         /**< Whether we want to use distance-based DS,
                              dist_ds=0, don't use,
                              dist_ds=1, store previous 1 frame
                           */
    
    int32 skip_count;      /**< Counting how many frames are skipped */

} downsampling_t;

/**
 * \struct gmm_select_t
 * \brief Structure that contains all parameters for CI-based GMM selection.
 */
typedef struct{
    int32 ci_pbeam;        /**< The beam which prune out unnesseary parent CI phones in 
                              CI-based GMM selection*/
    int32 dyn_ci_pbeam;    /**< The dynamic CI-beam computed by using both CI-pbeam and 
                              the counts*/
    float32 tighten_factor;/**< In the range of [0,1] (closed bounds)
                              A factor used to decide how many CD
                              senones will be computed. for those
                              "ignored" frames. */
    int32 *ci_occu;        /**< Recorded number of CD senones for a
                              particular base CI senone. */
    int32 *idx;            /**< temporary indices used in absolute
                              discounting of CI-based GMM selection */
    int32 max_cd;          /**< Maximum CD senones computed. 
                            */
} gmm_select_t;

/**
 * \struct gau_select_t
 * \brief Structure that contains all parameters related to Gaussian selection.
 */

typedef struct{
    int32 subvqbeam;	   /**< For selecting active mixture components based on subvq scores */
    int32 rec_bstcid;      /**< Best codeword ID for Gaussian Selection Map. */
} gau_select_t;

/**
 * \struct fast_gmm_t
 * \brief Structure that contains all parameter related to 4-Level Fast GMM computation. 
 */
typedef struct{
    downsampling_t* downs; /**< All structure for down-sampling */
    gmm_select_t* gmms;    /**< All structure for GMM-level of selection */
    gau_select_t* gaus;    /**< All structure for Gaussian-level of selection */
    int32 gs4gs;           /**< Whether the GS map is used for Gaussian Selection or not 
                              mainly for internal debugging of Conditional Down-Sampling */
    int32 svq4svq;         /**< Whether SVQ scores would be used as the Gaussian Scores */
    int32 rec_bst_senscr;  /**< recent best scores. */
    float32 *last_feat;    /**< Last feature frame */

} fast_gmm_t;

/**
 * Create and initialize a beam_t structure, with the given
 * parameters, converting them from prob space to logs3 space.  
 *
 * Note the last parameter is used in controling when the word beam is
 * applied.  
 * @return a pointer to created structure if successful, NULL otherwise.
 */

beam_t *beam_init (
    float64 hmm,  /**< Input: hmm beam*/
    float64 ptr,  /**< Input: phone transition beam */
    float64 wd,  /**< Input: word beam */
    float64 wdend,  /**< Input: word end beam. */
    int32 ptranskip, /**< Input: whether to apply phoneme transition beam rather than word beam.*/
    int32 n_ciphone,  /**< Input: number of ciphone to initialized arrays used in word end pruning. */
    logmath_t *logmath
    );


/** report the content of the beam_t data structure*/
void beam_report(beam_t* b /**< beam data structure */
    );

/** free the beam_t data structure */
void beam_free(beam_t *b /**< beam data structure */
    );

/**
 * Create and initialize a histprune_t structure, with the given parameters. 
 */

histprune_t *histprune_init (int32 maxhmm,         /**< Input: Maximum HMM per frame */
			     int32 maxhist,          /**< Input: Maximum history per frame */
			     int32 maxword,        /** Input: Maximum no. words per frame */
			     int32 hmmhistbinsize, /**< Input: The size of each histogram */
			     int32 numNodes        /**< Input: The number of nodes in the tree (or graph?) */
    );

void histprune_zero_histbin(histprune_t *h         /**< Input: histpurne structure */
    );
  
void histprune_update_histbinsize(histprune_t *h, /**< Input: histprune structure */
				  int32 hmmhistbinsize,  /**< Input: The size of each histogram */
				  int32 numNodes        /**< Input: The number of nodes in the tree (or graph?) */
    );

/** report the content of the histprune_t data structure*/
void histprune_report(histprune_t* h /**< Input: histprune structure */
    );

/** report the histogram bins */
void histprune_showhistbin(histprune_t *hp, /**< Input: histprune structure */
			   int32 nfr, /**< Number of frames in an utterance */
			   char* uttid /**< Utterance ID */
    );

/** free the histprune data structure */
void histprune_free(histprune_t *h /**< Input/Output :histprune structure */
    );
/**
 * Create and initialize a fast_gmm_t structure, withe the given parameters
 */
S3DECODER_EXPORT
fast_gmm_t *fast_gmm_init (int32 down_sampling_ratio, /**<  Input: The frame down-sampling ratio */
			   int32 mode_cond_ds,  /**< Input: Whether conditional down sampling is used */
			   int32 mode_dist_ds, /**< Input: (NOT USED) Whether distance-based down sampling is used */
			   int32 isGS4GS, /**< Input: Whether Gaussian selection map is only used for Gaussian Selection. Not for down sampling */
			   int32 isSVQ4SVQ, /**< Input: Whether SVQ is used in acoustic model computation. */
			   float64 subvqbeam, /**< Input: Subvq beam */
			   float64 cibeam,  /**< Input: CI phone beam */
			   float32 tighten_factor, /**< Input : A tightening factor used in down sampling */
			   int32 max_cd,    /**< Input: Max CD senone to be computed */
			   int32 n_ci_sen, /**< Input: no. of ci senone, use to initialize  the ci_occ array*/
			   logmath_t *logmath
    );

/** report the content of the fast_gmm_t data structure*/
void fast_gmm_report(fast_gmm_t* f  /**< Input: the fast GMM computation structure */
    );

/** Free the fast_gmm_t structure */
S3DECODER_EXPORT
void fast_gmm_free (fast_gmm_t *fg /**< Input: structure to free*/
    );
  

/**
 * Create and initialize the pl_t data structure 
 * @return an initialized phoneme lookahead data structure
 */
pl_t* pl_init(int32 pheurtype, /**< Input: Phoneme lookahead heuristic types TBD: (NOT DETAIL ENOUGH)*/
	      float64 pl_beam,  /**< Input: Phoneme lookahead beam */
	      int32 n_ciphone, /**< Input: Number of CI phones used in the cache size. */
	      logmath_t *logmath
    );

/** report the content of the phoneme lookahead data structure pl_t */
void pl_report(pl_t* pl  /**< Input: a phoneme lookahead data structure*/
    );

/** Free the phoneme lookahead data structure */
void pl_free(pl_t* pl /**< Input/Output: a phoneme looahead data structure */
    );

/** Phoneme look-ahead and compute phoneme heuristic.  This routine
    will compute a phone heuristic scores with lookahead win_efv
    frame and start at frame win_strt. 
*/
void pl_computePhnHeur(mdef_t* md, /**< Input: a model definition file*/
		       ascr_t *a,  /**< Input: an acoustic score data structure */
		       pl_t* pl,   /**< Input: a phoneme-lookahead data structure */
		       int32 heutype, /**< Input: type of heuristic used */
		       int32 win_strt, /**< Input: Starting frame of a window */
		       int32 win_efv   /**< Input: The effective window size*/
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif


#endif
