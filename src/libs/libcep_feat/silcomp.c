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
 * File: silcomp.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include "del_sil_seg.h"

#include <s3/silcomp.h>
#include <s3/r_agc_noise.h>
#include <s3/corpus.h>
#include <s3/s2_param.h>
#include <s3/err.h>

#include <string.h>

static uint32 veclen;

static uint32 *del_b = NULL;
static uint32 *del_e = NULL;
static uint32 n_del = 0;

void
silcomp_set_del_seg(uint32 *b,
		    uint32 *e,
		    uint32 n)
{
    del_b = b;
    del_e = e;
    n_del = n;
}
    
void
silcomp_set_veclen(uint32 l)
{
    veclen = l;
}

uint32
sil_compression(const char *type,
		vector_t *mfcc,
		uint32 in_n_frame)
{
    uint32 n_frame;

    if (strcmp(type, "current") == 0) {
	n_frame = delete_background (mfcc[0], in_n_frame, veclen,
				     histo_noise_level (mfcc[0],
							in_n_frame,
							veclen));
	
	return n_frame;
    }
    else if (strcmp(type, "sildelfn") == 0) {
	n_frame = del_sil_seg(mfcc, in_n_frame,
			      del_b, del_e, n_del);

	return n_frame;
    }
    else if (strcmp(type, "none") == 0) {
	return in_n_frame;
    }
    else {
	E_FATAL("Unsupported silence compression type %s\n", type);
    }

    return in_n_frame;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1996/08/05  12:50:09  eht
 * Include <string.h> and <s3/r_agc_noise.h> for prototypes
 *
 * Revision 1.2  1996/03/25  15:36:31  eht
 * Changes to allow for settable input feature length
 *
 * Revision 1.1  1995/12/14  20:12:03  eht
 * Initial revision
 *
 *
 */
