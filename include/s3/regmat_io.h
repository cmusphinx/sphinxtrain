/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: regmat_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef REGMAT_IO_H
#define REGMAT_IO_H

#define REGMAT_PARAM_VERSION 	"0.1"

#include <s3/prim_type.h>

int32
regmat_param_write(const char *file_name,
		   const char *comment,
		   float32 ****regr,
		   float32 *****regl,
		   uint32 n_mllr_class,
		   uint32 n_feat,
		   const uint32 *veclen,
		   uint32 mllr_mult,
		   uint32 mllr_add);

int32
regmat_param_read(const char **comment,
		  float32 *****regr,
		  float32 ******regl,
		  uint32 *n_mllr_class,
		  uint32 *n_feat,
		  uint32 **veclen,
                  uint32 *mllr_mult,
                  uint32 *mllr_add,
		  const char *file_name);


#endif /* REGMAT_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/07/29  16:33:40  eht
 * Initial revision
 *
 *
 */
