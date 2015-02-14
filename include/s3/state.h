/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * File: state.h
 * 
 * Description: 
 *	This file defines the model state data structure.  The state
 *	data structure contains indices of model parameters associated
 *	with the state and it contains adjacency lists of prior states
 *	and next states.
 *
 *	The states defined in this structure are in the context of
 *	an utterance HMM built to model a whole utterance.  Hence it
 *	is built as a network of subword HMM's.
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef STATE_H
#define STATE_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>

/* The state ID representing "No state" */
#define NO_STATE	0xffffffff

typedef struct state_s {
    /* information necessary to compute output probability of
       an observation given this state */
    uint32 mixw;	/* mixing weight id for this state */
    uint32 cb;		/* codebook (i.e. Gaussian density family) for this state */
    uint32 ci_cb;	/* CI codebook */
    uint32 ci_mixw;	/* CI mixing weight id for this state */

    /* list of prior state indices and
       a list of tprobs to go from each prior state to this state */
    uint32 n_prior;
    uint32 *prior_state;
    float32 *prior_tprob;

    /* list of next state indices and
       a list of tprobs to go from each next state to this state */
    uint32 n_next;
    uint32 *next_state;
    float32 *next_tprob;

    uint32 tmat;	/* model transition matrix index */
    uint32 m_state;	/* The model state associated with this utterance
			   HMM. */

    uint32 l_mixw;	/* local mixing weight accumulator ID */
    uint32 l_cb;	/* local codebook accumulator ID */
    uint32 l_ci_mixw;	/* local CI mixing weight accumulator ID */
    uint32 l_ci_cb;	/* local CI codebook accumulator ID */
    float32 *tacc;	/* local transition observation count accumulators */

    uint32 phn;		/* CI phone ID (for constrained Baum-Welch) */
} state_t;

#ifdef __cplusplus
}
#endif
#endif /* STATE_H */ 

