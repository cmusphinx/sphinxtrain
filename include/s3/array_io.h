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
 * File: array_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef ARRAY_IO_H
#define ARRAY_IO_H


#include <s3/prim_type.h>
#include <s3/vector.h>
#include <s3/s3io.h>
#include <s3/s3.h>

#include <stdio.h>


/*
 * Old functionality;  left for file
 *   conversion purposes.
 */
int32
float32_3d_awrite(FILE *fp,
		  float32 ***out,
		  uint32 d1,
		  uint32 d2,
		  uint32 d3);

int32
float32_3d_aread(float32 ****out_arr,
		 uint32 *out_d1,
		 uint32 *out_d2,
		 uint32 *out_d3,
		 FILE *fp);

int32
float32_2d_awrite(FILE *fp,
		  float32 **out,
		  uint32 d1,
		  uint32 d2);

int32
float32_2d_aread(float32 ***out_arr,
		 uint32 *out_d1,
		 uint32 *out_d2,
		 FILE *fp);

int32
float32_feat_awrite(FILE *fp,
		    vector_t **out,
		    uint32 d1,
		    uint32 n_stream,
		    const uint32 *vecsize);

int32
float32_feat_aread(vector_t ***out_arr,
		   uint32 *out_d1,
		   uint32 *out_n_stream,
		   uint32 **out_vecsize,
		   FILE *fp);

#endif /* ARRAY_IO_H */ 


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
 * Revision 1.4  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.3  96/03/25  15:50:59  eht
 * Added 2d functions
 * 
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
