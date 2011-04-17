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
 * File: accum.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef ACCUM_H
#define ACCUM_H

#include <stdio.h>

#include <s3/vector.h>
#include <s3/gauden.h>
#include <s3/model_inventory.h>
#include <s3/state.h>

#include <sphinxbase/prim_type.h>
#include <sphinxbase/feat.h>

void
accum_den_terms(float32 **acc,
		float64 **den_terms,
		uint32 **den_idx,
		uint32 n_feat,
		uint32 n_top);

int32
accum_gauden(float32 ***cbacc,
	     uint32 *lcl2gbl,
	     uint32 n_lcl2gbl,
	     vector_t *frame,
	     uint32 ***den_idx,
	     gauden_t *g,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 pass2var,
	     float32 ***wacc,
	     int32 var_is_full,
	     FILE *pdumpfh,
	     feat_t *fcb);

int32
accum_global(model_inventory_t *inv,
	     state_t *state,
	     uint32 n_state,
	     int32 mixw_reest,
	     int32 tmat_reest,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 var_is_full);

int32
accum_dump(const char *out_dir,
	   model_inventory_t *inv,
	   int32 mixw_reest,
	   int32 tmat_reest,
	   int32 mean_reest,
	   int32 var_reest,
	   int32 pass2var,
	   int32 var_is_full,
	   int ckpt);

int32
accum_viterbi(uint32 *vit_sseq,
	      uint32 n_vit_sseq,
	      state_t *state,
	      vector_t **obs,
	      uint32 n_obs,
	      model_inventory_t *inv,
	      float64 ****den,
	      uint32 ****den_idx,
	      int32 mixw_reest,
	      int32 tmat_reest,
	      int32 mean_reest,
	      int32 var_reest);

void
accum_global_gauden(vector_t ***acc,
		    vector_t ***l_acc,
		    gauden_t *g,
		    uint32 *lcl2glb,
		    uint32 n_lcl2glb);
void
accum_global_gauden_full(vector_t ****acc,
			 vector_t ****l_acc,
			 gauden_t *g,
			 uint32 *lcl2glb,
			 uint32 n_lcl2glb);
void
accum_global_gauden_dnom(float32 ***dnom,
			 float32 ***l_dnom,
			 gauden_t *g,
			 uint32 *lcl2glb,
			 uint32 n_lcl2glb);

void
accum_global_mixw(model_inventory_t *inv, gauden_t *g);

void
accum_global_tmat(model_inventory_t *inv,
		  state_t *state,
		  uint32 n_state);

int32
accum_mmie_dump(const char *out_dir,
		const char *lat_ext,
		model_inventory_t *inv,
		int32 mean_reest,
		int32 var_reest);

int32
mmi_accum_gauden(float32 ***cbacc,
		 uint32 *lcl2gbl,
		 uint32 n_lcl2gbl,
		 vector_t *frame,
		 uint32 ***den_idx,
		 gauden_t *g,
		 int32 mean_reest,
		 int32 var_reest,
		 float64 arc_gamma,
		 feat_t *fcb);

#endif /* ACCUM_H */ 
