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
 * File: swap.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef SWAP_H
#define SWAP_H

#define SWAP_INT16(x)	*(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))

#define SWAP_INT32(x)	*(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
				(0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))

#define SWAP_FLOAT32(x)	SWAP_INT32((int32 *) x)

#include <s3/prim_type.h>

#include <s3/s3.h>

#include <stdio.h>

int32
swap_check(FILE *fp);

int32
swap_stamp(FILE *fp);

int
swap_little_endian(void);

#endif /* SWAP_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.4  96/03/25  15:52:35  eht
 * Added more functions
 * 
 * Revision 1.3  1995/10/18  11:26:34  eht
 * Include missing prototype for swap_stamp()
 *
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
