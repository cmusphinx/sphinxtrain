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
 * File: metric.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/metric.h>
#include <s3/div.h>

float64
wt_ent_inc(float32 **a,
	   float32 a_dnom,
	   float32 **b,
	   float32 b_dnom,
	   float32 **a_b,
	   uint32 n_stream,
	   uint32 n_density)
{
    return
	a_dnom * ddiv_d_state(a, a_b, n_stream, n_density) +
	b_dnom * ddiv_d_state(b, a_b, n_stream, n_density);
	
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
