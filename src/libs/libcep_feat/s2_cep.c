/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: s2_cep.c
 * 
 * 	Extracts MFCC[1..12] out of MFCC[0..12]
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include "s2_cep.h"

#include <s3/s2_param.h>

#include <stdlib.h>
#include <assert.h>

static uint32 veclen = 13;

void
s2_cep_set_veclen(uint32 l)
{
    veclen = l;
}

void
s2_cep_feat(vector_t **out,
	    vector_t *mfcc,
	    uint32 n_frame)
{
    uint32 i, k;
    vector_t frm;

    for (i = 0; i < n_frame; i++) {
	frm = mfcc[i];

	out[i][S2_POW_FEATURE][0] = frm[0];

	for (k = 1; k < veclen; k++) {
	    out[i][S2_CEP_FEATURE][k-1] = frm[k];
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
 * Revision 1.2  1996/03/25  15:36:31  eht
 * Changes to allow for settable input feature length
 *
 * Revision 1.1  1995/12/14  20:12:58  eht
 * Initial revision
 *
 *
 */
