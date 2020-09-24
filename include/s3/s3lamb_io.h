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
 * File: s3lamb_io.h
 * 
 * Description: 
 *    Prototypes and definitions for s3lamb_io
 *
 * Author: 
 *    Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S3LAMB_IO_H
#define S3LAMB_IO_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#define LAMBDA_FILE_VERSION "1.0"
#define LAMBDACNT_FILE_VERSION "1.0"

#include <sphinxbase/prim_type.h>
#include <sphinxbase/bio.h>

int
s3lamb_read(const char *fn,
	    float32 **out_lambda,
	    uint32 *out_n_lambda);

int
s3lamb_write(const char *fn,
	     const float32 *lambda,
	     uint32 n_lambda);

int
s3lambcnt_read(const char *fn,
	       uint32 **out_lambda_cnt,
	       uint32 *out_n_lambda_cnt);

int
s3lambcnt_write(const char *fn,
		const uint32 *lambda_cnt,
		uint32 n_lambda_cnt);

#ifdef __cplusplus
}
#endif
#endif /* S3LAMB_IO_H */ 
