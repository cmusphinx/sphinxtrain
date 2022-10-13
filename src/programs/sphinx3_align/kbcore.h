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
 * kbcore.h -- Structures for maintain the main models.
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
 * Revision 1.2  2006/04/06  14:03:02  dhdfu
 * Prevent confusion among future generations by calling this s2_semi_mgau instead of sc_vq
 * 
 * Revision 1.1  2006/04/05 20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 *
 * Revision 1.12  2006/02/23 05:54:58  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added linksilences.  This allows silences to be linked correctly in mode FLAT, TREE.
 * 2, Added s3_am_init: an all-in-one initalization routine SCHMM and CDHMM.
 *
 * Revision 1.11.4.5  2006/01/16 18:25:16  arthchan2003
 * Sphinx 3.x tree decoders assume silences are unlinked (set them to BAD_S3WID) before used. Whereas the flat lexicon decode doesn't have this assumption.  The changes in versions this branch also significantly changed behavior of the decoder. Thus the function LinkSilences is introduced to change back the behavior if necessary.
 *
 * Revision 1.11.4.4  2005/09/18 01:29:37  arthchan2003
 * 1, .s3cont. mode is supported.  When it is specified by -senmgau, it will invoke the MS version of GMM computation even for CDHMM. Not supposed to be documented for users. 2, Remove unlinkSilences and put it inside search-specific initialization.  Apparently, remove it entirely will screw up the current test of mode 4 and 5.  add it back will screw up mode 3.  That's why I used temp solution.
 *
 * Revision 1.11.4.3  2005/08/03 18:54:32  dhdfu
 * Fix the support for multi-stream / semi-continuous models.  It is
 * still kind of a hack, but it now works.
 *
 * Revision 1.11.4.2  2005/08/02 21:33:47  arthchan2003
 * Factored the code of initializing one hmm into s3_am_init. That is to say initialization of mdef, mgau, var, mixw and tmat could all be found one function.
 *
 * Revision 1.11.4.1  2005/07/20 21:19:52  arthchan2003
 * Added options such that finite state grammar option is now accepted.
 *
 * Revision 1.11  2005/06/21 23:28:48  arthchan2003
 * Log. Please also see comments of kb.[ch].  Major changes you could see
 * is that the lmset interface is now used rather than several interfaces
 * for reading lm. Other than that, you could say most changes are
 * harmless internal interfaces changes.
 *
 * Revision 1.5  2005/06/18 03:22:29  archan
 * Add lmset_init. A wrapper function of various LM initialization and initialize an lmset It is now used in decode, livepretend, dag and astar.
 *
 * Revision 1.4  2005/04/20 03:38:43  archan
 * Do the corresponding code changes for the lm code.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 11-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Removed svqpp stuff.  It doesn't work too well anyway.
 * 
 * 06-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added kb_t.svqpp_t and related handling.
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _S3_KBCORE_H_
#define _S3_KBCORE_H_

#include <sphinxbase/feat.h>

#include <s3types.h>

#include "cont_mgau.h"
#include "ms_mgau.h"
#include "s2_semi_mgau.h"
#include "mdef.h"
#include "dict.h"
#include "dict2pid.h"
#include "fillpen.h"
#include "tmat.h"
#include "subvq.h"
#include "gs.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

typedef struct lmset_s lmset_t;
typedef struct lm_s lm_t;

/** \file kbcore.h
 * \brief kb core structures, the structure that stores parameters for s3.X search
 */

typedef struct {
    cmd_ln_t *config; /**< The command-line or configuration object */
    feat_t *fcb; /**< feature end structure */
    mdef_t *mdef; /**< Model definition  */
    dict_t *dict; /**< Dictionary structure */
    dict2pid_t *dict2pid; /**< Conversion of dictionary to Phoneme ID */

    lmset_t *lmset; /**< LM Set. ARCHAN, since sphinx 3.6, it is used whenever an lm is allocated. 
                       This unified the internal data structure. */

    /*Specified either one of them when using kbcore.h.  It is not yet very nice now. */
    mgau_model_t *mgau; /**< Acoustic Model for single stream */
    ms_mgau_model_t *ms_mgau; /**< Acoustic Model for multipel stream */
    s2_semi_mgau_t *s2_mgau; /**< Acoustic model for Sphinx2 semi-continuous */

    fillpen_t *fillpen; /**< Filler penalty */
    subvq_t *svq; /**< SVQ */
    gs_t *gs; /**< Gaussian Selector */
    tmat_t *tmat; /**< Transition Matrix. */

    int32 maxNewHeurScore; /**< Temporary variables for phoneme lookahead. This stores the heuristic score */
    int32 lastfrm; /**, Temporary variables, should be removed */

    s3lmwid32_t startwid;
    s3lmwid32_t finishwid;
    logmath_t *logmath;
} kbcore_t;

  
/**
   Create a new kbcore 
*/
S3DECODER_EXPORT
kbcore_t *New_kbcore(cmd_ln_t *config);

/**
   Initialize just the acoustic model for kbcore, taking parameters
   from the global command-line module.
*/
S3DECODER_EXPORT
void s3_am_init(kbcore_t *kbc);


/**
 * Initialize one or more of all the major models:  pronunciation dictionary, acoustic models,
 * language models.  Parameters are taken from the command line (see cmdln_macro.h)
 */
kbcore_t *kbcore_init(cmd_ln_t *config);

/** free the kbcore */
S3DECODER_EXPORT
void kbcore_free (kbcore_t *kbcore  /**< The kbcore structure */
    );

/**
   Sphinx 3.x tree decoders assume silences are unlinked (set them
   to BAD_S3WID) before used. Whereas the flat lexicon decoder
   doesn't have such assumption.  These two functions change this
   behavior.  Called in mode 3, 4 and 5 to make sure different code
   works.  FIXME: This is dumb.
*/
void unlinksilences(lm_t* l, kbcore_t *kbc, dict_t *d);

void linksilences(lm_t* l, kbcore_t *kbc, dict_t *d);

/** Access macros; not meant for arbitrary use */
#define kbcore_config(k)	((k)->config)
#define kbcore_fcb(k)		((k)->fcb)
#define kbcore_mdef(k)		((k)->mdef)
#define kbcore_dict(k)		((k)->dict)
#define kbcore_dict2pid(k)	((k)->dict2pid)
#define kbcore_lm(k)		((k)->lmset ? (k)->lmset->cur_lm : NULL)
#define kbcore_fillpen(k)	((k)->fillpen)
#define kbcore_dict2lmwid(k,w)	((k)->dict2lmwid[w])
#define kbcore_mgau(k)		((k)->mgau)
#define kbcore_ms_mgau(k)	((k)->ms_mgau)
#define kbcore_s2_mgau(k)	((k)->s2_mgau)
#define kbcore_svq(k)		((k)->svq)
#define kbcore_gs(k)		((k)->gs)
#define kbcore_tmat(k)		((k)->tmat)
#define kbcore_lmset(k)		((k)->lmset)
/*#define kbcore_n_mgau(k)	((k)->mgau ? mgau_n_mgau((k)->mgau) : (k)->ms_mgau->s->n_sen)
 */
#define kbcore_n_mgau(k)	((k)->mgau ? mgau_n_mgau((k)->mgau) \
                                : ((k)->s2_mgau ? (k)->s2_mgau->n_sen \
                                   : (k)->ms_mgau->s->n_sen))
#define kbcore_logmath(k)	((k)->logmath)



#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
