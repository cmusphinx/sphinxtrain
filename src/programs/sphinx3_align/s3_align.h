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
 * align.h -- Exported time-aligner functions and data structures.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.9  2006/03/28  04:50:14  dhdfu
 * Add an option to control the insertion of optional silences and filler
 * words (the TTS people may want to use this, and I need it)
 * 
 * Revision 1.8  2006/02/24 18:30:20  arthchan2003
 * Changed back s3senid to int32.  Don't know the reason why using s3senid_t will cause failure in test. Need to talk with Dave.
 *
 * Revision 1.7  2006/02/24 16:42:48  arthchan2003
 * Fixed function prototype for align_sen_active
 *
 * Revision 1.6  2006/02/24 04:42:32  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Fixed dox-doc.
 *
 * Revision 1.5.4.2  2005/08/02 21:42:34  arthchan2003
 * 1, Moved static variables from function level to the application level. 2, united all initialization of HMM using s3_am_init, 3 united all GMM computation using ms_cont_mgau_frame_eval.
 *
 * Revision 1.5.4.1  2005/07/22 03:46:56  arthchan2003
 * 1, cleaned up the code, 2, fixed dox-doc. 3, use srch.c version of log_hypstr and log_hyp_detailed.
 *
 * Revision 1.5  2005/06/22 05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:01  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.4  2004/12/06 11:31:48  arthchan2003
 * Fix brief comments for programs.
 *
 * Revision 1.3  2004/12/05 12:01:32  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.2  2004/09/13 08:13:28  arthchan2003
 * update copyright notice from 200x to 2004
 *
 * Revision 1.1  2004/08/30 22:29:19  arthchan2003
 * Refactor the s3.0 tools, currently it is still quite messy, we need to make it modularize later on.
 *
 * Revision 1.1  2004/08/09 00:17:12  arthchan2003
 * Incorporating s3.0 align, at this point, there are still some small problems in align but they don't hurt. For example, the score doesn't match with s3.0 and the output will have problem if files are piped to /dev/null/. I think we can go for it.
 *
 * Revision 1.1  2003/02/14 14:40:34  cbq
 * Compiles.  Analysis is probably hosed.
 *
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * 
 * 13-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed align_sen_active to flag active senones instead of building a list
 * 		of them.
 * 
 * 15-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFBS_ALIGN_H_
#define _LIBFBS_ALIGN_H_

/** \file s3_align.h
    \brief data structure for alignment
 */
#include <sphinxbase/logmath.h>

#include <s3types.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

/** State level segmentation/alignment; one entry per frame */
typedef struct align_stseg_s {
    s3pid_t pid;                /**< Phone id */
    s3senid_t sen;              /**< Senone id */
    int8 state;                 /**< State id within phone */
    int8 start;                 /**< Whether this is an entry into phone start state */
    int32 score;                /**< Acoustic score for transition to this state from the
				   previous one in the list */
    int32 bsdiff;               /**< Distance between this state and the best in this frame */
    struct align_stseg_s *next; /**< Next state (in the next frame) in alignment */
} align_stseg_t;


/** Phone level segmentation/alignment information */
typedef struct align_phseg_s {
    s3pid_t pid;                /**< Phone id */
    s3frmid_t sf, ef;           /**< Start and end frame for this phone occurrence */
    int32 score;                /**< Acoustic score for this segment of alignment */
    int32 bsdiff;               /**< Distance between this and the best unconstrained state
				   sequence for the same time segment */
    struct align_phseg_s *next; /**< Next entry in alignment */
} align_phseg_t;


/** Word level segmentation/alignment information */
typedef struct align_wdseg_s {
    s3wid_t wid;                /**< Word id */
    s3frmid_t sf, ef;           /**< Start and end frame for this word occurrence */
    int32 score;                /**< Acoustic score for this segment of alignment */
    int32 bsdiff;               /**< Distance between this and the best unconstrained state
				   sequence for the same time segment */
    struct align_wdseg_s *next; /**< Next entry in alignment */
} align_wdseg_t;


int32 align_init(mdef_t * _mdef, tmat_t * _tmat, dict_t * _dict, cmd_ln_t *_config, logmath_t *_logmath);

void align_free(void);

int32 align_build_sent_hmm(char *transcript,  /**< In: Word transcript */
                           int insert_sil     /**< In: Whether to insert silences/fillers */
    );

int32 align_destroy_sent_hmm(void);

int32 align_start_utt(char *uttid);

/**
 * Called at the beginning of a frame to flag the active senones (any senone used
 * by active HMMs) in that frame.
 */
void align_sen_active(uint8 * senlist,  /**< Out: senlist[s] TRUE iff active in frame */
                      int32 n_sen               /**< In: Size of senlist[] array */
    );


/** Step time aligner one frame forward */
int32 align_frame(int32 * senscr                /**< In: array of senone scores this frame */
    );


/**<
 * Wind up utterance and return final result (READ-ONLY).  Results only valid until
 * the next utterance is begun.
 */
int32 align_end_utt(align_stseg_t ** stseg,     /**< Out: list of state segmentation */
                    align_phseg_t ** phseg,     /**< Out: list of phone segmentation */
                    align_wdseg_t ** wdseg      /**< Out: list of word segmentation */
    );

#ifdef __cplusplus
}
#endif


#endif
