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
 * File: agc_emax.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 	Does AGC using an estimate of the max power of the utterance
 *	based on decayed max computed from prior utterances.
 * 
 * Author: 
 * 	faa
 *********************************************************************/

#ifndef AGC_EMAX_H
#define AGC_EMAX_H

#include <s3/prim_type.h>

int agc_emax_proc (float32 *ocep,		/* ouput cepstrum frame */
		   float32 *icep,		/* input cepstrum frame */
		   uint32 cf_cnt);	/* Number of coeff's per frame */

#endif /* AGC_EMAX_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/10  12:35:52  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:45:04  eht
 * Initial revision
 *
 *
 */
