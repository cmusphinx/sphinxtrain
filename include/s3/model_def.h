/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: model_def.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef MODEL_DEF_H
#define MODEL_DEF_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>
#include <s3/acmod_set.h>

/*
 * This structure defines the tied transitions
 * and tied states for a phone in the system
 */
typedef struct model_def_entry_s {
    acmod_id_t p;
    uint32 tmat;
    uint32 *state;
    uint32 n_state;
    char **attrib;	     /* the attribute list for the
			      * phone (e.g. non-speech) */
} model_def_entry_t;

/*
 * This structure contains the definitions for all
 * phones and some parameters about the definitions.
 */
typedef struct model_def_s {
    acmod_set_t *acmod_set;	/* Data for names <-> id mapping, etc. */
    model_def_entry_t *defn;
    uint32 n_defn;

    uint32 n_total_state;

    uint32 n_tied_ci_state;
    uint32 n_tied_state;
    uint32 n_tied_tmat;

    uint32 *cb;		/* tied state -> codebook ID mapping */
    uint32 n_cb;	/* The # of distinct codebook ID's */

    uint32 max_n_state;	/* max # of states over all models */
    uint32 min_n_state;	/* min # of states over all models */

    acmod_id_t *ts2ci;	/* maps any tied state to ci model ID */
} model_def_t;

#ifdef __cplusplus
}
#endif
#endif /* MODEL_DEF_H */ 
