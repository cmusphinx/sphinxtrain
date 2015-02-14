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
 * File: mk_ts2ci.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <s3/mk_ts2ci.h>
#include <s3/s3.h>

#include <assert.h>

uint32 *
mk_ts2ci(model_def_t *mdef)
{
    uint32 i, j;
    uint32 *map;
    uint32 ts;
    acmod_set_t *acmod_set = mdef->acmod_set;

    map = (uint32 *)ckd_calloc(mdef->n_tied_state, sizeof(uint32));

    for (i = 0; i < mdef->n_tied_state; i++)
	map[i] = TYING_NO_ID;

    for (i = 0; i < mdef->n_defn; i++) {
	for (j = 0; j < mdef->defn[i].n_state; j++) {
	    ts = mdef->defn[i].state[j];
	    if ((ts != TYING_NON_EMITTING) && (map[ts] == TYING_NO_ID)) {
		assert(ts < mdef->n_tied_state);

		map[ts] = acmod_set_base_phone(acmod_set, i);
	    }
	}
    }

    for (i = 0; i < mdef->n_tied_state; i++) {
	if (map[i] == TYING_NO_ID) {
	    E_WARN("state %d has no mapping to a CI phone\n", i);
	}
    }

    return map;
}

