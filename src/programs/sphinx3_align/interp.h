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
 * interp.h -- CD-senone and CI-senone score interpolation
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
 * Revision 1.6  2006/02/22 16:48:30  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed issues in , 2, Fixed issues of dox-doc
 *
 * Revision 1.5.4.1  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 * Revision 1.5  2005/06/21 18:39:04  arthchan2003
 * Log. 1, Fixed doxygen documentation, 2, Added $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Log. 1, Fixed doxygen documentation, 2, Added Revision 1.6  2006/02/22 16:48:30  arthchan2003
 * Log. 1, Fixed doxygen documentation, 2, Added Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed issues in , 2, Fixed issues of dox-doc
 * Log. 1, Fixed doxygen documentation, 2, Added
 *
 * Revision 1.4  2005/06/13 04:02:55  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 05-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFBS_INTERP_H_
#define _LIBFBS_INTERP_H_


#include <s3types.h>

/** \file interp.h \brief Interpolation of CD/CI senones.
 * Interpolation is used in the time of SCHMM and it is still a very
 * useful technique for CDHMM as some experiments shown in Broadcast
 * news evaluation in 99. 
 */
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/**
 * \struct interp_t
 * \brief Wrapper structure of CD/CI interpolation
 */
typedef struct {
    logmath_t *logmath;
    int32 n_sen;	/**< \#senones */

    /**
     * \struct interp_wt_s
     * \brief Quick and Dirty wrapper of the CD and CI weight 
     */

    struct interp_wt_s {
        int32 cd;	/**< logs3(CD senone weight) */
        int32 ci;	/**< logs3(1 - cd) */
    } *wt;		/**< wt[i] = interpolation weight for senone i */
} interp_t;


/**
 * Read a set of CD/CI senone interpolation weights from the given file.
 * Return value: pointer to interpolation structure created.  Caller MUST NOT change its
 * contents.
 * @return an initialized interp_t structure 
 */
interp_t *interp_init (const char *interpfile,	/* In: interpolation weights file */
		       logmath_t *logmath
    );

/**
 * Interpolate a single given CD senone with the given CI senone score.
 * @return 0 if successful, -1 otherwise.
 */
int32 interp_cd_ci (interp_t *ip,	/**< In: Interpolation weights parameters */
		    int32 *senscr,	/**< In/Out: senscr[cd] interpolated with senscr[ci] */
		    int32 cd,		/**< In: see senscr above */
		    int32 ci		/**< In: see senscr above */
    );

/**
 * Interpolate each CD senone with its corresponding CI senone score.
 * @return 0 if successful, -1 otherwise.
 */
int32 interp_all (interp_t *ip,		/**< In: Interpolation weights parameters */
		  int32 *senscr,	/**< In/Out: senscr[cd] interpolated with
					   senscr[cimap[cd]], for cd >= n_ci_sen */
		  s3senid_t *cimap,	/**< In: see senscr above */
		  int32 n_ci_sen	/**< In: see senscr above */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
