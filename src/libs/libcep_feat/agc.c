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
 * File: agc.c
 * 
 * Description: 
 * 	This routine does various kinds of automatic gain control (AGC).
 *
 *	BEWARE: This is hardcoded to 13 order cep via S2_CEP_VECLEN.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <s3/agc.h>
#include <s3/agc_max.h>
#include <s3/agc_emax.h>
#include <s3/r_agc_noise.h>

#include <s3/cmd_ln.h>
#include <s3/s2_param.h>	/* for S2_CEP_VECLEN */
#include <s3/err.h>

#include <string.h>

static uint32 veclen;

void
agc_set_veclen(uint32 l)
{
    veclen = l;
}

void
agc(float32 *mfcc,
    uint32 n_frame)
{
    const char *agc_type = cmd_ln_access("-agc");
    uint32 i;

    if (strcmp(agc_type, "noise") == 0) {
	real_agc_noise(mfcc, n_frame, veclen);
    }
    else if (strcmp(agc_type, "max") == 0) {
	agc_max(mfcc, n_frame, veclen);
    }
    else if (strcmp(agc_type, "emax") == 0) {
	for (i = 0; i < n_frame; i++) {
	    agc_emax_proc(&mfcc[i*veclen], &mfcc[i*veclen],
			  veclen);
	}
    }
    else if (strcmp(agc_type, "none") == 0) {
	/* do nothing */
    }
    else if (agc_type == NULL) {
	E_WARN("no agc set\n");
	return ;
    }
    else {
	E_FATAL("unsupported agc type %s\n", agc_type);
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.2  1996/03/25  15:34:23  eht
 * Deal w/ variable length input feature (e.g. MFCC) vector sizes.
 *
 * Revision 1.1  1995/12/14  20:12:58  eht
 * Initial revision
 *
 *
 */
