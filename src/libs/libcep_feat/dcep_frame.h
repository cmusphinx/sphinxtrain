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
 * File: dcep_frame.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef DCEP_FRAME_H
#define DCEP_FRAME_H

#include <s3/prim_type.h>
#include <s3/vector.h>

void
dcep_frame_set_long_window_size(uint32 l_ws);

uint32
dcep_frame_long_window_size(void);

void
dcep_frame_set_short_window_size(uint32 s_ws);

uint32
dcep_frame_short_window_size(void);

void
short_dcep_frame(vector_t dcep,
		 vector_t power,
		 vector_t mfcc);

void
dcep_frame(vector_t dcep,
	   vector_t power,
	   vector_t mfcc);

#endif /* DCEP_FRAME_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1996/01/30  17:09:59  eht
 * Initial revision
 *
 *
 */
