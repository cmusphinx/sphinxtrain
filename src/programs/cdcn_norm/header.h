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

#ifndef __CDCN_NORM_HEADER_H__
#define __CDCN_NORM_HEADER_H__
#include <s3/common.h>

int readdistribution(char *filename, float ***mean,
		     float ***vars, float **prob, int *ncodes, int *Ndim);

float **two_D_ize(float *buff, int length, int dim);

float norm_utt(float **z, float **x, int num_frames, float **variance, 
	       float *prob, float **mean, int num_codes, int Ndim);

void resfft(float *x, int n, int m);

void rsfft(float *x, int n, int m);

float initialize(float **data, int num_frames, float *noise, float *tilt, 
		 float speech_threshold, float **mean,  float *prob, 
		 float **var, int ncodes, int Ndim);

float dist(float *x, float *y, float *variance, int Ndim);

void correction(float *tilt, float *noise, float **mean, float **corrbook, 
		int num_codes, int Ndim);

float max_q(float **variance, float *prob, float *noise, float *tilt, 
	    float **mean, float **corrbook, int num_codes, float **z, 
	    int num_frames, int Ndim);

void mmse_x(float **variance, float *prob, float *tilt, float *noise, 
	    float **means, float **corrbook, int num_codes, float **z, 
	    float **x, int num_frames, int Ndim);

#endif /* __CDCN_NORM_HEADER_H__ */
