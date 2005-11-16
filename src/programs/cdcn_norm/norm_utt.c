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
 * norm_utt normalizes the speech cepstrum vectors of the utterance z
 * to the vectors x, y with output codes. It finds the vectors x, noise
 * and tilt that maximize the a posteriori probability.
 * Coded by Alex Acero (acero@s),  November 1989 
 * Modified in many ways by Bhiksha:  June 94
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "header.h"

#define  SPEECH_THRESHOLD  1.0	/* This is the same threshold number that
				   is normally used to define the noise
				   threshold while computing the CDCN
				   distribution */

/* 
 * function to detect convergence of EM iprocedure
 * set the max num of iterations to 5
 */

int converged(float last_distortion, float distortion, int iteration)
{

	float improved;

	improved = 0.0;
	improved = (distortion - last_distortion);

	if ((improved > 0.0) && (iteration < 7))
		return (0);
	else
		return (1);
}

float norm_utt(float **z,	/* The observed cepstrum vectors */
	       float **x,	/* The normalized cepstrum */
	       int num_frames,	/* Number of frames in utterance */
	       float **variance,	/* The variances of the modes */
	       float *prob,	/* Ratio of a-priori probs to mod-variance of modes */
	       float **mean,	/* The means */
	       int num_codes,	/* Number of gaussian in distribution */
	       int Ndim		/* The Dimensionality of data */
    )
{
	float distortion, last_distortion, **corrbook, noise[64], tilt[64];
	int j, iteration;


	corrbook = (float **) ckd_calloc_2d(num_codes, Ndim, sizeof(float));

	/* Get initial estimates for noise, tilt, x, y */
	for (j = 1; j < Ndim; j++)
		tilt[j] = 0.0;
	initialize(z, num_frames, noise, tilt, SPEECH_THRESHOLD, mean,
		   prob, variance, num_codes, Ndim);
	correction(tilt, noise, mean, corrbook, num_codes, Ndim);
	distortion = max_q(variance, prob, noise, tilt, mean, corrbook,
			   num_codes, z, num_frames, Ndim);

	iteration = 0;
	last_distortion = -10.0;
	/* 
	 * Iterate until convergence in total distortion is reached 
	 */
	while (converged(last_distortion, distortion, iteration) == 0) {
		last_distortion = distortion;
		correction(tilt, noise, mean, corrbook, num_codes, Ndim);
		distortion =
		    max_q(variance, prob, noise, tilt, mean, corrbook,
			  num_codes, z, num_frames, Ndim);

		iteration++;

#ifdef DEBUG
		printf("iter = %d likelihood = %f \n", iteration,
		       distortion);
#endif
	}


	correction(tilt, noise, mean, corrbook, num_codes, Ndim);
	mmse_x(variance, prob, tilt, noise, mean, corrbook, num_codes, z,
	       x, num_frames, Ndim);
	ckd_free_2d((void **)corrbook);
	return (distortion);
}

