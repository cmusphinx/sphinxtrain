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
 * max_q reestimates the tilt cepstrum vector that maximizes the likelihood.
 * It also labels the cleaned speech.
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified by Bhiksha June 94:
 *     Accounted for variance (originally ignored in the program!?!?!)
 * Modified by PJM to squash bug in noise estimation 1995
 * Modified by Bhiksha to eliminate hard coded dimensionality
 *
 *************************************************************************/

#include <stdio.h>
#include <math.h>

#include "header.h"

float max_q(float **variance,	/* Speech cepstral variances of the modes */
	    float *prob,	/* Ratio of a-priori probabilities of the codes 
				   and the mod of their variances */
	    float *noise,	/* Cepstrum vector for the noise */
	    float *tilt,	/* Spectral tilt cepstrum */
	    float **mean,	/* The cepstrum mean */
	    float **corrbook,	/* The correction factor's codebook */
	    int num_codes,	/* Number of codewords in codebook */
	    float **z,		/* The input cepstrum */
	    int num_frames,	/* Number of frames in utterance */
	    int Ndim		/* Dimensionality of data */
    )
{
	float newtilt[64],	/* The new tilt vector */
	 newnoise[64],		/* The new noise vector */
	 distance,		/* distance value */
	 loglikelihood,		/* The log-likelihood */
	 probz,			/* Prob. of z given tilt/noise */
	 dennoise,		/* Denominator of noise estimation */
	 dentilt,		/* Denominator of tilt estimation */
	 pnoise,		/* Probability that frame is noise */
	 qk[64],		/* An auxiliary vector */
	 qi[64],		/* Contribution to tilt of frame i */
	 ni[64],		/* Contribution to noise of frame i */
	  fk;			/* Probabilities for different codewords */
	int i,			/* Index frames in utterance */
	 j,			/* Index coefficients within frame */
	 k;			/* Index codewords in codebook */

	/* Initialize newtilt and newnoise */
	for (j = 0; j < Ndim; j++) {
		newtilt[j] = 0.0;
		newnoise[j] = 0.0;
	}
	loglikelihood = 0.0;
	dennoise = 0.0;
	dentilt = 0.0;

	/* Reestimate tilt vector for all frames in utterance */
	for (i = 0; i < num_frames; i++) {
		/* Reestimate noise vector for codeword 0 */
		for (j = 0; j < Ndim; j++)
			qk[j] = z[i][j] - corrbook[0][j];
		distance = dist(qk, noise, variance[0], Ndim);
		fk = exp((double) -distance / 2) * prob[0];
		probz = fk;
		pnoise = fk;
		for (j = 0; j < Ndim; j++) {
			ni[j] = qk[j] * fk;
			qi[j] = 0.0;
		}

		/* Reestimate tilt vector across all codewords */
		for (k = 1; k < num_codes; k++) {
			/* Restimate tilt vector for codeword k */
			for (j = 0; j < Ndim; j++)
				qk[j] =
				    z[i][j] - mean[k][j] - corrbook[k][j];
			distance = dist(qk, tilt, variance[k], Ndim);
			fk = exp((double) -distance / 2) * prob[k];
			probz += fk;
			for (j = 0; j < Ndim; j++)
				qi[j] += qk[j] * fk;
		}
		if (probz != 0.0) {
			/*
			 * Earlier the sign in the loglikelihood used to be negative!! : PJM
			 */
			loglikelihood += log((double) probz);
			pnoise /= probz;
			dennoise += pnoise;
			dentilt += (1 - pnoise);
			for (j = 0; j < Ndim; j++) {
				newnoise[j] += ni[j] / probz;
				newtilt[j] += qi[j] / probz;
			}
		}
	}

	/* Normalize the estimated tilt vector across codewords */
	for (j = 0; j < Ndim; j++) {
		if (dennoise != 0)
			noise[j] = newnoise[j] / dennoise;
		if (dentilt != 0)
			tilt[j] = newtilt[j] / dentilt;
	}
	loglikelihood /= num_frames;
	/* 
	 * we deactivate this part of the code since it is not needed
	 *
	 * loglikelihood += OFFSET_LIKELIHOOD  ; 
	 *
	 */
	return (loglikelihood);
}
