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
 * File: viterbi.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef VITERBI_H
#define VITERBI_H

#include <s3/prim_type.h>
#include <s3/vector.h>
#include <s3/state.h>
#include <s3/model_inventory.h>

int32
viterbi_update(float64 *log_forw_prob,
	       vector_t **feature,
	       uint32 *sseq,
	       uint32 *t_seq,
	       uint32 *ms_sseq,
	       uint32 n_obs,
	       model_inventory_t *inv,
	       int32 mixw_reest,
	       int32 tmat_reest,
	       int32 mean_reest,
	       int32 var_reest,
	       int32 pass2var);

#endif /* VITERBI_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/07/29  16:22:23  eht
 * Initial revision
 *
 *
 */
