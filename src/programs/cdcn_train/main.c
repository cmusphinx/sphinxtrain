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
#include "header.h"

#define  QUIT(x)  {printf x; fflush (stdout); exit(-1);}

#define DESCRIPTION "\nThis program computes a mixture gaussian distribution for a set of \nspeech cepstral files for use with CDCN. The first mode is computed on only\nthe silence portions of the speech to comply with the requirements of the \nalgorithm. The dimensionality of the cepstral files is assumed to be 13\n"

#define USAGE "USAGE:\n%s -e <filename extension> \\\n\t-c <control file> \\\n\t-o <output file> \\\n\t-m <Total no of modes in distribution>\\\n\t-d <dimensionality of the data (default 13)>\\\n\t-w <c0 width to use for the noise mode> \\\n\t-z the file containing the initial distribution to continue EM from> \\\n\t-t <file to dump temporary results to> \n"

int main(int argc, char **argv)
{
	float **vector, *buff;
	float **mean, **variance, *c;
	float noisec, noisemean[64], noisevar[64];
	float atemp, noise_threshold, noise_width = 1.0;
	int numnoise, numspch, numvecs, Ndim, Nmodes, maxlength = 0;
	int i, j, k, length, *nbin, *bin;
	int initialize, superiter;
	char ctlfile[512], infileext[10], outfile[512], tempfile[512];
	char basefile[512], filename[512], initialcodebk[512], sw;

	FILE *listfile;

	/*
	 * by default I assume the programs is to be run from data
	 * only with no initial set of Gaussians. i.e. go to vq and them 
	 * em.
	 */
	/* SET DEFAULTS */
	initialize = 1;
	strcpy(tempfile, "CDCN.DIST.TEMP");
	strcpy(infileext, "");
	Ndim = 13;

	if (argc == 1) {
		printf(DESCRIPTION);
		QUIT((USAGE, argv[0]));
	}
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-')
			QUIT((USAGE, argv[0]));
		sw = argv[i][1];
		switch (sw) {
		case 'e':
		case 'E':
			{
				strcpy(infileext, argv[++i]);
				break;
			}
		case 'c':
		case 'C':
			{
				strcpy(ctlfile, argv[++i]);
				break;
			}
		case 'o':
		case 'O':
			{
				strcpy(outfile, argv[++i]);
				break;
			}
		case 'm':
		case 'M':
			{
				Nmodes = atoi(argv[++i]);
				break;
			}
		case 'd':
		case 'D':
			{
				Ndim = atoi(argv[++i]);
				break;
			}
		case 'w':
		case 'W':
			{
				sscanf(argv[++i], "%f", &noise_width);
				break;
			}
		case 'z':
		case 'Z':
			{
				strcpy(initialcodebk, argv[++i]);
				/*
				 * do not initialize from VQ but from this file
				 */

				/*
				 * notice that this is ONLY prepared for restarting the
				 * program from a previous EM files with the same number
				 * of Gaussians. i.e. if the machinbe breaks you can 
				 * contine EM from there
				 */
				initialize = 0;
				break;
			}
		case 't':
		case 'T':
			{
				/*
				 * Temporary file to store partially converged distribution
				 * to
				 */
				strcpy(tempfile, argv[++i]);
				break;
			}
		default:
			{
				printf(DESCRIPTION);
				QUIT((USAGE, argv[0]));
			}

		}
	}


	listfile = fopen(ctlfile, "r");
	if (listfile == NULL)
		QUIT(("Unable to open control file %s\n", ctlfile));

	numvecs = 0;
	numnoise = 0;
	for (j = 0; j < Ndim; ++j) {
		noisemean[j] = 0;
		noisevar[j] = 0;
	}

	maxlength = 0;
	while (fscanf(listfile, "%s\n", basefile) != EOF) {
		/*
		 * If an extension has been specified for the files, append it
		 */
		if (strcmp(infileext, ""))
			sprintf(filename, "%s.%s", basefile, infileext);
		else
			strcpy(filename, basefile);
		if (areadfloat(filename, &buff, &length) == -1) {
			/*
			 * What happens on a datafile read error? We can either just go on to
			 * the next cepfile, or stop the program.  Set switch in header.h.
			 */
#ifdef IGNORE_READ_ERR
			printf("READ FAILURE; skipping cepfile\n");
			fflush(stdout);
#else
			QUIT(("Unable to read %s\n", filename));
#endif
		} else {
			maxlength += length / Ndim;
			free(buff);
		}
	}
	printf("%d vectors in all\n", maxlength);
	rewind(listfile);

	/*
	 * For our particular case it is needed that the Maximum dimensionality
	 * is the same as the actual dimension of the data. Else the areadfloat
	 * routine fails!
	 */
	/* FIXME: We should iterate over the corpus for VQ instead of
	   allocating this unbearably massive array. */
	vector = (float **) ckd_calloc_2d(maxlength, Ndim, sizeof(float));

	while ((fscanf(listfile, "%s\n", basefile) != EOF)
	       && (numvecs <= maxlength)) {
		/*
		 * If an extension has been specified for the files, append it
		 */
		if (strcmp(infileext, ""))
			sprintf(filename, "%s.%s", basefile, infileext);
		else
			strcpy(filename, basefile);
		if (areadfloat(filename, &buff, &length) == -1) {
			/*
			 * What happens on a datafile read error? We can either just go on to
			 * the next cepfile, or stop the program.  Set switch in header.h.
			 */
#ifdef IGNORE_READ_ERR
			printf("READ FAILURE; skipping cepfile\n");
			fflush(stdout);
#else
			QUIT(("Unable to read %s\n", filename));
#endif
		} else {
#ifdef DEBUG
			printf("Success in reading %s feature file\n",
			       filename);
#endif
			for (i = 0; i < length; i += Ndim) {
				for (j = 0; j < Ndim; ++j)
					vector[numvecs][j] = buff[i + j];
				++numvecs;
			}
			free(buff);
		}
		if (numvecs > maxlength)
			QUIT(("**** Too many frames? Check your dimensionality!! ****\n"));
	}
	fclose(listfile);

	if (numvecs == 0)
		QUIT(("This is silly! You have given me only 0 vectors to compute a DISTRIBUTION!\nI am miffed! I am quitting!\n"));

	/*
	 * Compute threshold for the noise mode as the minimum c[0] + thresholding
	 */
	noise_threshold = vector[0][0];
	for (i = 0; i < numvecs; ++i)
		if (vector[i][0] < noise_threshold)
			noise_threshold = vector[i][0];
	noise_threshold += noise_width;
#ifdef DEBUG
	printf("Noise threshold = %f\n", noise_threshold);
#endif

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
	printf
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
	if (initialize) {
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


		vector_quantize(mean, Nmodes, vector, numspch, Ndim, bin);

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
		    (initialcodebk, &mean, &variance, &c, &Nmodes, Ndim))
			QUIT(("Unable to read initial distribution\n"));
	}

	for (superiter = 0; superiter < 1; ++superiter) {
		estimate_multi_modals(vector, numspch, Ndim, Nmodes, mean,
				      variance, c, tempfile, 9);
		if (store_distribution
		    (outfile, Nmodes, Ndim, noisec, noisemean, noisevar, c,
		     mean, variance) != 0) {
			printf("Unable to open %s to store distribution\n",
			       outfile);
			printf("Superiter = %d\n", superiter);
		}
	}

	ckd_free_2d((void **)vector);
	ckd_free(c);
	ckd_free_2d((void **)mean);
	ckd_free_2d((void **)variance);
	return 0;
}
