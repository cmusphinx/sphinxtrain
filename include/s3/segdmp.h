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
 * File: segdmp.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef SEGDMP_H
#define SEGDMP_H

#include <s3/prim_type.h>
#include <s3/acmod_set.h>
#include <s3/vector.h>

typedef enum {
    SEGDMP_TYPE_VQ,
    SEGDMP_TYPE_MFCC,
    SEGDMP_TYPE_FEAT
} segdmp_type_t;

/*
 * Segment dump creation/write
 */
int
segdmp_open_write(const char **dirs,		/* dump directories */
		  const char *dmp_fn,		/* dump file name */
		  const char *idx_fn,		/* index file name */
		  uint32 n_id,		/* # of id's */
		  uint32 *n_seg,		/* # of segments per id */
		  uint32 **n_frame,		/* # of frames per segment */
		  segdmp_type_t data_type);

int
segdmp_close(void);

/*
 * Append frames to the segment dump
 */

int
segdmp_add_vq(acmod_id_t p,
	      unsigned char *ccode,
	      unsigned char *dcode,
	      unsigned char *pcode,
	      unsigned char *ddcode,
	      uint32 n_frame);

int
segdmp_add_mfcc(acmod_id_t p,
		vector_t *mfcc,
		uint32 n_frame,
		uint32 veclen);

int
segdmp_add_feat(acmod_id_t p,
		vector_t **feat,
		uint32 n_frame);

/*
 * Segment dump reading
 */
int
segdmp_open_read(const char **dirs,		/* dump directories */
		 const char *dmp_fn,		/* dump file name */
		 const char *idx_fn,		/* index file name */
		 uint32 *n_id,			/* # of id's */
		 segdmp_type_t *data_type);

/*
 * Get data from the segment dump
 */

int
segdmp_next_vq(acmod_id_t p,
	       unsigned char **out_ccode,
	       unsigned char **out_dcode,
	       unsigned char **out_pcode,
	       unsigned char **out_ddcode,
	       uint32 *out_len);

int
segdmp_next_mfcc(acmod_id_t p,
		 vector_t **out_mfcc,
		 uint32 *out_n_frame,
		 uint32 *out_veclen);

int
segdmp_next_feat(acmod_id_t p,
		 vector_t ***out_feat,	/* use feat_*() routines to find out other dims */
		 uint32 *out_n_frame);

/*
 * Segment dump state query calls
 */

uint32
segdmp_n_seg(uint32 id);

/*
 * Misc
 */
void
segdmp_set_bufsz(uint32 sz_in_meg);


#endif /* SEGDMP_H */ 


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
 * Revision 1.1  2000/09/24 21:38:30  awb
 * *** empty log message ***
 *
 * Revision 1.3  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.2  96/07/29  16:33:40  eht
 * n_stream and vecsize determined at initialization time
 * 
 * Revision 1.1  1996/03/25  15:52:35  eht
 * Initial revision
 *
 *
 */
