/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: matrix.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <s3/prim_type.h>

void
norm_3d(float32 ***arr,
	uint32 d1,
	uint32 d2,
	uint32 d3);

void
accum_3d(float32 ***out,
	 float32 ***in,
	 uint32 d1,
	 uint32 d2,
	 uint32 d3);

void
floor_3d(float32 ***m,
	 uint32 d1,
	 uint32 d2,
	 uint32 d3,
	 float32 floor);

void
floor_nz_3d(float32 ***m,
	    uint32 d1,
	    uint32 d2,
	    uint32 d3,
	    float32 floor);

void
floor_nz_1d(float32 *v,
	    uint32 d1,
	    float32 floor);

void
band_nz_1d(float32 *v,
	   uint32 d1,
	   float32 band);

#endif /* MATRIX_H */ 


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
