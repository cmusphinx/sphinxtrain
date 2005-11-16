/* ====================================================================
 * Copyright (c) 1989-2005 Carnegie Mellon University.  All rights 
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
/************************************************************************
 *									*
 *	This subroutine computes a split-radix FFT for real and		*
 *	even data of length 2n where n = 2^(m). It uses a real FFT 	*
 *	of length n/2 and the algorithm described in "On the Use	*
 *	of Symmetry in FFT Computation" by L. Rabiner in Trans. on	*
 *	ASSP June 1979, pp. 233-239.					*
 *									*
 *	Input format:  x[0], x[1], ... x[n]				*
 *	Output format: x[0], x[1], ... x[n]				*
 *	The result is in place and the transform is also real		*
 *	and even.							*
 *									*
 *	If one desires to compute the inverse FFT one can get it	*
 *	by calling resfft (x, n, m). The actual result is 2*n times	*
 *	the true IFFT.							*
 *									*
 ************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "header.h"

void resfft(float *x, int n, int m)
{
	int n2, n4, i, j, k, nk;
	float b0, yt, *y;
	double e;

	n2 = n;
	n4 = n2 / 2;
	for (b0 = 0, i = 1; i < n2; i += 2)
		b0 += x[i];
	b0 *= 2;
	y = (float *) ckd_calloc(n2, sizeof(float));
	y[0] = x[0];
	y[n4] = x[n2];
	for (i = 1; i < n4; i++) {
		j = 2 * i;
		yt = x[j + 1] - x[j - 1];
		y[i] = x[j] + yt;
		y[n2 - i] = x[j] - yt;
	}
	rsfft(y, n2, m);
	e = 3.141592653589793 / n;
	for (k = 1; k < n4; k++)
		y[n2 - k] /= 2 * sin(e * k);
	x[0] = y[0] + b0;
	x[n2] = y[0] - b0;
	x[n4] = y[n4];
	for (k = 1; k < n4; k++) {
		nk = n2 - k;
		x[k] = y[k] + y[nk];
		x[nk] = y[k] - y[nk];
	}
	ckd_free(y);
}
