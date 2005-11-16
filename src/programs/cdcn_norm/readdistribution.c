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
/*---------------------------------------------------------------------------*
 * This routine reads the text file holding mean, variance and mode probablities
 * of the mixture density parameters computed by EM. It returns 
 * prob/sqrt(mod var) instead of var to simplify computation in CDCN
 * Coded by Bhiksha 
 ----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "header.h"

#define QUIT(x) {printf x; fflush (stdout); exit(-1);}

int readdistribution(char *filename,
		     float ***mean,
		     float ***vars, float **prob, int *ncodes, int *Ndim)
{
	int i, j, ndim;

	float **meanbuff;
	float **varbuff;
	float *probbuff;
	float temp;

	FILE *codefile;

	if ((codefile = fopen(filename, "r")) == NULL)
		QUIT(("Unable to open Codebook file\n"));

	fscanf(codefile, "%d %d", ncodes, &ndim);
	meanbuff = (float **) ckd_calloc_2d((*ncodes), ndim, sizeof(float));
	varbuff = (float **) ckd_calloc_2d((*ncodes), ndim, sizeof(float));
	probbuff = (float *) ckd_calloc((*ncodes), sizeof(float));

	for (i = 0; i < (*ncodes); ++i) {
		fscanf(codefile, "%f", &probbuff[i]);
		for (j = 0; j < ndim; ++j)
			fscanf(codefile, "%f", &meanbuff[i][j]);

		temp = 1;
		for (j = 0; j < ndim; ++j) {
			fscanf(codefile, "%f", &varbuff[i][j]);
			temp *= sqrt(varbuff[i][j]);
		}
		probbuff[i] /= temp;
	}

	fclose(codefile);
	*mean = meanbuff;
	*vars = varbuff;
	*prob = probbuff;
	*Ndim = ndim;

	return (0);
}
