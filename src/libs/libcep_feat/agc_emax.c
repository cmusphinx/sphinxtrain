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
 * File: agc_emax.c
 * 
 * Description: 
 * 	Approximates the agc_max algorithm using a running estimate
 *	of the max c[0] value.  The estimate of the max decays over
 *	time to cope with changing signal power.
 *
 * Author: 
 * 	faa
 *********************************************************************/

#include <s3/agc_emax.h>
#include <s3/prim_type.h>

#include <string.h>


int agc_emax_proc (float32 *ocep,		/* ouput cepstrum frame */
		   float32 *icep,		/* input cepstrum frame */
		   uint32 cf_cnt)	/* Number of coeff's per frame */
{
    static float64 target_max = 1.0;
    static float64 max = 1.0;
    static float64 delta_max = 0.0;
    static float64 decay = 1.0/3000;
    static float64 min_max = -0.5;
  
    
    if (icep[0] > target_max) {
    	target_max = icep[0];
	if (delta_max < ((target_max - max) / 100.0))
	    delta_max = ((target_max - max) / 100.0);
    }

    if (target_max >= max) {
        max += delta_max;
    }
    else {
        if (target_max > min_max)
	    target_max -= decay;
	if (max > min_max)
	     max -= decay;
    }
    
    icep[0] -= max;
    
    memcpy((char *)ocep, (char *)icep, sizeof(float)*cf_cnt);

    return 1;
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
