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
 * File: n_words.c
 * 
 * Description: 
 *	This routine counts the number of space separated strings
 *	(i.e. words) in a given string.
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/n_words.h>

#include <ctype.h>


uint32
n_words(char *str)
{
    int n;

    n = 0;
    for (;;) {
	while (*str && isspace(*str)) str++;
	if (*str) {
	    while (!isspace(*str) && *str) str++;
		++n;
	}
	else break;
    }

    return n;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1995/10/12  17:41:30  eht
 * Updated comments
 *
 * Revision 1.2  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.1  1995/06/02  14:52:54  eht
 * Initial revision
 *
 *
 */
