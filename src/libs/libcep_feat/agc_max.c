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
