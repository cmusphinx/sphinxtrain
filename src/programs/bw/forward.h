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
 * File: forward.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef FORWARD_H
#define FORWARD_H

#include <s3/state.h>
#include <s3/prim_type.h>
#include <s3/model_inventory.h>
#include <s3/vector.h>

uint32 *
backtrace(state_t *state, uint32 fs_id, uint32 *n_vit_sseq);

int32
forward(float64 **active_alpha,
	uint32 **active_astate,
	uint32 *n_active_astate,
	float64 *scale,
	float64 **dscale,
	vector_t **feature,
	uint32 n_obs,
	state_t *state_seq,
	uint32 n_state,
	model_inventory_t *inv,
	float64 beam);

void
forward_set_viterbi(int state);

#endif /* FORWARD_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.6  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.5  96/07/29  16:21:12  eht
 * float32 -> float64
 * 
 * Revision 1.4  1996/03/26  15:17:51  eht
 * Fix beam definition bug
 *
 * Revision 1.3  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.2  1995/10/10  12:44:06  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/06/02  20:41:22  eht
 * Initial revision
 *
 *
 */
