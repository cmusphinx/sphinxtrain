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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>
#include <s3/acmod_set.h>
#include <s3/vector.h>
#include "segdmp.h"

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
		  segdmp_type_t data_type,
		  uint32 n_stream,
		  uint32 *vecsize,
		  uint32 blksize
		  );

int
segdmp_close(void);

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
		 segdmp_type_t *data_type, 
		 uint32 n_stream,
		 uint32 *vecsize,
		 uint32 blksize);

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


#ifdef __cplusplus
}
#endif
#endif /* SEGDMP_H */ 
