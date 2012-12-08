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
 * File: viterbi.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef VITERBI_H
#define VITERBI_H

#include <sphinxbase/prim_type.h>
#include <sphinxbase/feat.h>

#include <s3/vector.h>
#include <s3/state.h>
#include <s3/model_inventory.h>
#include <s3/s3phseg_io.h>

#include "baum_welch.h"

int32
write_phseg(const char *filename,
	    model_inventory_t *modinv,
	    state_t *state_seq,
	    uint32 **active_astate,
	    uint32 *n_active_astate,
	    uint32 n_state,
	    uint32 n_obs,
	    float64 **active_alpha,
	    float64 *scale,
	    uint32 **bp);

int32
write_s2stseg(const char *filename,
	      state_t *state_seq,
	      uint32 **active_astate,
	      uint32 *n_active_astate,
	      uint32 n_state,
	      uint32 n_obs,
	      uint32 **bp);

int32
viterbi_update(float64 *log_forw_prob,
	       vector_t **feature,
	       uint32 n_obs,
	       state_t *state,
	       uint32 n_state,
	       model_inventory_t *inv,
	       float64 a_beam,
	       float32 spthresh,
	       s3phseg_t *phseg,
	       int32 mixw_reest,
	       int32 tmat_reest,
	       int32 mean_reest,
	       int32 var_reest,
	       int32 pass2var,
	       int32 var_is_full,
	       FILE *pdumpfh,
	       bw_timers_t *timers,
	       feat_t *fcb);

int32
mmi_viterbi_run(float64 *log_forw_prob,
		vector_t **feature,
		uint32 n_obs,
		state_t *state,
		uint32 n_state,
		model_inventory_t *inv,
		float64 a_beam);

int32
mmi_viterbi_update(vector_t **feature,
		   uint32 n_obs,
		   state_t *state,
		   uint32 n_state,
		   model_inventory_t *inv,
		   float64 a_beam,
		   int32 mean_reest,
		   int32 var_reest,
		   float64 arc_gamma,
		   feat_t *fcb);

#endif /* VITERBI_H */ 

