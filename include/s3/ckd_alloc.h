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
 * file: ckd_alloc.h
 * 
 * traceability: 
 * 
 * description: 
 * 
 * author: 
 * 
 *********************************************************************/

#ifndef CKD_ALLOC_H
#define CKD_ALLOC_H

#include <stdlib.h>

#define ckd_salloc(s)                     __ckd_salloc((s), __FILE__, __LINE__)
#define ckd_calloc(d1, s)                 __ckd_calloc((d1), (s), __FILE__, __LINE__)
#define ckd_malloc(s)                     __ckd_malloc((s), __FILE__, __LINE__)
#define ckd_realloc(p, s)                 __ckd_realloc((p), (s), __FILE__, __LINE__)
#define ckd_free(p)                       __ckd_free((p), __FILE__, __LINE__)

/* array storage management */
#define ckd_calloc_2d(d1, d2, s)          __ckd_calloc_2d((d1), (d2), (s), __FILE__, __LINE__)
#define ckd_free_2d(p)                    __ckd_free_2d((p), __FILE__, __LINE__)
#define ckd_alloc_2d_ptr(d1, d2, bf, sz)    __ckd_alloc_2d_ptr((d1), (d2), (bf), (sz), __FILE__, __LINE__)

#define ckd_calloc_3d(d1, d2, d3, s)       __ckd_calloc_3d((d1), (d2), (d3), (s), __FILE__, __LINE__)
#define ckd_free_3d(p)                     __ckd_free_3d((p), __FILE__, __LINE__)
#define ckd_alloc_3d_ptr(d1, d2, d3, bf, sz) __ckd_alloc_3d_ptr((d1), (d2), (d3), (bf), (sz), __FILE__, __LINE__)

#define ckd_calloc_4d(d1, d2, d3, d4, s)  __ckd_calloc_4d((d1), (d2), (d3), (d4), (s), __FILE__, __LINE__)
#define ckd_free_4d(p)                    __ckd_free_4d((p), __FILE__, __LINE__)

char *
__ckd_salloc(char *string,
	     char *caller_file,
	     int caller_line);

void *
__ckd_calloc(size_t n_elem,
	     size_t elem_size,
	     char *caller_file,
	     int caller_line);

void **
__ckd_calloc_2d(size_t d1,
		size_t d2,		
		size_t elem_size,
		char *caller_file,
		int caller_line);

void
__ckd_free_2d(void **ptr,
	      char *caller_file,
	      int caller_line);


void ***
__ckd_calloc_3d(size_t d1,
		size_t d2,		
		size_t d3,		
		size_t elem_size,
		char *caller_file,
		int caller_line);

void
__ckd_free_3d(void ***ptr,
	      char *caller_file,
	      int caller_line);


void ****
__ckd_calloc_4d(size_t d1,
		size_t d2,		
		size_t d3,		
		size_t d4,		
		size_t elem_size,
		char *caller_file,
		int caller_line);
void
__ckd_free_4d(void ****ptr,
	      char *caller_file,
	      int caller_line);

void *
__ckd_malloc(size_t size,
	     char *caller_file,
	     int caller_line);

void *
__ckd_realloc(void *ptr,
	      size_t new_size,
	      char *caller_file,
	      int caller_line);

void
__ckd_free(void *ptr,
	   char *caller_file,
	   int caller_line);

void ***
__ckd_alloc_3d_ptr(int d1,
		   int d2,
		   int d3,
		   void *store,
		   size_t elem_size,
		   char *caller_file,
		   int caller_line);

void **
__ckd_alloc_2d_ptr(int d1,
		   int d2,
		   void *store,
		   size_t elem_size,
		   char *caller_file,
		   int caller_line);

#endif /* CKD_ALLOC_H */ 


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
 * Revision 1.7  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.6  96/07/29  16:40:50  eht
 * add array allocation here so that I don't
 * go crazy trying to remember the function call names
 * 
 * Revision 1.5  1996/02/02  17:58:00  eht
 * Add ckd_salloc macro
 *
 * Revision 1.4  1996/02/02  17:49:27  eht
 * Added ckd_salloc function
 *
 * Revision 1.3  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.2  1995/10/09  20:29:52  eht
 * New interface/macros that remove need for __FILE__, __LINE__ args
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
