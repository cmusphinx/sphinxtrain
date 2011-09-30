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

#ifndef __MLLR_IO__
#define __MLLR_IO__
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

int32
store_reg_mat (const char    *regmatfn,
	       const uint32  *veclen,
	       uint32  n_class,
	       uint32  n_stream,
	       float32 ****A,
	       float32 ***B);


int32
read_reg_mat (const char   *regmatfn,
	      uint32  **veclen,
	      uint32  *n_class,
	      uint32  *n_stream,
	      float32 *****A,
	      float32 ****B);

int32
free_mllr_A(float32 ****A,
	    uint32  n_class,
	    uint32  n_stream);


int32
free_mllr_B(float32 ***B,
	    uint32  n_class,
	    uint32  n_stream);

int32
free_mllr_reg(float32 *****regl,
              float32 ****regr,
              uint32  n_class,
              uint32  n_stream);

#ifdef __cplusplus
}
#endif
#endif /* __MLLR_IO__ */
