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
 * File: s3lamb_io.h
 * 
 * Description: 
 *    Prototypes and definitions for s3lamb_io
 *
 * Author: 
 *    Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S3LAMB_IO_H
#define S3LAMB_IO_H

#define LAMBDA_FILE_VERSION "1.0"
#define LAMBDACNT_FILE_VERSION "1.0"

#include <s3/prim_type.h>

int
s3lamb_read(const char *fn,
	    float32 **out_lambda,
	    uint32 *out_n_lambda);

int
s3lamb_write(const char *fn,
	     const float32 *lambda,
	     uint32 n_lambda);

int
s3lambcnt_read(const char *fn,
	       uint32 **out_lambda_cnt,
	       uint32 *out_n_lambda_cnt);

int
s3lambcnt_write(const char *fn,
		const uint32 *lambda_cnt,
		uint32 n_lambda_cnt);

#endif /* S3LAMB_IO_H */ 


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
