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
 * File: prim_type.h
 * 
 * Description: 
 *	This file defines some unambiguous primitive types.
 * 
 * Author: 
 *
 * 08-31-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 * 
 *********************************************************************/

#ifndef PRIM_TYPE_H
#define PRIM_TYPE_H


#if defined(__alpha)
typedef long		int64;
#endif
typedef int		int32;
typedef short		int16;
typedef char		int8;
typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
typedef float		float32;
typedef double		float64;

#ifdef _WIN32
typedef char * caddr_t;
#endif

#endif	/* PRIM_TYPE_H */


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.7  1996/07/29  16:40:50  eht
 * added WIN32
 *
 * Revision 1.6  1996/03/25  15:50:59  eht
 * Added define of int64 where they are available
 *
 * Revision 1.5  1996/03/21  12:23:09  eht
 * Added 64-bit definitions for Alpha
 *
 * Revision 1.4  1995/10/13  19:43:06  eht
 * Change comments to conform to SPHINX-III style
 * Changed _WINDOWS to WIN32 to check for compilation under WIN32
 * API.
 *
 *
 */
