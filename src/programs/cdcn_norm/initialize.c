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
 * initialize finds an estimate of the noise vector as the average of all
 * frames whose power is below a threshold.
 * It also computes the initial estimate of the channel as the mean difference
 * between the noisy speech and the clean speech statistics
 * Coded by Bhiksha, Aug 94
 *
 *************************************************************************/

#include <stdio.h>
#include <math.h>

float initialize(float **data,	/* The observation cepstrum vectors */
		 int num_frames,	/* Number of frames in utterance */
		 float *noise,	/* Cepstrum vector for the noise */
		 float *tilt, float speech_threshold,	/* Threshold for speech and noise */
		 float **mean, float *prob, float **var, int ncodes,	/* Number of gaussians in distribution */
		 int Ndim	/* Dimensionality of data */
    )
{
	float noise_ceiling,	/* Threshold to separate speech and noise */
	 min,			/* Minimum log-energy in utterance */
	 max,			/* Maximum log-energy in utterance */
	 speech_power,		/* Average speech power */
	 codemean[64], localprob[256];
	int i,			/* Index all frames in utterance */
	 j,			/* Index all coefficients within frame */
	 noise_frames,		/* Number of noise frames */
	 speech_frames;		/* Number of speech frames */


	/* De-normalize prob w.r.t variance */
	for (i = 0; i < ncodes; ++i) {
		localprob[i] = 1.0;
		for (j = 0; j < Ndim; ++j)
			localprob[i] *= var[i][j];
		localprob[i] = prob[i] * sqrt(localprob[i]);
	}
	/* Initialize tilt */
	for (j = 0; j < Ndim; ++j) {
		tilt[j] = 0;
		codemean[j] = 0;
		for (i = 0; i < ncodes; ++i)
			codemean[j] += localprob[i] * mean[i][j];
	}
	for (i = 0; i < num_frames; ++i)
		for (j = 0; j < Ndim; ++j)
			tilt[j] += data[i][j];

	for (j = 0; j < Ndim; ++j)
		tilt[j] = tilt[j] / (float) num_frames - codemean[j];

	/* Search for the extrema c[0] in the file */
	min = max = data[0][0];
	for (i = 0; i < num_frames; i++) {
		if (data[i][0] < min)
			min = data[i][0];
		if (data[i][0] > max)
			max = data[i][0];
	}

	/* Compute thresholds for noise */
	noise_ceiling = min + (max - min) / 20;

	/* Every frame whose power is below noise_ceiling is considered noise.
	   and every frame above is considered speech */
	noise_frames = 0;
	speech_frames = 0;
	speech_power = 0;
	for (j = 0; j < Ndim; j++)
		noise[j] = 0.0;
	for (i = 0; i < num_frames; i++) {
		if (data[i][0] < noise_ceiling) {
			noise_frames++;
			for (j = 0; j < Ndim; j++)
				noise[j] += data[i][j];
		} else {
			speech_frames++;
			speech_power += data[i][0];
		}
	}
	for (j = 0; j < Ndim; j++)
		noise[j] /= noise_frames;
	speech_power /= speech_frames;
	return (speech_power);
}
