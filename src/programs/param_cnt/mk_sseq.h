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
 * File: mk_sseq.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MK_SSEQ_H
#define MK_SSEQ_H

#include <s3/prim_type.h>
#include <s3/model_def.h>

uint32 *
mk_sseq(uint16 *seg,
	uint32 n_frame,
	acmod_id_t *phone,
	uint32 n_phone,
	model_def_t *mdef);

#endif /* MK_SSEQ_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:32  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/03/25  15:22:04  eht
 * Initial revision
 *
 *
 */
