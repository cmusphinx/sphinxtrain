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
 * File: s2_feat.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S2_FEAT_H
#define S2_FEAT_H

#include <s3/vector.h>
#include <s3/prim_type.h>

uint32
s2_feat_n_stream(void);

const uint32 *
s2_feat_vecsize(void);

uint32
s2_feat_blksize(void);

uint32
s2_feat_id(void);

const char *
s2_feat_doc(void);

void
s2_feat_set_in_veclen(uint32 len);

vector_t **
s2_feat_compute(vector_t *mfcc,
		uint32 *inout_n_frame);

vector_t **
s2_feat_alloc(uint32 n_frames);

void
s2_feat_free(vector_t **f);

void
s2_feat_print(const char *label,
	      vector_t **f,
	      uint32 n_frames);


#endif /* S2_FEAT_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:13  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.4  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1996/03/25  15:36:54  eht
 * Allow for settable input feature length
 *
 * Revision 1.2  1996/01/30  17:09:59  eht
 * *** empty log message ***
 *
 * Revision 1.1  1995/12/07  20:17:46  eht
 * Initial revision
 *
 * Revision 1.1  1995/12/07  19:30:13  eht
 * Initial revision
 *
 * Revision 1.3  1995/12/04  15:00:27  eht
 * Added prototype for feat_n_stream()
 *
 * Revision 1.2  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
