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
 * File: s3.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef S3_H
#define S3_H

#include <s3/err.h>
#include <s3/prim_type.h>

#define	S3_SUCCESS	0
#define S3_ERROR	-1
#define S3_WARNING	-2

#define MAX_IEEE_NORM_POS_FLOAT32	 3.4e+38
#define MAX_POS_FLOAT32	 		 3.4e+38

#define MIN_IEEE_NORM_POS_FLOAT32	 1.2e-38
#define MIN_POS_FLOAT32	 		 1.2e-38

#define MIN_IEEE_NORM_NEG_FLOAT32	-3.4e+38
#define MIN_NEG_FLOAT32			-3.4e+38

#define MAX_IEEE_NORM_POS_FLOAT64	 1.8e+307
#define MAX_POS_FLOAT64	 		 1.8e+307

#define MIN_IEEE_NORM_POS_FLOAT64	 2.2e-308
#define MIN_POS_FLOAT64	 		 2.2e-308

#define MIN_IEEE_NORM_NEG_FLOAT64	-1.8e+307
#define MIN_NEG_FLOAT64			-1.8e+307

/* The maximum # of states for any given acoustic model */
#define MAX_N_STATE	20

/* The maximum # of attributes associated with any
 * given acoustic model */
#define MAX_N_ATTRIB	5

#ifndef TRUE
#define TRUE  1
#define FALSE 0	/* assume that true is never defined w/o false */
#endif

/* Timer for elapsed I/O time */
#define IO_ELAPSED	0

/* Timer for utt processing elapsed time */
#define UTT_ELAPSED	1
#define UTT_IO_ELAPSED	2
#define UTT_BW_ELAPSED	3

#define TYING_NON_EMITTING	(0xffffffff)
#define TYING_NO_ID		(0xffffffff)

#define MAX_VERSION_LEN	128

#define MEG *1024*1024

#endif /* S3_H */ 

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
 * Revision 1.4  96/07/29  16:40:50  eht
 * Make some of the constants less long winded.
 * 
 * Revision 1.3  1996/01/26  18:29:54  eht
 * Added some floating point MIN and MAX constants
 *
 * Revision 1.2  1995/10/10  12:25:04  eht
 * Add TYING_NO_ID to the set of symbolic constants defined.
 *
 * Revision 1.1  1995/10/09  21:17:24  eht
 * Initial revision
 *
 *
 */
