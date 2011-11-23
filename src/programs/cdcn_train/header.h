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
#ifndef __CDCN_HEADER_H__
#define __CDCN_HEADER_H__

/*#define maxdim 13 */
#define maxdim 40
#define maxmodes 257
#define PI	3.1415927

#include <s3/common.h>
#include <s3/s3.h>

float gauss(float *x,
	    float *mean, float *hafinvvar, float corprod, int Ndim);

void vector_quantize(float **codes, int Ncodes,
		     float **vector, int Nvecs,
		     int Ndim, int *bin,
		     int niter, float threshold);

int read_backup_distribution(const char *filen,
			     float ***mean,
			     float ***variance,
			     float **pi, int *Nmodes, int Ndim);

int store_distribution(const char *file,
		       int Nmodes,
		       int Ndim,
		       float noisec,
		       float *noisemean,
		       float *noisevar,
		       float *c, float **mean, float **var);

void estimate_multi_modals(float **x, int N, int Ndim, int K, 
			   float **mean, float **var, float *c, 
			   const char *tempfile, int numiters,
			   float Threshold);



#endif /* __CDCN_HEADER_H__ */
