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
 * File: s3mixw_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S3MIXW_IO_H
#define S3MIXW_IO_H

#define MIXW_FILE_VERSION "1.0"

#include <s3/prim_type.h>
#include <s3/s3.h>

int
s3mixw_read(const char *fn,
	    float32 ****out_mixw,
	    uint32 *out_n_mixw,
	    uint32 *out_n_feat,
	    uint32 *out_n_density);


int
s3mixw_intv_read(const char *fn,
		 uint32 mixw_s,
		 uint32 mixw_e,
		 float32 ****out_mixw,
		 uint32 *out_n_mixw,
		 uint32 *out_n_feat,
		 uint32 *out_n_density);


int
s3mixw_write(const char *fn,
	     float32 ***mixw,
	     uint32 n_mixw,
	     uint32 n_feat,
	     uint32 n_density);

#endif /* S3MIXW_IO_H */ 


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
