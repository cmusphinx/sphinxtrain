/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * mllr.h -- Application of MLLR regression matrices to codebook means
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
 * Revision 1.6  2005/06/21 18:50:45  arthchan2003
 * Log. 1, Fixed doxygen documentation, 2, Added $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 *  keyword.
 *
 * Revision 1.5  2005/06/19 19:41:22  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.4  2005/06/13 04:02:55  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 24-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon University
 *              First checked in from sphinx 3.0 to sphinx 3.5
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _MLLR_H_
#define _MLLR_H_


/** \file mllr.h
 * \brief (Sphinx 3.X specific) Single Stream MLLR.
 */
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

#include <s3types.h>
#include "cont_mgau.h"


/** Dump the regression matrix from a given file */
void mllr_dump(float32 ***A, /**< In: [nclass][streamlen][streamlen]*/
	       float32 **B,    /**< In: [nclass][streamlen]*/
	       float32 **H,    /**< In: [nclass][streamlen]*/
	       int32 veclen,  /**< In: vector length*/
	       int32 nclass   /**< In: Number of classes */
    );
/*
 * Load a regression matrix from the given file.  Space for the matrix is allocated
 * by this routine.  (The regression "matrix" is actually a matrix A and a vector B.)
 * @return 0 if successful, -1 otherwise.
 */
int32 mllr_read_regmat (const char *regmatfile,	/**< In: File to be read */
			float32 ****A,		/**< Out: [*A][nclass][streamlen][streamlen] */
			float32 ***B,		/**< Out: [*B][nclass][streamlen] */
			float32 ***H,		/**< Out: [*H][nclass][streamlen] */
			int32 *nclass,		/**< Out: number of classes */
			int32 ceplen);          /**< In: vector length */

/**
 * Free a regression matrix previously read in by mllr_read_regmat.
 * @return 0 if successful, -1 otherwise.
 */
int32 mllr_free_regmat (float32 ***A,		/**< In: A[streamlen][streamlen] */
			float32 **B,		/**< In: B[streamlen] */
			float32 **H		/**< In: H[streamlen] */
    );


/**
 * Transform a mixture density mean matrix according to the given regression matrix.
 * @return 0 if successful, -1 otherwise.
 */

int32 mllr_norm_mgau (mgau_model_t *mgauset, /**< In/Out: The gaussian distribution needs to be transformed */
		      float32 ***A,	/**< In: "matrix" portion of regression matrix */
		      float32 **B,	/**< In: "vector" portion of regression matrix */
		      float32 **H,	/**< In: diagonal "matrix" for variance adaptation */
		      int32 nclass,	/**< In: number of classes */
		      int32 *cb2mllr	/**< In: class to senone mapping */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
