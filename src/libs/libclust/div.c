/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: div.c
 * 
 * Description: 
 *    This file contains a collection of entropy and divergence
 *    measures of discrete
 *    distributions.
 *
 *    This package expects that its arguments are acceptable to the
 *    log() package.  See individual pre-condition sections for the
 *    details for a given function.
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <math.h>

#include <s3/div.h>

#include <assert.h>

static float64 ln2lg2 = 1.442695041;	/* conversion factor for bits-of-info */

/*
 * Description:
 *	This routine computes the entropy of a given discrete
 *	pdf.
 *
 * Arguments:
 *	float32 *p
 *		A discrete pdf
 *	uint32   n
 *		The length of argument p.
 *
 * Returns:
 *	The entropy of the pdf p in bits.
 *
 * Pre-conditions:
 *	p[i] must be a valid arg to log() for all i in 0..(n-1)
 */
float64
ent_d(float32 *p, uint32 n)
{
    uint32 k;
    float64 y;

    for (k = 0, y = 0; k < n; k++) {
	assert(p[k] != 0);
	
	if (p[k] > 1e-8)
	    y -= p[k] * log(p[k]);
    }

    return y * ln2lg2;
}
/*
 * Description:
 *	This routine computes the entropy of a given discrete model
 *	state that may use multiple independent input streams.
 *
 * Arguments:
 *	float32 **p
 *		A discrete pdf for a model state
 *	uint32   f
 *		The # of independent feature streams
 *	uint32   n
 *		The # of codewords
 *
 * Returns:
 *	The entropy of the discrete state output probability p in bits.
 *
 * Pre-conditions:
 *	p[j][k] must be a valid arg to log() for all j in 0..(f-1),
 *		k in 0..(n-1)
 *
 */

float64
ent_d_state(float32 **p, uint32 f, uint32 n)
{
    uint32 j;
    float64 y;

    for (j = 0, y = 0; j < f; j++) {
	y += ent_d(p[j], n);
    }

    return y;
}

/*
 * Description:
 *	This routine computes the directed divergence (similarity metric)
 *	of two discrete pdf's.
 *
 * Arguments:
 *	float32 *p
 *		A discrete pdf
 *	float32 *q
 *		A discrete pdf
 *	uint32   n
 *		The length of arguments p and q.
 *
 * Returns:
 *	The directed divergence of p and q.
 *
 * Pre-conditions:
 *	p[i]/q[i] must be a valid arg to log() for all i in 0..(n-1)
 */
float64
ddiv_d(float32 *p, float32 *q, uint32 n)
{
    uint32 k;
    float64 y;

    for (k = 0, y = 0; k < n; k++) {
	assert(q[k] != 0);

	if (p[k] > 0)
	    y += p[k] * log(p[k] / q[k]);
    }

    return y * ln2lg2;
}

/*
 * Description:
 *	This routine computes a symmetric divergence (similarity metric)
 *	of two discrete pdf's.
 *
 * Arguments:
 *	float32 *p
 *		A discrete pdf
 *	float32 *q
 *		A discrete pdf
 *	uint32   n
 *		The length of arguments p and q.
 *
 * Returns:
 *	A symmetric divergence of p and q.
 *
 * Pre-conditions:
 *	p[i]/q[i] must be a valid arg to log() for all i in 0..(n-1)
 */
float64
div_d(float32 *p, float32 *q, uint32 n)
{
    return ddiv_d(p, q, n) + ddiv_d(q, p, n);
}

/*
 * Description:
 *	This routine computes a symmetric divergence (similarity metric)
 *	of two discrete output probabilities.
 *
 * Arguments:
 *	float32 *p
 *		A discrete pdf
 *	float32 *q
 *		A discrete pdf
 *	uint32   f
 *		The # of independent feature streams
 *	uint32   n
 *		The # of codewords
 *
 * Returns:
 *	A symmetric divergence of output probabilities p and q.
 *
 * Pre-conditions:
 *	p[j][k]/q[j][k] must be a valid arg to log() for all j in 0..(f-1),
 *		and k in 0..(n-1),
 */
float64
div_d_state(float32 **p, float32 **q, uint32 f, uint32 n)
{
    uint32 j;
    float64 y;

    for (j = 0, y = 0; j < f; j++) {
	y += div_d(p[j], q[j], n);
    }

    return y;
}

/*
 * Description:
 *	This routine computes the directed divergence (similarity metric)
 *	of two discrete output probabilities.
 *
 * Arguments:
 *	float32 *p
 *		A discrete pdf
 *	float32 *q
 *		A discrete pdf
 *	uint32   f
 *		The # of independent feature streams
 *	uint32   n
 *		The # of codewords
 *
 * Returns:
 *	The directed divergence of output probabilities p and q.
 *
 * Pre-conditions:
 *	p[j][k]/q[j][k] must be a valid arg to log() for all j in 0..(f-1),
 *		and k in 0..(n-1),
 */
float64
ddiv_d_state(float32 **p, float32 **q, uint32 f, uint32 n)
{
    uint32 j;
    float64 y;

    for (j = 0, y = 0; j < f; j++) {
	y += ddiv_d(p[j], q[j], n);
    }

    return y;
}

/* ADDITION FOR CONTINUOUS_TREE HMM 19 May 98 - 
   Entropy/likelihood function for continuous gaussians PDFs */

/*
 * Description:
 *      This routine computes the entropy of a given gaussian pdf
 *      pdf.
 *
 * Arguments:
 *      float32 *mean
 *              Means - actually not used
 *      float32 *var
 *              Variances - diagonal only
 *      uint32   n
 *              The length of mean/var vectors
 *
 * Returns:
 *      The entropy of the pdf p in bit
 *
 */

#define HAFLN2PI  0.9189385332

float64
ent_cont(float32 *mean, float32 *var, uint32 n)
{
    uint32 k;
    float64 y;

    for (k = 0, y = 0; k < n; k++) {
        assert(var[k] != 0);
        y += log(var[k]);
    }
    y *= 0.5;
    y += (float64)n*(HAFLN2PI + 0.5);

    return(-y * ln2lg2); /* Convert to log base 2 */
}

/* END ADDITION FOR CONTINUOUS_TREE */

