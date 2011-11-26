/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: best_q.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef BEST_Q_H
#define BEST_Q_H


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>
#include <s3/quest.h>

float64
best_q(float32 ****mixw,
       float32 ****means,
       float32 ****vars,
       uint32  *veclen,
       uint32 n_model,
       uint32 n_state,
       uint32 n_stream,
       uint32 n_density,
       float32 *stwt,
       uint32 **dfeat,
       uint32 n_dfeat,
       quest_t *all_q,
       uint32 n_all_q,
       pset_t *pset,
       uint32 *id,
       uint32 n_id,
       float32 ***dist,
       float64 node_wt_ent,  /* Weighted entropy of node */
       quest_t **out_best_q);

float64
two_class(float32 ****mixw_occ,
          float32 ****means,
          float32 ****vars,
          uint32 *veclen,
          uint32 n_dist,
          uint32 n_state,
          uint32 n_stream,
          uint32 n_density,
          float32 *stwt,
          uint32 *bclust,
          float32 mwfloor);

#ifdef __cplusplus
}
#endif

#endif /* BEST_Q_H */ 

