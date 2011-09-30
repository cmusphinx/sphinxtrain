/* ====================================================================
 * Copyright (c) 1994-2005 Carnegie Mellon University.  All rights 
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
/******************************************************************************
*    Main routine for computing the distribution for CDCN.
*    Coded by Bhiksha Raj, June 94
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "header.h"
#include "parse_cmd_ln.h"

int main(int argc, char **argv)
{
	vector_t *vector, *buff;
	float **mean, **variance, *c;
	float noisec, noisemean[64], noisevar[64];
	float atemp, noise_threshold, noise_width;
	int numnoise, numspch, numvecs, Ndim, Nmodes;
	int maxframes, vector_alloc;
	int i, j, k, *nbin, *bin;
	int superiter, stride;
	int32 length, x;

	parse_cmd_ln(argc, argv);
	Ndim = cmd_ln_int32("-ceplen");
	Nmodes = cmd_ln_int32("-nmodes");
	stride = cmd_ln_int32("-stride");
	maxframes = cmd_ln_int32("-maxframes");
	noise_width = cmd_ln_float32("-noisewidth");
	 /* FIXME: this will break when we have terabytes of memory... */
	if (maxframes == -1)
		maxframes = INT_MAX;

	/*
	 * by default I assume the programs is to be run from data
	 * only with no initial set of Gaussians. i.e. go to vq and them 
	 * em.
	 */
	corpus_set_mfcc_dir(cmd_ln_str("-cepdir"));
	corpus_set_mfcc_ext(cmd_ln_str("-cepext"));
	if (corpus_set_ctl_filename(cmd_ln_str("-ctlfn")) != S3_SUCCESS)
		E_FATAL("Failed to read control file %s\n", cmd_ln_str("-ctlfn"));

	if (corpus_init() != S3_SUCCESS)
		E_FATAL("Corpus initialization failed\n");

	numvecs = 0;
	numnoise = 0;
	for (j = 0; j < Ndim; ++j) {
		noisemean[j] = 0;
		noisevar[j] = 0;
	}

	/* Read in all frames (you can set a maximum to avoid dying) */
	/* Pick a reasonable size (about 60MB) to start with. */
	E_INFO("Allocating 100000 frames initially\n");
	vector_alloc = 1000000;
	vector = (vector_t *) ckd_calloc_2d(vector_alloc, Ndim, sizeof(float));
	E_INFO("Reading frames... ");
	while (corpus_next_utt() && (numvecs < maxframes)) {
		corpus_get_generic_featurevec(&buff, &length, Ndim);
		for (x = 0; x < length; x += stride) {
			if (numvecs >= vector_alloc) {
				vector_alloc = numvecs + 1000000;
				E_INFOCONT(" (Reallocating to %d frames) ", vector_alloc);
				vector = ckd_realloc(vector, sizeof(vector_t *) * vector_alloc);
				vector[0] = ckd_realloc(vector[0],
							Ndim*sizeof(float)*vector_alloc);
				for (j = 1; j < vector_alloc; ++j)
					vector[j] = vector[0] + j * Ndim;
			}
			memcpy(vector[numvecs], buff[x], Ndim*sizeof(float));
			++numvecs;
		}
		free(buff[0]);
		ckd_free(buff);
	}
	E_INFOCONT("%d vectors in all\n", numvecs);

	if (numvecs == 0)
		E_FATAL(("This is silly! You have given me only 0 vectors to compute a DISTRIBUTION!\nI am miffed! I am quitting!\n"));

	/*
	 * Compute threshold for the noise mode as the minimum c[0] + thresholding
	 */
	noise_threshold = vector[0][0];
	for (i = 0; i < numvecs; ++i)
		if (vector[i][0] < noise_threshold)
			noise_threshold = vector[i][0];
	noise_threshold += noise_width;
	E_INFO("Noise threshold = %f\n", noise_threshold);

	numnoise = 0;
	numspch = 0;
	for (i = 0; i < numvecs; ++i) {
		if (vector[i][0] <= noise_threshold) {
			for (j = 0; j < Ndim; ++j) {
				noisemean[j] += vector[i][j];
				noisevar[j] += vector[i][j] * vector[i][j];
			}
			++numnoise;
		} else {
			for (j = 0; j < Ndim; ++j)
				vector[numspch][j] = vector[i][j];
			++numspch;
		}
	}
	E_INFO
	    ("%d vectors found below noise threshold %f, %d vectors found above it\n",
	     numnoise, noise_threshold, numspch);


	/*
	 * Compute noise statistics
	 */
	for (j = 0; j < Ndim; ++j) {
		noisemean[j] /= (float) numnoise;
		noisevar[j] =
		    noisevar[j] / (float) numnoise -
		    noisemean[j] * noisemean[j];
	}
	noisec = (float) numnoise / (float) numvecs;
	Nmodes -= 1;		/* ACCOUNT FOR NOISE MODE : Rest of modes = total modes-1 */

	/*
	 * We Vector Quantize to obtain the initial values for the EM.
	 * If this codebook already exists, we skip the VQ and directly
	 * compute the variances and c[]s after obtaining the mean values
	 * as the code words in the existing codebook
	 */

	/*
	 * do this only if we are not requesting a restart from a previous 
	 * temp statistics file .
	 */
	if (cmd_ln_str("-cbfn") == NULL) {
		/*
		 * allocate the mean and variance and c arrays.
		 */
		c = (float *) ckd_calloc(Nmodes, sizeof(float));
		mean = (float **) ckd_calloc_2d(Nmodes, Ndim, sizeof(float));
		variance = (float **) ckd_calloc_2d(Nmodes, Ndim, sizeof(float));

		nbin = (int *) ckd_calloc(Nmodes, sizeof(int));	/* no of vectors in a mode */

		/*
		 * The vector_quantize routine performs VQ with a mahalonobis metric
		 * and returns the codes as the means and the wieghts as the variances
		 * of the initial estimates of the modes, which will further be 
		 * employed in EM. Note that the variances are purely diagonal
		 * We initialize all initial c[] to be equal
		 */

		bin = (int *) ckd_calloc(numspch, sizeof(int));


		vector_quantize(mean, Nmodes, vector, numspch, Ndim, bin,
				cmd_ln_int32("-vqiter"),
				cmd_ln_float32("-vqthresh"));

		for (i = 0; i < Nmodes; ++i)
			c[i] = 1.0 / (float) Nmodes;
		for (k = 0; k < Nmodes; ++k) {
			nbin[k] = 0;
			for (i = 0; i < Ndim; ++i)
				variance[k][i] = 0;
		}
		for (i = 0; i < numspch; ++i) {
			for (j = 0; j < Ndim; ++j) {
				atemp = (vector[i][j] - mean[bin[i]][j]);
				variance[bin[i]][j] += atemp * atemp;
			}
			++nbin[bin[i]];
		}
		for (k = 0; k < Nmodes; ++k) {
			for (j = 0; j < Ndim; ++j)
				variance[k][j] /= nbin[k];
		}

		ckd_free(bin);	/* We do not need this array anymore */
		ckd_free(nbin);	/* Chappie not needed anymore */
	} else {
		/* 
		 * if initialize = 0  ===> I want to skip the VQ and go to EM
		 * straight
		 */
		if (!read_backup_distribution
		    (cmd_ln_str("-cbfn"), &mean, &variance, &c, &Nmodes, Ndim))
			E_FATAL(("Unable to read initial distribution\n"));
	}

	for (superiter = 0; superiter < 1; ++superiter) {
		estimate_multi_modals(vector, numspch, Ndim, Nmodes, mean,
				      variance, c, cmd_ln_str("-tmpfn"),
				      cmd_ln_int32("-emiter"),
				      cmd_ln_float32("-emthresh"));
		if (store_distribution
		    (cmd_ln_str("-outfn"), Nmodes, Ndim, noisec, noisemean, noisevar, c,
		     mean, variance) != 0) {
			E_FATAL("Unable to open %s to store distribution\n",
			       cmd_ln_str("-tmpfn"));
		}
	}

	ckd_free_2d((void **)vector);
	ckd_free(c);
	ckd_free_2d((void **)mean);
	ckd_free_2d((void **)variance);
	return 0;
}
