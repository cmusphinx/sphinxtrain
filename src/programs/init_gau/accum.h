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
 * File: accum.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef ACCUM_H
#define ACCUM_H

#include <s3/vector.h>
#include <s3/prim_type.h>

int
accum_state_mean(vector_t ***mean_acc,
		 float32  ***dnom,
		 vector_t **f,
		 uint32 *del_b,
		 uint32 *del_e,
		 uint32 n_del,
		 uint32 n_feat,
		 const uint32 *veclen,
		 uint32 *sseq,
		 uint32 *ci_sseq,
		 uint32 n_frame);

int
accum_state_var(vector_t ***var_acc,
		vector_t ***mean,
		float32  ***dnom,
		vector_t **f,
		uint32 *del_b,
		uint32 *del_e,
		uint32 n_del,
		uint32 n_feat,
		const uint32 *veclen,
		uint32 *sseq,
		uint32 *ci_sseq,
		uint32 n_frame);

#endif /* ACCUM_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1996/08/06  14:13:23  eht
 * added "const" attribute to veclen arguments
 *
 * Revision 1.2  1996/02/02  17:35:39  eht
 * formatting change
 *
 * Revision 1.1  1996/01/30  17:11:47  eht
 * Initial revision
 *
 *
 */
