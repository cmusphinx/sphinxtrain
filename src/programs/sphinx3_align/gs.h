/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * gs.h -- gaussian density module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.7  2006/02/22 16:41:58  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed issue in $ keyword, 2, Fixed dox-doc
 *
 * Revision 1.6.4.1  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 * Revision 1.6  2005/06/21 18:28:05  arthchan2003
 * Log. 1, Fixed doxygen documentation. 2, Add $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Log. 1, Fixed doxygen documentation. 2, Add Revision 1.7  2006/02/22 16:41:58  arthchan2003
 * Log. 1, Fixed doxygen documentation. 2, Add Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed issue in $ keyword, 2, Fixed dox-doc
 * Log. 1, Fixed doxygen documentation. 2, Add
 * Log. 1, Fixed doxygen documentation. 2, Add Revision 1.6.4.1  2005/07/05 05:47:59  arthchan2003
 * Log. 1, Fixed doxygen documentation. 2, Add Fixed dox-doc. struct level of documentation are included.
 * Log. 1, Fixed doxygen documentation. 2, Add keyword.
 *
 * Revision 1.4  2005/06/13 04:02:55  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 29-Dec-03    A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University.
 *              Initial version created           
 */

#ifndef _LIB_GS_H_
#define _LIB_GS_H_

#include <stdio.h>

#include <s3types.h>

/**  \file gs.h
 * \brief (Not opened to public) Gaussian selector Implentation
 * 
 * Implementation of Bochierri 93's idea of Gaussian Selection. A
 * pre-computed Gaussian selector map (not distributed in Sphinx 3 but
 * in s3fast.tgz distributed in Arthur Chan's web page) encode the
 * closest neighbors of a given codeword and a given GMM.
 *
 * In decoding, the closest codeword of the feature is first found and
 * the nearest neighborhood(s) for all GMMs will be retrieved from the
 * Gaussian selector map. 
 * 
 * The reason why this file is not opened to the public is because
 * sub-vector quantization supersed this idea conceptually.  Although
 * our SVQ implementation do not assume multiple Gaussians to be
 * computed given a sub-stream(sub-vector) codeword is found. It is
 * not difficult to extend the implementation to accomplish that. 
 * 
 * Saying this, for implementation symmetry, we should still check in the 
 * Gaussian selector implementation in this archive. 
 */

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/**
 *  \struct gs_t
 *  Wrapper structure of the gaussian selection algorithm
 */
typedef struct gs_s {
    logmath_t *logmath;
    int32 n_mgau;  /**< number of GMMs */
    int32 n_feat;  /**< number of streams */
    int32 n_code;  /**< number of code word */
    int32 n_density; /**< number of density */
    int32 n_featlen; /**< (This is not consistent to the Gaussian family of function */
    int32 n_mbyte; /**< number of bytes to read each time */
    float32 **codeword; /**< n_code * n_featlen */
    uint32 ***codemap; /**< n_feat * n_mgau * n_code*/
    FILE *fp;       /**< A file pointer to dump the gaussian selector */
    int32* mgau_sl; /**< The short list for how many Gaussians will be computed */
} gs_t;


/** display the Gaussian selector */

int32 gs_display(char *file, /**< file to display*/
		 gs_t *gs    /**< The structure of the gaussian selector*/
    );

/** Read the Gaussian selector */
gs_t* gs_read(const char *file, /**< a file to display */
	      logmath_t *logmath
    );

/** Choose the closet Gaussian codebook to use */
int32 gc_compute_closest_cw ( gs_t *gs, /**< gaussain selector */
			      float32 *feat /**< feature */
    );

/** Find the short list of the Gaussian selector */
int32 gs_mgau_shortlist(gs_t *gs,  /**< gaussain selector */
			int32 m,   /**< mixture index */
			int32 n,   /**< number of mixtures */
			float32 *feat, /**< feature vector */
			int32 bst_codeid /**< best code indx */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
