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
 * File: accum.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef ACCUM_H
#define ACCUM_H

#include <s3/prim_type.h>
#include <s3/vector.h>
#include <s3/gauden.h>
#include <s3/model_inventory.h>
#include <s3/state.h>

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
	     float32 ****spkr_xfrm_a,
	     float32  ***spkr_xfrm_b,
	     uint32 ***den_idx,
	     gauden_t *g,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 pass2var,
	     float32 ***wacc);

int32
accum_global(model_inventory_t *inv,
	     state_t *state,
	     uint32 n_state,
	     int32 mixw_reest,
	     int32 tmat_reest,
	     int32 mean_reest,
	     int32 var_reest,
	     int32 mllr_mult,
 	     int32 mllr_add);

int32
accum_dump(const char *out_dir,
	   model_inventory_t *inv,
	   int32 mixw_reest,
	   int32 tmat_reest,
	   int32 mean_reest,
	   int32 var_reest,
	   int32 pass2var,
	   int32 mllr_mult,
	   int32 mllr_add,
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

int
accum_regmat(float32 ***cbacc,
	     model_inventory_t *inv,
	     vector_t *frame,
	     uint32 ***den_idx);

void
accum_global_gauden(vector_t ***acc,
		    vector_t ***l_acc,
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

void
accum_global_regmat(float32 ****regr_acc,
                    float32 ****l_regr_acc,
                    float32 *****regl_acc,
                    float32 *****l_regl_acc,
                    gauden_t *g);
#endif /* ACCUM_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.9  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.8  1996/07/29  16:21:12  eht
 * Got rid of accum_trans() and accum_non_emit_trans() and added
 * accum_den_terms()
 *
 * Revision 1.7  1996/03/26  15:17:51  eht
 * Fix beam definition bug
 *
 * Revision 1.6  1996/01/30  17:12:27  eht
 * Include a CI codebook (mixture Gaussian) weighted vector accumulators
 *
 * Revision 1.5  1995/11/30  20:52:00  eht
 * Allow tmat_reest to be passed as an argument
 *
 * Revision 1.4  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.3  1995/10/10  12:44:06  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/08/09  20:19:19  eht
 * Add mixing weight id argument so that error output count
 * be more informative
 *
 * Revision 1.1  1995/06/02  20:41:22  eht
 * Initial revision
 *
 *
 */
