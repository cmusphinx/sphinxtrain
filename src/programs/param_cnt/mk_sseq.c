/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * Revision 1.2  2000/09/29  22:35:14  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:32  awb
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
