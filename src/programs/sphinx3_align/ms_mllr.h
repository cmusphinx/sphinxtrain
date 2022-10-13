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
 * ms_mllr.h -- Application of MLLR regression matrices to codebook means
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
 * Revision 1.7  2006/02/22 17:21:35  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Added ms_mllr.[ch]
 *
 * Revision 1.6.4.1  2005/07/20 19:39:01  arthchan2003
 * Added licences in ms_* series of code.
 *
 * Revision 1.6  2005/06/21 18:56:13  arthchan2003
 * 1, Fixed doxygen documentation. 2, Added $ keyword.
 *
 * Revision 1.6  2005/06/19 04:50:02  archan
 * Sphinx3 to s3.generic: allow multiple classes for MLLR
 *
 * Revision 1.5  2005/06/13 04:02:56  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _MS_MLLR_H_
#define _MS_MLLR_H_

#include <s3types.h>

/** \file ms_mllr.h
    \brief (Sphinx 3.0 specific) Multiple stream MLLR.
*/
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/**
 * Load a regression matrix from the given file.  Space for the matrix is allocated
 * by this routine.  (The regression "matrix" is actually a matrix A and a vector B.)
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ms_mllr_read_regmat (const char *regmatfile,	/**< In: File to be read */
			   float32 *****A,		/**< Out: [*A][stream][nclass][streamlen][streamlen] */
			   float32 ****B,		/**< Out: [*B][stream][nclass][streamlen] */
			   float32 ****H,		/**< Out: [*H][stream][nclass][streamlen] */
			   int32 *streamlen,	/**< In: Dimension of each feature stream */
			   int32 n_stream,	/**< In: \#Feature streams */
			   int32 *nclass	/**< Out: number of classes */
    );

/**
 * Free a regression matrix previously read in by mllr_read_regmat.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ms_mllr_free_regmat (float32 ****A,	/**< In: A[stream][nclass][streamlen][streamlen] */
			   float32 ***B,	/**< In: B[stream][nclass][streamlen] */
			   float32 ***H,	/**< In: H[stream][nclass][streamlen] */
			   int32 n_stream	/**< In: \#Feature streams */
    );

/**
 * Transform a mixture density mean matrix according to the given regression matrix.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ms_mllr_norm_mgau (float32 ***mean,	/**<< In/Out: Set of vectors to be transformed:
						   mean[stream][density] */
                         float32 ***mvar,	/**<< In/Out: Set of vectors to be transformed:
						   var[stream][density] */
			 int32 n_density,	/**< In: \#densities in above mean matrix */
			 float32 ****A,	/**< In: "matrix" portion of regression matrix */
			 float32 ***B,	/**< In: "vector" portion of regression matrix */
			 float32 ***H,	/**< In: regression "matrix" for variances */
			 int32 *streamlen,	/**< In: Dimension of each feature stream */
			 int32 n_stream,	/**< In: \#Feature streams */
			 int32 _class	/**< In: number of classes */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
