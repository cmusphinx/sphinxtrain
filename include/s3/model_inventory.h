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
 * File: model_inventory.h
 * 
 * Description: 
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#ifndef MODEL_INVENTORY_H
#define MODEL_INVENTORY_H

#include <s3/acmod_set.h>
#include <s3/gauden.h>
#include <s3/prim_type.h>
#include <s3/model_def.h>

/* data structure definition */

typedef struct {
    model_def_t    *mdef;	/* acoustic model definitions */
    acmod_set_t    *acmod_set;	/* describes mapping of strings to id's and
				   vice versa */

    float32 ***mixw;		/* mixing weight array */
    float32 ***mixw_acc;	/* mixing weight count accumulators */
    float32 ***l_mixw_acc;	/* local (per utterance) mixing weight accumulators */
    uint32 n_mixw;		/* number of mixing weights */
    uint32 n_feat;		/* number of feature streams */
    uint32 n_density;		/* number of densities per mixture */

    uint32 *mixw_inverse;	/* converts a local (to the utternace)
				   mixing weight ID to a global one */
    uint32 n_mixw_inverse;	/* # of local mixing weight ID's
				 [0..(n_mixw_inverse-1)]*/

    uint32 *cb_inverse;		/* local->global map for codebook ID's */
    uint32 n_cb_inverse;	/* # of local codebook ID's */

    float32 ***tmat;		/* transition probability matrices */
    float32 ***tmat_acc;	/* transition probability accumulators */
    float32 **l_tmat_acc;	/* local (per utterance) transition probability matrix */
    uint32 n_tmat;		/* number of transition matrices */
    uint32 n_state_pm;		/* number of states per model */

    gauden_t *gauden;		/* gaussian densities (see <s3/gauden.h>) */

} model_inventory_t;

/*
 * Public Interface
 */
 

/* Creation */

model_inventory_t *
mod_inv_new(void);

/* Free the entire model inventory */
void
mod_inv_free(model_inventory_t *minv);

/* Setting of simple parameters */
void
mod_inv_set_n_feat(model_inventory_t *minv,
		   uint32 n_feat);

void
mod_inv_set_n_density(model_inventory_t *minv,
		      uint32 n_density);

/* Read routines */
int32
mod_inv_read_gauden(model_inventory_t *minv,
		    const char *meanfn,
		    const char *varfn,
		    float32 varfloor,
		    uint32 n_top);

int32
mod_inv_read_tmat(model_inventory_t *minv,
		  const char *fn,
		  float32 floor);

int32
mod_inv_read_mixw(model_inventory_t *minv,
		  const model_def_t *mdef,
		  const char *fn,
		  float32 floor);

/* Allocation of reestimation accumulators */
int32
mod_inv_alloc_gauden_acc(model_inventory_t *minv);

int32
mod_inv_alloc_tmat_acc(model_inventory_t *minv);

int32
mod_inv_alloc_mixw_acc(model_inventory_t *minv);

int
mod_inv_restore_acc(model_inventory_t *minv,
		    const char *accumdir,
		    int mixw_reest,
		    int mean_reest,
		    int var_reest,
		    int tmat_reest,
		    int mllr_mult,
		    int mllr_add);

#endif /* MODEL_INVENTORY_H */ 


/*
 * Log record.  Maintained by CVS.
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
 * Revision 1.6  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.5  1996/07/29  16:49:12  eht
 * added read/initialization calls
 *
 * Revision 1.4  1996/03/25  15:54:13  eht
 * Commented
 *
 * Revision 1.3  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/09/08  19:13:52  eht
 * Updated to use acmod_set module instead of pset module
 *
 * Revision 1.1  1995/09/08  15:21:06  eht
 * Initial revision
 *
 *
 */
