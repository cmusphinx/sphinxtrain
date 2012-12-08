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
 * File: backward.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef BACKWARD_H
#define BACKWARD_H

#include <sphinxbase/prim_type.h>
#include <sphinxbase/feat.h>

#include <s3/vector.h>
#include <s3/state.h>
#include <s3/model_inventory.h>

#include "baum_welch.h"

int32
backward_update(float64 **active_alpha,
		uint32 **active_astate,
		uint32 *n_active_astate,
		float64 *scale,
		float64 **dscale,
		vector_t **feature,
		uint32 n_obs,
		state_t *state_seq,
		uint32 n_state,
		model_inventory_t *inv,
		float64 beam,
		float32 spthresh,
		int32 mixw_reest,
		int32 tmat_reest,
		int32 mean_reest,
		int32 var_reest,
		int32 pass2var,
		int32 var_is_full,
		FILE *pdumpfn,
		bw_timers_t *timers,
		feat_t *fcb);

void
partial_op(float64 *p_op,
	   float64 op,

	   float64 **den,
	   uint32  **den_idx,
	   float32 **mixw,

	   uint32 n_feat,
	   uint32 n_top);

void
partial_ci_op(float64 *p_op,

	      float64 **den,
	      uint32  **den_idx,
	      float32 **mixw,

	      uint32 n_feat,
	      uint32 n_top);

void
den_terms_ci(float64 **d_term,

	     float64 post_j,
	     float64 *p_ci_op,
	     float64 **den,
	     uint32  **den_idx,
	     float32 **mixw,
	     uint32 n_feat,
	     uint32 n_top);

void
den_terms(float64 **d_term,
	  float64 p_reest_term,
	  float64 *p_op,
	  float64 **den,
	  uint32  **den_idx,
	  float32 **mixw,
	  uint32 n_feat,
	  uint32 n_top);

#endif /* BACKWARD_H */ 

