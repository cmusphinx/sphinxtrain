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
 * File: fgets_wo_nl.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/fgets_wo_nl.h>
#include <s3/err.h>
#include <s3/prim_type.h>

#include <string.h>

char *
fgets_wo_nl(char *str,
	    size_t max,
	    FILE *fp)
{
    char *out;
    uint32 len;

    out = fgets(str, max, fp);

    if (out == NULL)
	return NULL;

    len = strlen(out);

    if (out[len-1] == '\n')
	out[len-1] = '\0';
    else {
	E_FATAL("input string too long.  Truncated.\n");

	return NULL;
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
 *
 */
