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
#ifndef __MLLR_H__
#define __MLLR_H__

#include <s3/feat.h>
#define TINY 1.0e-20;


int32 regmat_read (
	     const char    *accumdir,
             float32 ******regl,
             float32 *****regr,
             const uint32  **veclen,
             uint32  *n_mllr_class,
             uint32  *n_stream,
             uint32  *mllr_mult,
             uint32  *mllr_add
            );


int32 compute_mllr (
             float64 *****regl,
             float64 ****regr,
             const uint32  *veclen,
             uint32  nclass,
             uint32  nfeat,
             uint32  mllr_mult,
             uint32  mllr_add,
             float32 *****A,
             float32 ****B
            );


int32
invert(float32 **ainv,
       float32 **a,
       int32 len);

int32 solve  (
        float64 **regl,
        float64 *regr,
        int32   len,
        float64 *A
       );

int32 ludcmp (
         float64 **a,
         int32   n,
         int32   *indx,
         float64 *d
        );

int32 lubksb(
       float64 **a,
       int32   n,
       int32   *indx,
       float64 b[],
       float64 x[]
      );

#endif

