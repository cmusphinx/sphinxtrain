/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.  
 *   All rights reserved.
 *
 *********************************************************************
 *
 * File: v6_feat.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef V6_FEAT_H
#define V6_FEAT_H


#include <s3/vector.h>

const char *
v6_feat_doc(void);

uint32
v6_feat_id(void);

uint32
v6_feat_n_stream(void);

uint32
v6_feat_blksize(void);

const uint32 *
v6_feat_vecsize(void);

void
v6_feat_set_in_veclen(uint32 len);

vector_t **
v6_feat_alloc(uint32 n_frames);

void
v6_feat_free(vector_t **f);

vector_t **
v6_feat_compute(vector_t *mfcc,
		uint32 *inout_n_frame);

void
v6_feat_print(const char *label,
	      vector_t **f,
	      uint32 n_frames);

#endif /* V6_FEAT_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:38:16  eht
 * Initial revision
 * 
 *
 */
