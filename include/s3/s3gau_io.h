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
 * File: s3gau_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef S3GAU_IO_H
#define S3GAU_IO_H

#include <s3/vector.h>

#define GAU_FILE_VERSION	"1.0"
#define GAUCNT_FILE_VERSION	"1.0"
#define GAUDNOM_FILE_VERSION	"1.0"

int
s3gau_read(const char *fn,
	   vector_t ****out,
	   uint32 *out_n_mgau,
	   uint32 *out_n_feat,
	   uint32 *out_n_density,
	   const uint32 **out_veclen);

int
s3gau_write(const char *fn,
	    const vector_t ***out,
	    uint32 n_mgau,
	    uint32 n_feat,
	    uint32 n_density,
	    const uint32 *veclen);

int
s3gaucnt_read(const char *fn,
	      vector_t ****out_wt_mean,
	      vector_t ****out_wt_var,
	      int32 *out_pass2var,
	      float32 ****out_dnom,
	      uint32 *out_n_mgau,
	      uint32 *out_n_feat,
	      uint32 *out_n_density,
	      const uint32 **out_veclen);

int
s3gaucnt_write(const char *fn,
	       vector_t ***wt_mean,
	       vector_t ***wt_var,
	       int32 pass2var,
	       float32 ***dnom,
	       uint32 n_cb,
	       uint32 n_feat,
	       uint32 n_density,
	       const uint32 *veclen);

int
s3gaudnom_read(const char *fn,
	       float32 ****out_dnom,
	       uint32 *out_n_cb,
	       uint32 *out_n_feat,
	       uint32 *out_n_density);

int
s3gaudnom_write(const char *fn,
		float32 ***dnom,
		uint32 n_cb,
		uint32 n_feat,
		uint32 n_density);

#endif /* S3GAU_IO_H */ 


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
