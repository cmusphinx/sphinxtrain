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
 * adaptor.h -- Wrapper for structures of adaptation. 
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
 * Revision 1.3  2006/02/22 14:48:13  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Added adapt_set_mllr (Moved from kb)
 *
 *
 * Revision 1.1.4.3  2005/09/18 01:12:31  arthchan2003
 * Add adapt_set_mllr which is responsible for single stream MLLR switching.
 *
 * Revision 1.1.4.2  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 * Revision 1.1.4.1  2005/06/27 05:22:19  arthchan2003
 * Merged from the HEAD.
 *
 * Revision 1.2  2005/06/22 08:00:07  arthchan2003
 * Completed all doxygen documentation on file description for libs3decoder/libutil/libs3audio and programs.
 * Revision 1.2  2005/06/22 08:00:07  arthchan2003
 * Completed all doxygen documentation on file description for libs3decoder/libutil/libs3audio and programs.
 *
 * Revision 1.1  2005/06/21 17:59:44  arthchan2003
 * Log: Implementation and Interface of adapt_am_t, a wrapper of
 * adaptation capabability of Sphinx 3.  It takes the responsibility from
 * kb_t to manage regA, regB and mllr_nclass.  Interfaces are not fully
 * completed. So "pointer" symtom code still appears in kb.c
 *
 * Revision 1.3  2005/06/19 19:41:21  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.2  2005/03/30 01:08:37  archan
 * codebase-wide update. Performed an age-old trick: Adding $Log into all .c and .h files. This will make sure the commit message be preprended into a file.
 *
 * 
 * 25-Mar-2005  Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First started. 
 */

/** \file adaptor.h
 * \brief Wrapper function of speaker adaptation.
 */


#ifndef _ADAPT_H_
#define _ADAPT_H_

#include <s3types.h>
#include <cont_mgau.h>
#include <mdef.h>

#define ADAPTATION_MLLR 0

#if 0 /* for forward compatibility */
#define ADAPTATION_MAP 1
#define ADAPTATION_PLSA 2
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** variables for acoustic model adaptation.  Currently, this really means MLLR */

/** \struct adapt_am_t
 * \brief A wrapper for all adaptation operations in Sphinx 3.X including MLLR and MAP. 
 */
typedef struct {
    char* prevmllrfn; /**< Last MLLR matrix files */
#if 0
    float32** regA; /**< Regression matrix : the multiplcation term */
    float32* regB; /**< Regression matrix : the bias term */
#endif

    float32*** regA; /** Regression matrices : the multiplcation term */
    float32** regB; /** Regression matrices : the bias term */
    float32** regH; /** Regression matrices : scale factor for variance */
    int32 mllr_nclass; /** Number of regression matrices */
} adapt_am_t ;

/** Initialize the adaptor structure */
S3DECODER_EXPORT
adapt_am_t* adapt_am_init(void);
  
/** free adaptor structure */
S3DECODER_EXPORT
void adapt_am_free(adapt_am_t *ad);

S3DECODER_EXPORT
void adapt_set_mllr(adapt_am_t *ad, /**< The adaptor object */
                    mgau_model_t *g,  /**< Gaussian model object */
                    const char *mllrfile,  /**< MLLR file name */
                    const char* cbs2mllr,  /**< CB2MLLR */
                    mdef_t *mdef, /**< MDEF */
                    cmd_ln_t *config
    );
#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus


}
#endif

#endif
