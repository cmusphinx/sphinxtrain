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
 * File: mllr_class_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MLLR_CLASS_IO_H
#define MLLR_CLASS_IO_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#define MLLR_CLASS_VERSION	"0.1"

#include <sphinxbase/prim_type.h>
#include <s3/gauden.h>

int
mllr_class_read(uint32 **mllr_cmap,
		uint32 *n_map,
		uint32 *n_class,
		const char **comment,
		const char *file_name);

int
mllr_class_write(const char *file_name,
		 uint32 *mllr_class_map,
		 uint32 n_mgau,
		 uint32 n_mllr_class,
		 char *comment);

int
ck_mllr_class(uint32 *mllr_class_map,
	      uint32 n_map,
	      uint32 n_mgau);

#ifdef __cplusplus
}
#endif
#endif /* MLLR_CLASS_IO_H */

