/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: del_sil_seg.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$"; */

#include <s3/feat.h>
#include <s3/err.h>

#include <string.h>
#include <assert.h>

uint32
del_sil_seg(vector_t *mfcc,
	    uint32 n_frame_in,
	    uint32 *del_b,
	    uint32 *del_e,
	    uint32 n_del)
{
    uint32 n_del_frame;	/* # of deleted frames */
    uint32 n_frame;	/* # of frames output */
    uint32 i;		/* source frame index */
    uint32 j;		/* destination frame indext */
    uint32 d;		/* next deleted segment */
    uint32 ceplen;	/* # of components of the cepstrum vector */

    if (n_del == 0)
	return n_frame_in;
    
    for (i = 0, n_del_frame = 0; i < n_del; i++) {
	n_del_frame += del_e[i] - del_b[i] + 1;
    }

    ceplen = feat_mfcc_len();

    n_frame = n_frame_in - n_del_frame;

    for (i = 0, j = 0, d = 0; i < n_frame_in; i++, j++) {
	if ((d < n_del) && (i == del_b[d])) {
	    i = del_e[d] + 1;
	    ++d;
	}
	if ((i != j) && (i < n_frame_in))
	    memcpy(mfcc[j], mfcc[i], sizeof(float32) * ceplen);
    }

    if (d != n_del) {
	E_FATAL("d(%u) != n_del(%u)\n", d, n_del);
    }

    return n_frame;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
