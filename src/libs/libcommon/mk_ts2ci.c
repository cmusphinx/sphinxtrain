/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: mk_ts2ci.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/mk_ts2ci.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
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

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.2  96/07/29  16:36:56  eht
 * include header files to define missing prototypes
 * 
 * Revision 1.1  1996/06/17  14:40:41  eht
 * Initial revision
 *
 *
 */
