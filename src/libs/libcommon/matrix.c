/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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
 * File: matrix.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/matrix.h>
#include <s3/err.h>

void
norm_3d(float32 ***arr,
	uint32 d1,
	uint32 d2,
	uint32 d3)
{
    uint32 i, j, k;
    float64 s;

    for (i = 0; i < d1; i++) {
	for (j = 0; j < d2; j++) {

	    /* compute sum (i, j) as over all k */
	    for (k = 0, s = 0; k < d3; k++) {
		s += arr[i][j][k];
	    }

	    /* do 1 floating point divide */
	    s = 1.0 / s;

	    /* divide all k by sum over k */
	    for (k = 0; k < d3; k++) {
		arr[i][j][k] *= s;
	    }
	}
    }
}

void
accum_3d(float32 ***out,
	 float32 ***in,
	 uint32 d1,
	 uint32 d2,
	 uint32 d3)
{
    uint32 i, j, k;

    for (i = 0; i < d1; i++) {
	for (j = 0; j < d2; j++) {
	    for (k = 0; k < d3; k++) {
		out[i][j][k] += in[i][j][k];
	    }
	}
    }
}

void
floor_3d(float32 ***m,
	 uint32 d1,
	 uint32 d2,
	 uint32 d3,
	 float32 floor)
{
    uint32 i, j, k;

    for (i = 0; i < d1; i++) {
	for (j = 0; j < d2; j++) {
	    for (k = 0; k < d3; k++) {
		if (m[i][j][k] < floor)
		    m[i][j][k] = floor;
	    }
	}
    }
}

void
floor_nz_3d(float32 ***m,
	    uint32 d1,
	    uint32 d2,
	    uint32 d3,
	    float32 floor)
{
    uint32 i, j, k;

    for (i = 0; i < d1; i++) {
	for (j = 0; j < d2; j++) {
	    for (k = 0; k < d3; k++) {
		if ((m[i][j][k] != 0) && (m[i][j][k] < floor))
		    m[i][j][k] = floor;
	    }
	}
    }
}
void
floor_nz_1d(float32 *v,
	    uint32 d1,
	    float32 floor)
{
    uint32 i;

    for (i = 0; i < d1; i++) {
	if ((v[i] != 0) && (v[i] < floor))
	    v[i] = floor;
    }
}

/* Ensures that non-zero values x such that
 *	-band < x < band, band > 0
 * are set to -band if x < 0 and band if x > 0.
 */
void
band_nz_1d(float32 *v,
	   uint32 d1,
	   float32 band)
{
    uint32 i;

    for (i = 0; i < d1; i++) {
	if (v[i] != 0) {
	    if ((v[i] > 0) && (v[i] < band)) {
		v[i] = band;
	    }
	    else if ((v[i] < 0) && (v[i] > -band)) {
		v[i] = -band;
	    }
	}
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:05:40  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
