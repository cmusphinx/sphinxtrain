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
 * File: agc_max.c
 * 
 * Description: 
 *      Given a sequence of cepstral frames, find and subtract the
 *	max c[0] value.
 *
 * Author: 
 * 	faa
 *********************************************************************/


#include <s3/agc_max.h>
#include <s3/prim_type.h>


void
agc_max (float *cep,		/* The cepstrum data */
	 uint32 fcnt,	/* Number of cepstrum frame availiable */
	 uint32 cf_cnt)	/* Number of coeff's per frame */
{
    static char *rname = "agc_max";
    float32 *p;			/* Data pointer */
    float32 max_energy;		/* Minimum log-energy */
    uint32 i;		/* frame index */
    (void)rname;

    /* Determine max log-energy in utterance */
    max_energy = *cep;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
      if (*p > max_energy)
	max_energy = *p;
    }

#ifdef AGC_MAX_VERBOSE
    fflush(stdout);
    fprintf (stderr, "(max mfcc[0] == %6.3f)  ", max_energy);
    fflush(stderr);
#endif

    /* Subtract max_energy from all log_energy values
     */
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt)
      *p -= max_energy;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1995/10/17  13:05:04  eht
 * Cleaned up code a bit so that it is more ANSI compliant
 *
 * Revision 1.2  1995/10/10  12:36:12  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/06/02  20:57:22  eht
 * Initial revision
 *
 *
 */
