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
 * File: s3tmat_io.h
 * 
 * Description: 
 *     SPHINX-III transition matrix file I/O functions
 *
 * Author: 
 *     Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S3TMAT_IO_H
#define S3TMAT_IO_H

#define TMAT_FILE_VERSION	"1.0"

#include <s3/prim_type.h>
#include <s3/s3.h>

int
s3tmat_read(const char *fn,
	    float32 ****out_tmat,
	    uint32 *out_n_tmat,
	    uint32 *out_n_state);

int
s3tmat_write(const char *fn,
	     float32 ***tmat,
	     uint32 n_tmat,
	     uint32 n_state);

#endif /* S3TMAT_IO_H */ 

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
