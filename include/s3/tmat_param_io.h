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
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
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
 * File: tmat_param_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef TMAT_PARAM_IO_H
#define TMAT_PARAM_IO_H

#define TMAT_PARAM_VERSION	"0.1"

#include <s3/prim_type.h>

int32
tmat_param_write(const char *file_name,
		 const char *comment,
		 float32 ***tmat,
		 unsigned int n_tmat,
		 unsigned int n_state);

int32
tmat_param_read(const char **out_comment,
		float32 ****out_tmat,
		unsigned int *out_n_tmat,
		unsigned int *out_n_state,
		const char *file_name);

int32
tmat_param_unbundle(const char *dir,
		    float32 ***tmat,
		    unsigned int n_tmat,
		    unsigned int n_state);

#endif /* TMAT_PARAM_IO_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:12  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:30  awb
 * *** empty log message ***
 *
 * Revision 1.5  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.4  96/11/04  07:08:13  eht
 * Added prototype for tmat_param_unbundle
 * 
 * Revision 1.3  96/07/29  16:33:40  eht
 * added bundle/unbundle routines
 * 
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
