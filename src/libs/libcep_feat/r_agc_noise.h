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
 * File: r_agc_noise.h
 * 
 * Description: 
 * 
 * Author: 
 * 	faa
 *********************************************************************/

#ifndef R_AGC_NOISE_H
#define R_AGC_NOISE_H

#include <s3/prim_type.h>

void
real_agc_noise (float *cep,		/* The cepstrum data */
		uint32 fcnt,	/* Number of cepstrum frame availiable */
		uint32 cf_cnt);	/* Number of coeff's per frame */

/* batch silence compression routines */
int
delete_background (float32 *cep,	/* The cepstrum data */
		   int32 fcnt,		/* Number of cepstrum frame availiable */
		   int32 cf_cnt,	/* Number of coeff's per frame */
		   float32 thresh);	/* background threshold */

float
histo_noise_level (float *cep,	/* The cepstrum data */
		   int32 fcnt,	/* Number of cepstrum frame availiable */
		   int32 cf_cnt);	/* Number of coeff's per frame */

/* live silence compression routines */

int32
histo_add_c0 (float32 c0);

void
compute_noise_level (void);

#endif /* R_AGC_NOISE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:47:05  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1996/01/30  17:09:59  eht
 * Initial revision
 *
 * Revision 1.3  1995/10/12  17:43:36  eht
 * Add read_agc_noise() prototype.
 *
 * Revision 1.2  1995/10/10  12:35:52  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:45:04  eht
 * Initial revision
 *
 *
 */
