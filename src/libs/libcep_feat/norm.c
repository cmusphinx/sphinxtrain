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
 * File: norm.c
 * 
 * Description: 
 *	This file implements batch CMN for an utterance.
 *
 *	norm_mean()	- compute the mean of the input vectors
 *			  and then subtract the mean from the
 *			  input vectors. Leave coefficient 0
 *			  untouched. * 
 * Author: 
 * 	eht (?), faa (?) dunno
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include <s3/norm.h>
#include <s3/ckd_alloc.h>

#include <s3/prim_type.h>
#include <s3/cmd_ln.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void norm_mean (float32 *vec,		/* the data */
		uint32 nvec,	/* number of vectors (frames) */
		uint32 veclen)	/* number of components per vector */
{
    static double      *mean = 0, *var = 0; 
    double   temp;
    float32            *data;
    uint32               i, f; 
    const char *normvar = cmd_ln_access("-varnorm");

    if (mean == 0)
	mean = (double *) ckd_calloc (veclen, sizeof (double));
    if (var == 0) 
	var = (double *) ckd_calloc (veclen, sizeof (double));

    for (i = 0; i < veclen; i++)
	mean[i] = var[i] = 0.0;

    /*
     * Compute the sum
     */
    for (data = vec, f = 0; f < nvec; f++, data += veclen) {
	for (i = 0; i < veclen; i++)
	    mean[i] += (double)data[i];
    }

    /*
     * Compute the mean
     */
    for (i = 0; i < veclen; i++) {
	mean[i] /= (double)nvec;
    }

    if (strcmp(normvar,"yes") == 0){
        for (data = vec, f = 0; f < nvec; f++, data += veclen) {
    	    for (i = 0; i < veclen; i++)
	        var[i] += ((double)data[i]-mean[i])*((double)data[i]-mean[i]);
        }
        for (i = 0; i < veclen; i++) {
    	    temp = var[i] / (double)nvec;
            var[i] = sqrt(temp);
        }
    }
    
    /*
     * Normalize the data
     */
    for (data = vec, f = 0; f < nvec; f++, data += veclen) {
	for (i = 0; i < veclen; i++)
	    data[i] -= (float)mean[i];
    }
    if (strcmp(normvar,"yes") == 0){
        for (data = vec, f = 0; f < nvec; f++, data += veclen) {
    	    for (i = 0; i < veclen; i++)
	        data[i] /= (float)var[i];
        }
    }
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1995/10/17  13:05:04  eht
 * Cleaned up code a bit so that it is more ANSI compliant
 *
 * Revision 1.2  1995/10/10  12:36:12  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/06/02  20:57:22  eht
 * Initial revision
 *
 *
 */
