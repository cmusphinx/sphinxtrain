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
 * File: model_def_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef MODEL_DEF_IO_H
#define MODEL_DEF_IO_H

#include <s3/acmod_set.h>
#include <s3/prim_type.h>

#include <s3/model_def.h>

#include <s3/s3.h>


#define MODEL_DEF_VERSION "0.3"

int32
model_def_read(model_def_t **out_mdef,
	       const char *file_name);

int32
model_def_write(model_def_t *mdef,
		const char *file_name);

#endif /* MODEL_DEF_IO_H */ 

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
 * Revision 1.7  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.6  96/07/29  16:33:40  eht
 * added bundle/unbundle functions
 * 
 * Revision 1.5  1996/03/25  15:52:26  eht
 * Commented
 *
 * Revision 1.4  1996/01/26  17:55:01  eht
 * Include mapping from tied states to mixture Gaussian densities in the model_def_t
 * structure
 *
 * Revision 1.3  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/09/08  19:13:52  eht
 * Updated to replace pset module with acmod_set module
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
