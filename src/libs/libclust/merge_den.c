/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: merge_den.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/merge_den.h>

#include <string.h>

void
merge_d(float32 ***opdf,
	uint32 *i2o,

	float32 ***ipdf,
	uint32 n_ipdf,

	uint32 n_stream,
	uint32 n_cw)
{
    uint32 o, i, j, k;

    for (i = 0; i < n_ipdf; i++) {
	o = i2o[i];

	for (j = 0; j < n_stream; j++) {
	    for (k = 0; k < n_cw; k++) {
		opdf[o][j][k] += ipdf[i][j][k];
	    }
	}
    }
}

void
interpolate_d(float32 ***opdf,
	      uint32 n_opdf,
	      float32 ***ipdf,
	      uint32 n_ipdf,
	      float32 *wt,
	      uint32 n_wt,
	      uint32 *i2wt,
	      uint32 *i2o,
	      uint32 n_stream,
	      uint32 n_codeword)
{
    uint32 o, i, j, k;
    float32 _wt;
    
    for (i = 0; i < n_ipdf; i++) {
	o = i2o[i];
	_wt = i2wt[i];
	for (j = 0; j < n_stream; j++) {
	    for (k = 0; k < n_codeword; k++) {
		opdf[o][j][k] += _wt * ipdf[i][j][k];
	    }
	}
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
