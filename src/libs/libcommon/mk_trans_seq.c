/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * Revision 1.2  2000/09/29  22:35:13  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
