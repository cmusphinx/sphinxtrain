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
 * File: div.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef DIV_H
#define DIV_H

#include <s3/prim_type.h>

float64
ent_d(float32 *p,
      uint32 n_codeword);

float64
ent_d_state(float32 **p,
	    uint32 n_stream,
	    uint32 n_codeword);

float64
div_d(float32 *p,
      float32 *q,
      uint32 n_codeword);

float64
div_d_state(float32 **p,
	    float32 **q,
	    uint32 n_stream,
	    uint32 n_codeword);

float64
ddiv_d(float32 *p,
       float32 *q,
       uint32 n_codeword);

float64
ddiv_d_state(float32 **p,
	     float32 **q,
	     uint32 n_stream,
	     uint32 n_codeword);

/* ADDITION FOR CONTINUOUS_TREE */
float64
ent_cont(float32 *mean, 
         float32 *var, 
         uint32 n);
/* END ADDITION FOR CONTINUOUS_TREE */

#endif /* DIV_H */ 

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
