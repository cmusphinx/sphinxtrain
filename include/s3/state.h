/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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

#include <s3/prim_type.h>

/* The state ID representing "No state" */
#define NO_STATE	0xffffffff

typedef struct {
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

    uint32 bp;		/* backpointer index (for Viterbi algorithm) */
} state_t;

#endif /* STATE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/17  07:04:53  eht
 * Added CI codebook fields
 * 
 * Revision 1.3  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.2  97/01/07  11:50:00  eht
 * Collapsed mean/var fields into one field.
 * 
 * Revision 1.1  1996/06/17  14:51:49  eht
 * Initial revision
 *
 * Revision 1.3  1996/02/02  17:43:06  eht
 * Deal w/ CI mean and var as well.
 *
 * Revision 1.2  1996/01/30  17:12:27  eht
 * Include the mixture Gaussian index of the CI m. Gau. associated
 * with each state.
 *
 * Revision 1.1  1995/10/23  20:52:51  eht
 * Initial revision
 *
 *
 */
