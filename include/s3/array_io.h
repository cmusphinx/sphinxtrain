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
 * File: array_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef ARRAY_IO_H
#define ARRAY_IO_H


#include <s3/prim_type.h>
#include <s3/vector.h>
#include <s3/s3io.h>
#include <s3/s3.h>

#include <stdio.h>


/*
 * Old functionality;  left for file
 *   conversion purposes.
 */
int32
float32_3d_awrite(FILE *fp,
		  float32 ***out,
		  uint32 d1,
		  uint32 d2,
		  uint32 d3);

int32
float32_3d_aread(float32 ****out_arr,
		 uint32 *out_d1,
		 uint32 *out_d2,
		 uint32 *out_d3,
		 FILE *fp);

int32
float32_2d_awrite(FILE *fp,
		  float32 **out,
		  uint32 d1,
		  uint32 d2);

int32
float32_2d_aread(float32 ***out_arr,
		 uint32 *out_d1,
		 uint32 *out_d2,
		 FILE *fp);

int32
float32_feat_awrite(FILE *fp,
		    vector_t **out,
		    uint32 d1,
		    uint32 n_stream,
		    const uint32 *vecsize);

int32
float32_feat_aread(vector_t ***out_arr,
		   uint32 *out_d1,
		   uint32 *out_n_stream,
		   uint32 **out_vecsize,
		   FILE *fp);

#endif /* ARRAY_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.3  96/03/25  15:50:59  eht
 * Added 2d functions
 * 
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
