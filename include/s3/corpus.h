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


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>
#include <s3/vector.h>
#include <s3/acmod_set.h>
#include <s3/s3phseg_io.h>
#include <s3/s3io.h>

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

/* phone seg configuration functions */
int
corpus_set_phseg_dir(const char *dir);

int
corpus_set_phseg_ext(const char *ext);

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
		uint32 **out_veclen,
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
                              int32 *n_frame,
                              uint32 veclen);

int
corpus_get_sildel(uint32 **sf,
		  uint32 **ef,
		  uint32 *n_seg);

int32
corpus_provides_seg(void);
int
corpus_get_seg(uint16 **seg, int32 *n_frame);

int32 corpus_provides_phseg(void);
int corpus_get_phseg(acmod_set_t *acmod_set,
		     s3phseg_t **out_phseg);

int
corpus_load_lattice(s3lattice_t **out_lattice, const char *lat_dir, const char *lat_ext);

#ifdef __cplusplus
}
#endif
#endif /* CORPUS_H */ 
