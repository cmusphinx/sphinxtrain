/* ====================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All rights 
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
 * File: s3phseg_io.h
 * 
 * Description: 
 *     SPHINX-III phone segmentation file I/O functions
 *
 * Author: 
 *     David Huggins-Daines (dhuggins@cs.cmu.edu)
 *********************************************************************/

#ifndef S3PHSEG_IO_H
#define S3PHSEG_IO_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>
#include <s3/acmod_set.h>

typedef struct s3phseg_s {
    acmod_id_t phone;		/* phone id */
    uint32 sf, ef;		/* Start and end frame for this phone occurrence */
    int32 score;		/* Acoustic score for this segment of alignment */
    int32 tscore;		/* Transition ("LM") score for this segment */
    struct s3phseg_s *next;	/* Next entry in alignment */
} s3phseg_t;

typedef struct s3lattice_s {
  uint32 n_arcs;                /* total number of arcs in lattice */
  uint32 n_true_arcs;           /* the number of arcs from the numerator lattice */
  float64 prob;                 /* total log likelihood of lattice=alpha(Q)=beta(1) */
  float64 postprob;             /* the log posterior probability of the true path */
  struct s3arc_s *arc;          /* word arcs */
} s3lattice_t;

typedef struct s3arc_s {
  char word[128];                   /* current word */
  uint32 sf, ef;                    /* start and end frame for this word occurrence */
  uint32 n_prev_arcs, n_next_arcs;  /* number of preceding and succeeding arcs */
  float64 lm_score, ac_score;       /* language model score and acoustic score */
  float64 alpha, beta, gamma;       /* lattice level statistics accumulator */
  uint32 best_prev_arc, best_next_arc;        /* the prev and next arc id with the best ac score */
  uint32 *prev_arcs;                /* previous acrs */
  uint32 *next_arcs;                /* next arcs */
  uint32 good_arc;
} s3arc_t;

int s3phseg_read(const char *fn,
		 acmod_set_t *acmod_set,
		 s3phseg_t **out_phseg);

int s3phseg_write(const char *fn,
		  acmod_set_t *acmod_set,
		  s3phseg_t *phseg);

void s3phseg_free(s3phseg_t *phseg);

int s3lattice_read(const char *fn,
		   s3lattice_t **lattice);

#ifdef __cplusplus
}
#endif
#endif /* S3PHSEG_IO_H */
