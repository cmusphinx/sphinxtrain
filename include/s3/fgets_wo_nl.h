/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: fgets_wo_nl.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef FGETS_WO_NL_H
#define FGETS_WO_NL_H

#include <stdio.h>

char *
fgets_wo_nl(char *str,
	    size_t max,
	    FILE *fp);


#endif /* FGETS_WO_NL_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 *
 */
