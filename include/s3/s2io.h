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
 * File: s2io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef S2IO_H
#define S2IO_H

int
areadchar (char *file,
	   char **data_ref,
	   int *length_ref);

int
areadfloat (char *file,
	    float **data_ref,
	    int *length_ref);
int
awritefloat (char *file,
	     float *data,
	     int length);
int
areadfloat_part (char *file,
		 int s_coeff,
		 int e_coeff,
		 float **data_ref,
		 int *length_ref);

int
areadint (char *file,
	  int **data_ref,
	  int *length_ref);
int
awriteint (char *file,
	   int *data,
	   int length);

int
areadshort (char *file,
	    short **data_ref,
	    int *length_ref);

#endif /* S2IO_H */ 


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
