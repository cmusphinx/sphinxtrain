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
 * File: s3.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef S3_H
#define S3_H

#include <s3/err.h>
#include <s3/prim_type.h>

#define	S3_SUCCESS	0
#define S3_ERROR	-1
#define S3_WARNING	-2

#define MAX_IEEE_NORM_POS_FLOAT32	 3.4e+38
#define MAX_POS_FLOAT32	 		 3.4e+38

#define MIN_IEEE_NORM_POS_FLOAT32	 1.2e-38
#define MIN_POS_FLOAT32	 		 1.2e-38

#define MIN_IEEE_NORM_NEG_FLOAT32	-3.4e+38
#define MIN_NEG_FLOAT32			-3.4e+38

#define MAX_IEEE_NORM_POS_FLOAT64	 1.8e+307
#define MAX_POS_FLOAT64	 		 1.8e+307

#define MIN_IEEE_NORM_POS_FLOAT64	 2.2e-308
#define MIN_POS_FLOAT64	 		 2.2e-308

#define MIN_IEEE_NORM_NEG_FLOAT64	-1.8e+307
#define MIN_NEG_FLOAT64			-1.8e+307

/* The maximum # of states for any given acoustic model */
#define MAX_N_STATE	20

/* The maximum # of attributes associated with any
 * given acoustic model */
#define MAX_N_ATTRIB	5

#ifndef TRUE
#define TRUE  1
#define FALSE 0	/* assume that true is never defined w/o false */
#endif

/* Timer for elapsed I/O time */
#define IO_ELAPSED	0

/* Timer for utt processing elapsed time */
#define UTT_ELAPSED	1
#define UTT_IO_ELAPSED	2
#define UTT_BW_ELAPSED	3

#define TYING_NON_EMITTING	(0xffffffff)
#define TYING_NO_ID		(0xffffffff)

#define MAX_VERSION_LEN	128

#define MEG *1024*1024

#endif /* S3_H */ 

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
 * Revision 1.4  96/07/29  16:40:50  eht
 * Make some of the constants less long winded.
 * 
 * Revision 1.3  1996/01/26  18:29:54  eht
 * Added some floating point MIN and MAX constants
 *
 * Revision 1.2  1995/10/10  12:25:04  eht
 * Add TYING_NO_ID to the set of symbolic constants defined.
 *
 * Revision 1.1  1995/10/09  21:17:24  eht
 * Initial revision
 *
 *
 */
