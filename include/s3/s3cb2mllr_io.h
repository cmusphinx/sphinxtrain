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
 * File: s3cb2mllr_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef S3CB2MLLR_IO_H
#define S3CB2MLLR_IO_H

#include <s3/prim_type.h>

int
s3cb2mllr_read(const char *fn,
	       uint32 **out_cb2mllr,
	       uint32 *out_n_cb,
	       uint32 *out_n_mllr);

int
s3cb2mllr_write(const char *fn,
		uint32 *cb2mllr,
		uint32 n_cb,
		uint32 n_mllr);

#endif /* S3CB2MLLR_IO_H */ 


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
