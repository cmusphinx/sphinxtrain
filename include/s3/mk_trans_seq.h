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
 * File: mk_trans_seq.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MK_TRANS_SEQ_H
#define MK_TRANS_SEQ_H

#include <s3/prim_type.h>
#include <s3/model_def.h>

int
mk_trans_seq(uint32 **out_tmat_seq,
	     uint32 **out_mstate_seq,
	     uint16 *seg,
	     uint32 n_frame,
	     acmod_id_t *phone,
	     uint32 n_phone,
	     model_def_t *mdef);



#endif /* MK_TRANS_SEQ_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:39:10  eht
 * Initial revision
 * 
 *
 */
