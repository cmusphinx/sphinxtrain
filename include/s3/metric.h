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
 * File: metric.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef METRIC_H
#define METRIC_H

#include <s3/prim_type.h>

float64
wt_ent_inc(float32 **a,
	   float32 a_dnom,
	   float32 **b,
	   float32 b_dnom,
	   float32 **a_b,
	   uint32 n_stream,
	   uint32 n_cw);


#endif /* METRIC_H */ 


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
