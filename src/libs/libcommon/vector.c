/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: vector.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/vector.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

vector_t
vector_alloc(int32 n_dim)
{
    return (vector_t)ckd_calloc(n_dim, sizeof(float32));
}

int
vector_free(vector_t v)
{
    ckd_free(v);

    return S3_SUCCESS;
}

void
vector_floor(vector_t v, uint32 dim, float32 f)
{
    unsigned int i;

    for (i = 0; i < dim; i++)
	if (v[i] < f) v[i] = f;
}

void
vector_nz_floor(vector_t v, uint32 dim, float32 f)
{
    unsigned int i;

    for (i = 0; i < dim; i++)
	if ((v[i] != 0) && (v[i] < f)) v[i] = f;
}

int
vector_normalize(vector_t v,
		 uint32 dim)
{
    uint32 i;
    float64 s;
    int ret_val = S3_SUCCESS;

    for (i = 0, s = 0; i < dim; i++)
	s += v[i];

    if (s != 0) {
	for (i = 0; i < dim; i++)
	    v[i] /= s;
    }
    else {
	ret_val = S3_WARNING;
    }

    return ret_val;
}

void
vector_print(vector_t v,
	     uint32 dim)
{
    uint32 i;
    float64 s;
    
    for (i = 0, s = 0; i < dim; i++)
	printf("%e ", v[i]);
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.6  96/10/30  07:43:33  eht
 * Removed warning message that was just cluttering up the output
 * 
 * Revision 1.5  95/10/10  17:37:33  eht
 * Fixed some bogus return values
 * 
 * Revision 1.4  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.3  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.2  1995/06/02  14:52:54  eht
 * Use PWP's error package
 *
 * Revision 1.1  1995/02/13  15:48:25  eht
 * Initial revision
 *
 *
 */
