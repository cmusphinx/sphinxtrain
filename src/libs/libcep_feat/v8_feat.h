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
 * File: v8_feat.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef V8_FEAT_H
#define V8_FEAT_H


#include <s3/vector.h>

const char *
v8_feat_doc(void);

uint32
v8_feat_id(void);

uint32
v8_feat_n_stream(void);

uint32
v8_feat_blksize(void);

const uint32 *
v8_feat_vecsize(void);

void
v8_feat_set_in_veclen(uint32 len);

vector_t **
v8_feat_alloc(uint32 n_frames);

void
v8_feat_free(vector_t **f);

void
deltacep_frame(vector_t dcep,
               vector_t mfcc);

void
doubledelta_frame(vector_t dcep,
                  vector_t mfcc);


vector_t **
v8_feat_compute(vector_t *mfcc,
		uint32 *inout_n_frame);

void
v8_feat_print(const char *label,
	      vector_t **f,
	      uint32 n_frames);

#endif /* V8_FEAT_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.2  1996/03/25  15:36:54  eht
 * Allow for settable input feature length
 *
 * Revision 1.1  1996/01/30  17:09:59  eht
 * Initial revision
 *
 *
 */
