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
#include <stdlib.h>
#include "header.h"

int read_backup_distribution(const char *filen,
			     float ***mean,
			     float ***variance,
			     float **pi, int *Nmodes, int Ndim)
{
	int j, k, imodes, idim;
	float **varbuff, **meanbuff, *cbuff;

	FILE *dist;

	if ((dist = fopen(filen, "r")) == NULL) {
		printf("Unable to open distribution file\n");
		return (0);
	}

	fscanf(dist, "%d %d", &imodes, &idim);

	*Nmodes = imodes;

	if (idim != Ndim) {
		printf("ERROR in read_distribution:\n");
		printf
		    ("Distribution dimensionality != Dimensionality of data given\n");
		return (0);
	}

	varbuff = (float **) ckd_calloc_2d(imodes, idim, sizeof(float));
	meanbuff = (float **) ckd_calloc_2d(imodes, idim, sizeof(float));
	cbuff = (float *) ckd_calloc(imodes, sizeof(float));

	for (k = 0; k < imodes; ++k) {
		fscanf(dist, "%f", &cbuff[k]);
		for (j = 0; j < idim; ++j)
			fscanf(dist, "%f ", &meanbuff[k][j]);
		for (j = 0; j < idim; ++j)
			fscanf(dist, "%f ", &varbuff[k][j]);
	}
	fclose(dist);

	*variance = varbuff;
	*mean = meanbuff;
	*pi = cbuff;

	return (1);
}
