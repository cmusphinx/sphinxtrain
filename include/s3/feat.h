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
 * File: feat.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef FEAT_H
#define FEAT_H

#include <s3/vector.h>
#include <s3/prim_type.h>

#define FEAT_ID_SPHINX_II_STD	0
#define FEAT_ID_V1		1
#define FEAT_ID_V2		2
#define FEAT_ID_V3		3
#define FEAT_ID_V4		4
#define FEAT_ID_V5		5
#define FEAT_ID_V6		6
#define FEAT_ID_V7		7
#define FEAT_ID_V8		8
#define FEAT_ID_MAX		9
#define FEAT_ID_NONE		0xffffffff

typedef struct {
    void (*set_in_veclen)(uint32 len);
    const char * (*doc)(void);
    uint32 (*id)(void);
    uint32 (*n_stream)(void);
    uint32 (*blksize)(void);
    const uint32 * (*vecsize)(void);
    vector_t ** (*alloc)(uint32 n_frame);
    void (*free)(vector_t **f);
    vector_t ** (*compute)(vector_t *mfcc,
			   uint32 *inout_n_frame);
    void (*print)(const char *label,
		  vector_t **f,
		  uint32 n_frame);
} feat_conf_t;

int
feat_set(const char *id_name);

uint32
feat_id(void);

const char *
feat_doc(void);

void
feat_set_in_veclen(uint32 len);

uint32
feat_mfcc_len(void);

uint32
feat_n_stream(void);

const uint32 *
feat_vecsize(void);

uint32
feat_blksize(void);

int
feat_ck_vecsize(const char *tag,
		const uint32 *vecsize,
		uint32 n_stream);

vector_t **
feat_compute(vector_t *mfcc,
	     uint32 *inout_n_frame);

void
feat_print(const char *label,
	   vector_t **feat,
	   uint32 n_frame);

void
feat_print_mfcc(vector_t *mfcc,
		uint32 n_frame);

vector_t **
feat_alloc(uint32 n_frames);

void
feat_free(vector_t **f);

#define FEAT_NO_SIZE	0xffffffff

#endif /* FEAT_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.7  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.6  1996/08/05  13:00:10  eht
 * - Begin feat control block type of structure
 * - Add silence deletion function
 *
 * Revision 1.5  1996/03/25  15:46:57  eht
 * Added ability to set input feature vector size
 * changed feat_comp_id() to feat_id()
 *
 * Revision 1.4  1996/01/26  18:33:28  eht
 * *** empty log message ***
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
