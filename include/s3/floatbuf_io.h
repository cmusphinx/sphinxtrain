/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: floatbuf_io.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef FLOATBUF_IO_H
#define FLOATBUF_IO_H

#include <s3/prim_type.h>
#include <s3/s3.h>

#include <stdio.h>

int32
float32buf_write(FILE *fp,
		 float32 *b,
		 uint32 n);
int32
float32buf_read(float32 **out_b,
		uint32 *out_n,
		FILE *fp);

#endif /* FLOATBUF_IO_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
