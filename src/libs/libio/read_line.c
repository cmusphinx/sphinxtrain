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
 * File: read_line.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/read_line.h>
#include <s3/err.h>

#include <string.h>

char *
read_line(char *buf,
	  size_t max_len,
	  uint32 *n_read,
	  FILE *fp)
{
    char *out;
    int last;

    do {
	out = fgets(buf, max_len, fp);
	(*n_read)++;
    } while ((out != NULL) && (out[0] == '#'));

    if (strlen(buf) == (max_len-1)) {
	E_WARN("line %d may be truncated due to max_len==%d\n",
	       *n_read, max_len);
    }
    
    if (out != NULL) {
	last = strlen(out)-1;
    
	if (out[last] == '\n')
	    out[last] = '\0';
    }
    
    return out;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
