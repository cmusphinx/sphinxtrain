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
/*************************************************************************
 *
 * Subroutine correction computes the correction cepstrum vectors for a 
 * codebook given the spectral tilt and the noise cepstrum vectors.
 * For every codeword it finds the corresponding correction vector. That
 * is, every cepstrum vector within a cluster is transformed differently.
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified by PJM to correct bug in 0th mode correction factor estimate
 * Modified by Bhiksha to eliminate Harcoded Dimensionality
 *
 *************************************************************************/

#include <stdio.h>
#include <math.h>

#include "header.h"

/* Define parameters needed for the FFTs */
#define N                       32	/* Parameters for FFT */
#define N2                      64
#define M                       5

/*************************************************************************
 *
 * This function computes ln[exp(x) + 1] which is the non-linear function
 * necessary for the noise subtraction in the cepstral domain.
 *
 * Coded by Alex Acero (acero@s),  November 1989 
 *
 *************************************************************************/

float f1(float x)
{
	float y;

	y = log(exp((double) x) + 1.0);
	return (y);
}


void correction(float *tilt,	/* The spectral tilt cepstrum vector */
		float *noise,	/* The noise cepstrum vector */
		float **mean,	/* The means */
		float **corrbook,	/* The correction cepstrum vectors */
		int num_codes,	/* The number of gaussians in the distribution */
		int Ndim	/* The dimensionality of the data */
    )
{
	float aux[N + 1];		/* auxiliary vector for FFTs */
	int i,			/* Index for all modes in distribution */
	 j;			/* Index for all coefficients within a frame */


	/* 
	 * The 0th correction is different from the rest of the corrections:
	 * Here we speak of the y = n + s() expansion rather than  y = x + q + r()
	 */
	/* Take direct FFT of -(noise - tilt - codeword) */
	for (j = 0; j < Ndim; j++)
		aux[j] = mean[0][j] + tilt[j] - noise[j];
	for (j = Ndim; j <= N; j++)
		aux[j] = 0.0;
	resfft(aux, N, M);
	/* Process every frequency through non-linear function f1 */
	for (j = 0; j <= N; j++)
		aux[j] = f1(aux[j]);

	/* Take inverse FFT and write result back */
	resfft(aux, N, M);
	for (j = 0; j < Ndim; j++)
		corrbook[0][j] = aux[j] / N2;

	for (i = 1; i < num_codes; i++) {
		/* Take direct FFT of noise - tilt - codeword */
		for (j = 0; j < Ndim; j++)
			aux[j] = noise[j] - tilt[j] - mean[i][j];
		for (j = Ndim; j <= N; j++)
			aux[j] = 0.0;
		resfft(aux, N, M);

		/* Process every frequency through non-linear function f1 */
		for (j = 0; j <= N; j++)
			aux[j] = f1(aux[j]);

		/* Take inverse FFT and write result back */
		resfft(aux, N, M);
		for (j = 0; j < Ndim; j++)
			corrbook[i][j] = aux[j] / N2;
	}
}

