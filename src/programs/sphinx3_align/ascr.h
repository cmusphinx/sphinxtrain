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
 * ascr.h -- Acoustic (senone) scores
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
 * Revision 1.10  2006/02/23 05:19:57  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Added acoustic scores. 1, change names of sen to senscr. Add functions to clean up ascr's buffers.
 *
 * Revision 1.9.4.2  2005/08/02 21:11:33  arthchan2003
 * Changed sen to senscr, this avoid confusion in align,allphone, decode_anytopo.
 *
 * Revision 1.9.4.1  2005/07/24 01:31:03  arthchan2003
 * add interface to set temporary active array correctly.
 *
 * Revision 1.9  2005/06/21 22:32:25  arthchan2003
 * Log. Significant expansion of ascr_t to be the container of all types
 * of acoustic scores.  Implementations of init, report, free functions
 * are now provided. ascr_shift_one_cache_frame is used to provide a
 * wrapper for shifting one frame of cache frame in phoneme lookahead.
 * It is expected to be used later more if more complicated scheme of
 * lookahead is used.
 *
 * Revision 1.5  2005/04/20 03:31:57  archan
 * Part of refactoring: add ascr_shift_one_cache_frame which could shift one cache frame.
 *
 * Revision 1.4  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 19-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#include <s3types.h>

/** \file ascr.h
    \brief Wrapper to hold senone scores. 
*/
#ifndef _S3_ASCR_H_
#define _S3_ASCR_H_

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/**
 * Structure to hold senone scores (ordinary and composite), in one frame.
 */

typedef struct {
    int32 *senscr;		        /**< TEMPORARY VARIABLES: Senone scores in current frame */
    int32 *comsen;	        /**< TEMPORARY VARIABLES:  Composite senone scores in current frame */

    uint8 *ssid_active;		/**< TEMPORARY VARIABLES:  For determining the active senones in any frame */
    uint8 *comssid_active;        /**< TEMPORARY VARIABLES:  Composite senone active */
    uint8 *sen_active;            /**< TEMPORARY VARIABLES: Structure that record whether the current state is active. */
    uint8 *rec_sen_active;        /**< TEMPORARY VARIABLES: Most recent senone active state */
    int32 **cache_ci_senscr;      /**< TEMPORARY VARIABLES: Cache of ci senscr in the next pl_windows frames, include this frame.*/
    int32 *cache_best_list;       /**< TEMPORARY VARIABLES: Cache of best the ci sensr the next pl_windows, include this frame*/

    int32 n_sen;    /**< No. of senone. */
    int32 n_comsen; /**< No. of composite senone. */
    int32 n_sseq;   /**< No. of senone sequence. */
    int32 n_comsseq; /**< No. of composite senone sequence. */
    int32 pl_win;    /**< Phoneme lookahead window. */
    int32 n_cisen;    /**< No. of CI senone */
} ascr_t;


/**
 * Create an ascr_t structure for the given number of senones (ordinary and composite).
 * @return Pointer to created structure if successful, NULL otherwise.
 */
S3DECODER_EXPORT
ascr_t *ascr_init (int32 n_sen,		/**< Input: \#Ordinary senones */
		   int32 n_comsen,	/**< Input: \#Composite senones */
		   int32 n_sseq,        /**< Input: \# of senone sequences */ 
		   int32 n_comsseq,     /**< Input: \# of composite senone sequences */ 
		   int32 pl_win,        /**< Input: size of phoneme lookahead windows */
		   int32 n_cisen        /**< Input: \# of CI senones */
    );

/**
 * Report the ascr parameters. 
 */

void ascr_report(ascr_t * a /**<Input : ascr data structure */
    ) ;

/**
   Report the senone scores and comsen scores array. For debugging. 
*/
void ascr_print_senscr(ascr_t *a /**<Input: ascr data structure */
    );

/**
 * Free ascr_t 
 */ 
S3DECODER_EXPORT
void ascr_free(ascr_t  *a /**<Input: an initialized ascr data structure */
    );

/**
 * Shift one frame for the cache
 */
void ascr_shift_one_cache_frame(ascr_t *a,  /**<Input: an initialized ascr data structure */
				int32 win_efv /**<Input: the effective size of cache windows */
    );
     
/**
   Clear the senone active array
*/
void ascr_clear_sen_active(ascr_t *a /**<Input, an initialized ascr
					data structure */
    );

/** 
    Clear senone sequence ID active array. 
*/
void ascr_clear_ssid_active(ascr_t *a /**< Input, an initialized ascr
					 data structure */
    );
			    
/**
   Clear the composite senone sequence ID active array
*/
void ascr_clear_comssid_active(ascr_t *a /**< Input, an initialized ascr
					    data structure */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif


#endif

