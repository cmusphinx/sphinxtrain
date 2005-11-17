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
 * Normalizes cepstrum files.
 *
 * Coded by Alex Acero - 1990
 * Completely overhauled by Bhiksha: 1994
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "header.h"

#define DESCRIPTION "\nProgram to compensate speech cepstra files for the effect of noise and channel\nPerforms normalization on a bunch of noisy files given clean speech statistics\n(CDCN style statistics). The routine reads the noisy files from the input path\nand dumps the compensated files in the output path. The control files should\nNOT contain file extensions.\nNOTE: The program assumes that the dimension of the CDCN statistics and the\n    data are the same. If your cepstral files and the distribution have \n    different dimensionality the program will die a horrible death!!\n"

#define  USAGE "\nUSAGE:\n%s -c <control file> \\\n\t-i <path of noisy mfc files> \\\n\t-o <path to write compensated mfc files (default: inpath itself)> \\\n\t-x <extension of input mfc file names (default: 'mfc')> \\\n\t-y <extension to use on compensated mfc files (default: 'cdcn.mfc')> \\\n\t-d <file with CDCN distribution to use>\n"

#define  QUIT(x)  {printf x; fflush (stdout); exit(-1);}

int main(int argc, char **argv)
{
	float *fbuff,		/* A temporary buffer */
	**z,			/* The observed cepstrum vectors */
	**x,			/* The normalized cepstrum */
	**variance,		/* Variance of gaussians */
	*prob,			/* Ratio of a-priori prob. of modes to mod variance */
	 distortion,		/* The final distortion */
	**mean;			/* The means */

	int num_frames,		/* Number of frames in utterance */
	 num_codes,		/* Number of codewords in codebook */
	 num_utt,		/* Number of utterances in ctl-file */
	 Ndim,			/* Dimensionality of cepstra */
	 i;			/* Index for arguments */

	char ctl_file[1024],
	    code_file[1024],
	    buff[1024],
	    in_cepfile[1024], out_cepfile[1024],
	    infileext[64], outfileext[64], inpath[512], outpath[512], sw;


	FILE *fp;

	/* Default */
	strcpy(ctl_file, "");
	strcpy(code_file, "");
	strcpy(infileext, "mfc");
	strcpy(outfileext, "cdcn.mfc");
	strcpy(inpath, "");
	strcpy(outpath, "");

	/* Input interface */

	if (argc <= 1) {
		printf(DESCRIPTION);
		QUIT((USAGE, argv[0]));
	}

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-')
			QUIT((USAGE, argv[0]));
		sw = argv[i][1];
		switch (sw) {
		case 'c':
		case 'C':
			{
				strcpy(ctl_file, argv[++i]);
				break;
			}
		case 'x':
		case 'X':
			{
				strcpy(infileext, argv[++i]);
				break;
			}
		case 'y':
		case 'Y':
			{
				strcpy(outfileext, argv[++i]);
				break;
			}
		case 'i':
		case 'I':
			{
				strcpy(inpath, argv[++i]);
				break;
			}
		case 'o':
		case 'O':
			{
				strcpy(outpath, argv[++i]);
				break;
			}
		case 'd':
		case 'D':
			{
				strcpy(code_file, argv[++i]);
				break;
			}
		default:
			QUIT((USAGE, argv[0]));
		}
	}

	if (!strcmp(inpath, "") || !strcmp(code_file, "") ||
	    !strcmp(ctl_file, ""))
		QUIT((USAGE, argv[0]));

	if (!strcmp(outpath, ""))
		strcpy(outpath, inpath);

	printf
	    ("\n**************** USING FOLLOWING VALUES ******************\n");
	printf("Input path = %s\n", inpath);
	printf("Output path = %s\n", outpath);
	printf("Control file = %s\n", ctl_file);
	printf("Speech distribution = %s\n", code_file);
	printf("\n\n");

	/* Read codebook */
	readdistribution(code_file, &mean, &variance, &prob, &num_codes,
			 &Ndim);


	/* Read input cepstrum file and allocate memory */
	if ((fp = fopen(ctl_file, "r")) == NULL)
		QUIT(("Cannot open control file %s\n", ctl_file));

	for (num_utt = 0; fscanf(fp, "%s\n", buff) != EOF; num_utt++) {
		sprintf(in_cepfile, "%s/%s.%s", inpath, buff, infileext);

		if (areadfloat(in_cepfile, &fbuff, &num_frames) < 0) {
			continue;
		}
		num_frames /= Ndim;
		z = (float **) two_D_ize(fbuff, num_frames, Ndim);
		x = (float **) ckd_calloc_2d(num_frames, Ndim, sizeof(float));

		distortion = norm_utt(z, x, num_frames, variance, prob,
				      mean, num_codes, Ndim);
		printf("Distortion = %f\n", distortion);
		fflush(stdout);

		sprintf(out_cepfile, "%s/%s.%s", outpath, buff,
			outfileext);

		/* 
		 * Write file back 
		 * Note: Due the the structure of our 2D arrays x[0] in fact
		 * holds the address of the entire x array. Look at alloc2d.c
		 * to see what I mean
		 */
		awritefloat(out_cepfile, x[0], num_frames * Ndim);
		ckd_free_2d((void **)z);
		ckd_free_2d((void **)x);
	}
	printf("Totally, %d cepfiles have been processed (norm)\n",
	       num_utt);
	fflush(stdout);
	ckd_free_2d((void **)variance);
	ckd_free_2d((void **)mean);
	return 0;
}
