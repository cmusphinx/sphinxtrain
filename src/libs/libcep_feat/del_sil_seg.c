/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
