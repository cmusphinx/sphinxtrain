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
 * File: inc_densities.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef INC_DENSITIES_H
#define INC_DENSITIES_H

#include <s3/prim_type.h>
#include <s3/vector.h>

int
inc_densities(float32 ***new_mixw,
	      vector_t ***new_mean,
	      vector_t ***new_var,

	      float32 ***mixw,
	      vector_t ***mean,
	      vector_t ***var,
	      float32 ***dnom,
	      
	      uint32 n_mixw,
	      uint32 n_mgau,
	      uint32 n_feat,
	      uint32 n_density,

	      uint32 n_inc);

#endif /* INC_DENSITIES_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1996/07/29  16:27:04  eht
 * *** empty log message ***
 *
 * Revision 1.1  1996/03/26  15:17:51  eht
 * Initial revision
 *
 *
 */
