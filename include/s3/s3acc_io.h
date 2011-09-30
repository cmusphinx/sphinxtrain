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
 * File: s3acc_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef S3ACC_IO_H
#define S3ACC_IO_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <s3/vector.h>

int rdacc_tmat(const char *dir,
	       float32 ****inout_tmat_acc,
	       uint32 *inout_n_tmat,
	       uint32 *inout_n_state_pm);

int rdacc_mixw(const char *dir,
	       float32 ****inout_mixw_acc,
	       uint32 *inout_n_mixw,
	       uint32 *inout_n_stream,
	       uint32 *inout_n_density);

int rdacc_den(const char *dir,
	      vector_t ****inout_wt_mean,
	      vector_t ****inout_wt_var,
	      int32 *inout_pass2var,
	      float32  ****inout_dnom,
	      uint32 *inout_n_mgau,
	      uint32 *inout_n_stream,
	      uint32 *inout_n_density,
	      uint32 **inout_veclen);

int
rdacc_den_full(const char *dir,
	       vector_t ****inout_wt_mean,
	       vector_t *****inout_wt_var,
	       int32 *inout_pass2var,
	       float32  ****inout_dnom,
	       uint32 *inout_n_mgau,
	       uint32 *inout_n_stream,
	       uint32 *inout_n_density,
	       uint32 **inout_veclen);

int rdacc_mmie_den(const char *dir,
		   const char *lat_name,
		   vector_t ****inout_wt_mean,
		   vector_t ****inout_wt_var,
		   float32  ****inout_dnom,
		   uint32 *inout_n_mgau,
		   uint32 *inout_n_stream,
		   uint32 *inout_n_density,
		   uint32 **inout_veclen);

#ifdef __cplusplus
}
#endif
#endif /* S3ACC_IO_H */ 

