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
 * File: two_class.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef TWO_CLASS_H
#define TWO_CLASS_H

#include <s3/prim_type.h>

float64
two_class(float32 ****mixw_occ,
/* ADDITION FOR CONTINUOUS_TREES */
          float32 ****means,
          float32 ****vars,
          uint32 *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
          uint32 n_dist,
          uint32 n_state,
          uint32 n_stream,
          uint32 n_density,
          float32 *stwt,
          uint32 *bclust,
          float32 mwfloor);

#endif /* TWO_CLASS_H */ 


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
