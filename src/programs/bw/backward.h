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
 * File: backward.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef BACKWARD_H
#define BACKWARD_H

#include <s3/prim_type.h>
#include <s3/vector.h>
#include <s3/state.h>
#include <s3/model_inventory.h>

int32
backward_update(float64 **active_alpha,
		uint32 **active_astate,
		uint32 *n_active_astate,
		float64 *scale,
		float64 **dscale,
		vector_t **feature,
		uint32 n_obs,
		float32 ****spkr_xfrm_ainv,
		float32  ***spkr_xfrm_b,
		state_t *state_seq,
		uint32 n_state,
		model_inventory_t *inv,
		float64 beam,
		float32 spthresh,
		int32 mixw_reest,
		int32 tmat_reest,
		int32 mean_reest,
		int32 var_reest,
		int32 pass2var,
		int32 mllr_mult,
		int32 mllr_add);

#endif /* BACKWARD_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.7  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.6  1996/07/29  16:21:12  eht
 * float32 -> float64
 *
 * Revision 1.5  1996/03/26  15:17:51  eht
 * Fix beam definition bug
 *
 * Revision 1.4  1995/11/30  20:52:00  eht
 * Allow tmat_reest to be passed as an argument
 *
 * Revision 1.3  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.2  1995/10/10  12:44:06  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/06/02  20:43:56  eht
 * Initial revision
 *
 *
 */
