/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: mk_trans_seq.c
 * 
 * Description: 
 * 	This routine generates a sequence of transition matrices and
 *	state pairs given a model definition and state segmentation.
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/mk_trans_seq.h>
#include <s3/s2_param.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <assert.h>

int
mk_trans_seq(uint32 **out_tmat_seq,
	     uint32 **out_mstate_seq,
	     uint16 *seg,
	     uint32 n_frame,
	     acmod_id_t *phone,
	     uint32 n_phone,
	     model_def_t *mdef)
{
    uint32 *tmat_seq;
    uint32 *mstate_seq;
    uint32 f;
    uint32 p;
    model_def_entry_t *defn;
    uint32 n_state = S2_N_STATE-1;	/* BEWARE: this should not stay */
    uint16 seg_val;
    uint32 tmat=0;

    tmat_seq = ckd_calloc(n_frame, sizeof(uint32));
    mstate_seq = ckd_calloc(n_frame, sizeof(uint32));

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

	    tmat = defn->tmat;
	}

	/* model state [0..N-1] where N is # of states/model */
	mstate_seq[f] = seg_val % n_state;
	tmat_seq[f] = tmat;
    }

    *out_tmat_seq = tmat_seq;
    *out_mstate_seq = mstate_seq;

    return S3_SUCCESS;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
