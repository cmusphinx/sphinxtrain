/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * 4. Redistributions of any form whatsoever must retain the following
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
 * File: cmn.c
 * 
 * Description: 
 * 	Do various types of cepstral mean normalization.
 *
 *	BEWARE: This is hardcoded to 12th order cep via S2_CEP_VECLEN.
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/cmn.h>
#include <s3/norm.h>
#include <s3/live_norm.h>

#include <s3/cmd_ln.h>
#include <s3/s2_param.h>
#include <s3/err.h>

#include <string.h>

static int veclen;

void
cmn_set_veclen(uint32 l)
{
    veclen = l;
}

void
cmn(float32 *mfcc,
    uint32 n_frame)
{
    const char *type = cmd_ln_access("-cmn");
    uint32 i;

    if (strcmp(type, "current") == 0) {
	norm_mean(mfcc, n_frame, veclen);
    }
    else if (strcmp(type, "prior") == 0) {
	for (i = 0; i < n_frame; i++) {
	    mean_norm_acc_sub(&mfcc[i*veclen]);
	}
	mean_norm_update();
    }
    else if (strcmp(type, "none") == 0) {
    }
    else {
	E_FATAL("Unsupported CMN type %s\n", type);
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.6  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.5  96/08/05  12:48:34  eht
 * Include norm.h and live_norm.h for prototypes
 * 
 * Revision 1.4  1996/08/05  12:44:31  eht
 * Include <string.h> for prototypes
 *
 * Revision 1.3  1996/08/05  12:43:56  eht
 * Fixed inconsistency w/ prototype
 *
 * Revision 1.2  1996/03/25  15:34:58  eht
 * Deal w/ configurable input feature vector length.
 *
 * Revision 1.1  1995/12/14  20:12:58  eht
 * Initial revision
 *
 *
 */
