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
#include <stdio.h>
#include "header.h"


int store_distribution(const char *file,
		       int Nmodes,
		       int Ndim,
		       float noisec,
		       float *noisemean,
		       float *noisevar,
		       float *c, float **mean, float **var)
{
	int i, j;

	FILE *outputfile;

	/*
	 * We need to adjust for noise mode
	 * Therefore, the actual number of modes in the distribution
	 * (including the noise mode) is Nmodes + 1 at this point.
	 */

	if ((outputfile = fopen(file, "w")) == NULL)
		return (-1);

	fprintf(outputfile, "%d %d\n", Nmodes + 1, Ndim);
	fprintf(outputfile, "%f\n", noisec);
	for (j = 0; j < Ndim; ++j)
		fprintf(outputfile, "%f ", noisemean[j]);
	fprintf(outputfile, "\n");
	for (j = 0; j < Ndim; ++j)
		fprintf(outputfile, "%f ", noisevar[j]);
	fprintf(outputfile, "\n");
	/*
	 * The variance is printed for each mode to maintain uniformity of 
	 * distribution file structure for other algorithms we use
	 */
	for (i = 0; i < Nmodes; ++i) {
		fprintf(outputfile, "%f\n", (1.0 - noisec) * c[i]);
		for (j = 0; j < Ndim; ++j)
			fprintf(outputfile, "%f ", mean[i][j]);
		fprintf(outputfile, "\n");
		for (j = 0; j < Ndim; ++j)
			fprintf(outputfile, "%f ", var[i][j]);
		fprintf(outputfile, "\n");
	}
	fclose(outputfile);
	printf("Wrote distribution to %s\n", file);
	fflush(stdout);
	return (0);
}
