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
 * File: s2_feat.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S2_FEAT_H
#define S2_FEAT_H

#include <s3/vector.h>
#include <s3/prim_type.h>

uint32
s2_feat_n_stream(void);

const uint32 *
s2_feat_vecsize(void);

uint32
s2_feat_blksize(void);

uint32
s2_feat_id(void);

const char *
s2_feat_doc(void);

void
s2_feat_set_in_veclen(uint32 len);

vector_t **
s2_feat_compute(vector_t *mfcc,
		uint32 *inout_n_frame);

vector_t **
s2_feat_alloc(uint32 n_frames);

void
s2_feat_free(vector_t **f);

void
s2_feat_print(const char *label,
	      vector_t **f,
	      uint32 n_frames);


#endif /* S2_FEAT_H */ 

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
 * Revision 1.3  1996/03/25  15:36:54  eht
 * Allow for settable input feature length
 *
 * Revision 1.2  1996/01/30  17:09:59  eht
 * *** empty log message ***
 *
 * Revision 1.1  1995/12/07  20:17:46  eht
 * Initial revision
 *
 * Revision 1.1  1995/12/07  19:30:13  eht
 * Initial revision
 *
 * Revision 1.3  1995/12/04  15:00:27  eht
 * Added prototype for feat_n_stream()
 *
 * Revision 1.2  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
