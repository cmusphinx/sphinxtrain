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
 * File: corpus.h
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef CORPUS_H
#define CORPUS_H

#include <s3/prim_type.h>
#include <s3/vector.h>

#include <stdio.h>
#include <stddef.h>

/* MFCC directory/extension configuration
   functions */
int
corpus_set_mfcc_dir(const char *root);

int
corpus_set_mfcc_ext(const char *ext);

/* seg file configuration functions */
int
corpus_set_seg_dir(const char *root);

int
corpus_set_seg_ext(const char *ext);

/* sent file configuration functions */
int
corpus_set_sent_dir(const char *root);

int
corpus_set_sent_ext(const char *ext);

/* Vector quantized feature functions */
int
corpus_set_ccode_dir(const char *root);

int
corpus_set_ccode_ext(const char *ext);

int
corpus_set_dcode_dir(const char *root);

int
corpus_set_dcode_ext(const char *ext);

int
corpus_set_pcode_dir(const char *root);

int
corpus_set_pcode_ext(const char *ext);

int
corpus_set_ddcode_dir(const char *root);

int
corpus_set_ddcode_ext(const char *ext);

/* Corpus LSN file configuration functions */
int
corpus_set_lsn_filename(const char *fn);

/* Corpus silence frame deletion */
int
corpus_set_sildel_filename(const char *fn);

/* Per utterance MLLR transforms */
int
corpus_set_mllr_filename(const char *fn);
int
corpus_set_mllr_dir(const char *fn);

/* checkpoint the corpus module
   (i.e. write out offset and remaining run length) */
int
corpus_ckpt(const char *fn);


int
corpus_set_ctl_host(char *host_port_spec);

/* Control file configuration functions */
int
corpus_set_ctl_filename(const char *filename);

int
corpus_set_interval(uint32 n_skip,
		    uint32 run_len);

/* set the offset and run length from a checkpoint file */
int
corpus_ckpt_set_interval(const char *fn);

int
corpus_set_partition(uint32 r,
		     uint32 of_s);

uint32
corpus_get_begin(void);

/* Initialization function to be called after
   configuration functions */

int
corpus_init(void);

/* After reaching the end of a (sub)corpus.  This
 * call sets things up as they were after corpus_init() */
int
corpus_reset(void);

/* data access/info functions */
int
corpus_next_utt(void);
char *
corpus_utt_full_name(void);
char *
corpus_utt_brief_name(void);
char *
corpus_utt(void);

int32
corpus_provides_sent(void);
int
corpus_get_sent(char **trans);

int
corpus_has_xfrm(void);
int
corpus_get_xfrm(float32 *****out_a,
		float32 ****out_b,
		const uint32 **out_veclen,
		uint32 *out_n_mllrcls,
		uint32 *out_n_stream);

int32
corpus_provides_mfcc(void);
int
corpus_get_mfcc(vector_t **mfc,
		uint32 *n_frame,
		uint32 *veclen);

int
corpus_get_generic_featurevec(vector_t **mfc,
                              uint32 *n_frame,
                              uint32 veclen);

int
corpus_get_sildel(uint32 **sf,
		  uint32 **ef,
		  uint32 *n_seg);

int32
corpus_provides_seg(void);
int
corpus_get_seg(uint16 **seg, uint32 *n_frame);

int32
corpus_provides_ccode(void);
int
corpus_get_ccode(unsigned char **ccode, uint32 *n_frame);
int32
corpus_provides_dcode(void);
int
corpus_get_dcode(unsigned char **dcode, uint32 *n_frame);
int32
corpus_provides_pcode(void);
int
corpus_get_pcode(unsigned char **pcode, uint32 *n_frame);
int32
corpus_provides_ddcode(void);
int
corpus_get_ddcode(unsigned char **ddcode, uint32 *n_frame);

#endif /* CORPUS_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  17:46:09  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:30  awb
 * *** empty log message ***
 *
 * Revision 1.10  97/07/18  14:04:42  eht
 * Added corpus_reset() call
 * 
 * Revision 1.9  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.8  1996/03/26  15:18:39  eht
 * Fix local function definition
 *
 * Revision 1.7  1996/03/25  15:50:59  eht
 * Added VQ code functions
 *
 * Revision 1.6  1996/03/05  14:22:29  eht
 * Added include of <stdio.h> so that FILE type is resolved
 *
 * Revision 1.5  1996/03/05  14:21:26  eht
 * Added ability to check whether the corpus module is
 * configured to provide each data type.
 *
 * Revision 1.4  1995/12/01  19:54:13  eht
 * Added prototype for corpus_get_seg()
 *
 * Revision 1.3  1995/12/01  17:59:57  eht
 * - Add ability to get state segmentation data for each utterance
 * - Add ability to skip certain data types if not interested in them
 * 	(e.g. a decoder is not necessarily interested in word transcript or
 * 		state segmentation data).
 * - Added corpus_utt() function to just get the utt id
 *
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/10/09  21:04:24  eht
 * Initial revision
 *
 * Revision 1.4  1995/09/08  19:13:52  eht
 * Updated to remove references to pset module and add references
 * to acmod_set module
 *
 * Revision 1.3  1995/08/29  12:21:00  eht
 * Interface change (part 2)
 *
 * Revision 1.2  1995/08/29  12:16:43  eht
 * Interface change
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
