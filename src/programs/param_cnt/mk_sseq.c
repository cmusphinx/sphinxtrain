/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: mk_sseq.c
 * 
 * Description: 
 * 	Given a state seg sequence and phone sequence, find out what
 *	the senone sequence is for the whole utterance.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "mk_sseq.h"

#include <s3/ckd_alloc.h>
#include <s3/s2_param.h>

#include <assert.h>

uint32 *
mk_sseq(uint16 *seg,
	uint32 n_frame,
	acmod_id_t *phone,
	uint32 n_phone,
	model_def_t *mdef)
{
    uint32 *sseq;
    uint32 f;
    uint32 p;
    model_def_entry_t *defn=0;
    uint32 s;
    uint32 n_state = S2_N_STATE-1;	/* BEWARE: this should not stay */
    uint16 seg_val;

    sseq = ckd_calloc(n_frame, sizeof(uint32));

    p = 0;
    --p;

    for (f = 0; f < n_frame; f++) {
	seg_val = seg[f];

	if (seg_val & 0x8000) {
	    /* reached a phone begin marker */

	    ++p;	/* next phone */

	    assert(p < n_phone);

	    defn = &mdef->defn[phone[p]];

	    seg_val &= 0x7fff;	/* clr phone begin bit */
	}

	s = seg_val % n_state;	/* model state [0..N-1] where N is # of states/model */

	sseq[f] = defn->state[s];
    }

    return sseq;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:32  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/03/25  15:21:20  eht
 * Initial revision
 *
 *
 */
