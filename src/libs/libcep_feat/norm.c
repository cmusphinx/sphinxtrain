/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*********************************************************************
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
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
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
