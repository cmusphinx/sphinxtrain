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
 * File: lambda_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef LAMBDA_IO_H
#define LAMBDA_IO_H

#include <s3/prim_type.h>

#define LAMBDA_VERSION "0.1"

int
lambda_write(const char *fn,
	     float32 *lambda,
	     uint32 n_lambda,
	     const char *comment);

int
lambda_read(float32 **lambda,
	    uint32 *n_lambda,
	    const char **comment,
	    const char *fn);
	    
#define LAMBDA_ACC_VERSION "0.1"

int
lambda_acc_write(const char *fn,
		 float32 *lambda_acc,
		 uint32 n_lambda_acc,
		 const char *comment);

int
lambda_acc_read(float32 **lambda_acc,
		uint32 *n_lambda_acc,
		const char **comment,
		const char *fn);

#define LAMBDA_CNT_VERSION "0.1"

int
lambda_cnt_write(const char *fn,
		 uint32 *cnt,
		 uint32 n_cnt,
		 const char *comment);

int
lambda_cnt_read(uint32 **cnt,
		uint32 *n_cnt,
		const char **comment,
		const char *fn);


#endif /* LAMBDA_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1996/07/29  16:33:40  eht
 * Initial revision
 *
 *
 */
