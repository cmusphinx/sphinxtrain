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
 * File: tmat_param_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef TMAT_PARAM_IO_H
#define TMAT_PARAM_IO_H

#define TMAT_PARAM_VERSION	"0.1"

#include <s3/prim_type.h>

int32
tmat_param_write(const char *file_name,
		 const char *comment,
		 float32 ***tmat,
		 unsigned int n_tmat,
		 unsigned int n_state);

int32
tmat_param_read(const char **out_comment,
		float32 ****out_tmat,
		unsigned int *out_n_tmat,
		unsigned int *out_n_state,
		const char *file_name);

int32
tmat_param_unbundle(const char *dir,
		    float32 ***tmat,
		    unsigned int n_tmat,
		    unsigned int n_state);

#endif /* TMAT_PARAM_IO_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.5  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.4  96/11/04  07:08:13  eht
 * Added prototype for tmat_param_unbundle
 * 
 * Revision 1.3  96/07/29  16:33:40  eht
 * added bundle/unbundle routines
 * 
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
