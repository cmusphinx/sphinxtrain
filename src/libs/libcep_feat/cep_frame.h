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
 * File: cep_frame.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef CEP_FRAME_H
#define CEP_FRAME_H

#include <s3/vector.h>
#include <s3/prim_type.h>

void
cep_frame_set_size(uint32 sz);

void
cep_frame(vector_t cep,
	  vector_t pow,
	  vector_t mfcc);

#endif /* CEP_FRAME_H */ 


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
