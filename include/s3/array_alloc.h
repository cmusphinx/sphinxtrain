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
 * file: array_alloc.h
 * 
 * traceability: 
 * 
 * description: 
 * 
 * author: 
 * 
 *********************************************************************/

#ifndef ARRAY_ALLOC_H
#define ARRAY_ALLOC_H

#include <stddef.h>	/* defines size_t */

void ****
alloc_4d_array(int d1,
	       int d2,
	       int d3,
	       int d4,
	       size_t elem_size);
int
free_4d_array(void ****arr);

void ***
access_3d_array(int d1,
		int d2,
		int d3,
		void *store,
		size_t elem_size);
void ***
alloc_3d_array(int d1, int d2, int d3, size_t elem_size);

int
free_3d_array(void ***arr);

void **
alloc_2d_array(int d1, int d2, size_t elem_size);

int
free_2d_array(void **arr);

#endif /* ARRAY_ALLOC_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/10  15:26:58  eht
 * Get size_t definition from ANSI location of <stddef.h>
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
