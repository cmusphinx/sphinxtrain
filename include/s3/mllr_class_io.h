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
 * File: mllr_class_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MLLR_CLASS_IO_H
#define MLLR_CLASS_IO_H

#define MLLR_CLASS_VERSION	"0.1"

#include <s3/prim_type.h>
#include <s3/gauden.h>

int
mllr_class_read(uint32 **mllr_cmap,
		uint32 *n_map,
		uint32 *n_class,
		const char **comment,
		const char *file_name);

int
mllr_class_write(const char *file_name,
		 uint32 *mllr_class_map,
		 uint32 n_mgau,
		 uint32 n_mllr_class,
		 char *comment);

int
ck_mllr_class(uint32 *mllr_class_map,
	      uint32 n_map,
	      uint32 n_mgau);

#endif /* MLLR_CLASS_IO_H */ 


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
