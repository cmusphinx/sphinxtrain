/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * Revision 1.2  2000/09/29  22:35:12  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
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
