/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
/*
 * bio.h -- Sphinx-3 binary file I/O functions.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 18-Dec-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */



#ifndef _LIBIO_BIO_H_
#define _LIBIO_BIO_H_

#include <stdio.h>


#define BYTE_ORDER_MAGIC	(0x11223344)

/* Macro to byteswap an int16 variable.  x = ptr to variable */
#define SWAP_INT16(x)	*(x) = ((0x00ff & (*(x))>>8) | (0xff00 & (*(x))<<8))

/* Macro to byteswap an int32 variable.  x = ptr to variable */
#define SWAP_INT32(x)	*(x) = ((0x000000ff & (*(x))>>24) | \
				(0x0000ff00 & (*(x))>>8) | \
				(0x00ff0000 & (*(x))<<8) | \
				(0xff000000 & (*(x))<<24))

/* Macro to byteswap a float32 variable.  x = ptr to variable */
#define SWAP_FLOAT32(x)	SWAP_INT32((int32 *) x)


/*
 * Read binary file format header: has the following format
 *     s3
 *     <argument-name> <argument-value>
 *     <argument-name> <argument-value>
 *     ...
 *     endhdr
 *     4-byte byte-order word used to find file byte ordering relative to host machine.
 * Lines beginning with # are ignored.
 * Memory for name and val allocated by this function; use bio_hdrarg_free to free them.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 bio_readhdr (FILE *fp,		/* In: File to read */
		   char ***name,	/* Out: array of argument name strings read */
		   char ***val,		/* Out: corresponding value strings read */
		   int32 *swap);	/* Out: file needs byteswapping iff (*swap) */

/*
 * Write a simple binary file header, containing only the version string.  Also write
 * the byte order magic word.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 bio_writehdr_version (FILE *fp, char *version);

/*
 * Free name and value strings previously allocated and returned by bio_readhdr.
 */
void bio_hdrarg_free (char **name,	/* In: Array previously returned by bio_readhdr */
		      char **val);	/* In: Array previously returned by bio_readhdr */

/*
 * Like fread but perform byteswapping and accumulate checksum (the 2 extra arguments).
 * But unlike fread, returns -1 if required number of elements (n_el) not read; also,
 * no byteswapping or checksum accumulation is performed in that case.
 */
int32 bio_fread (void *buf,
		 int32 el_sz,
		 int32 n_el,
		 FILE *fp,
		 int32 swap,		/* In: Byteswap iff (swap != 0) */
		 uint32 *chksum);	/* In/Out: Accumulated checksum */

/*
 * Read a 1-d array (fashioned after fread):
 *     4-byte array size (returned in n_el)
 *     memory allocated for the array and read (returned in buf)
 * Byteswapping and checksum accumulation performed as necessary.
 * Fails fatally if expected data not read.
 * Return value: #array elements allocated and read; -1 if error.
 */
int32 bio_fread_1d (void **buf,		/* Out: contains array data; allocated by this
					   function; can be freed using ckd_free */
		    int32 el_sz,	/* In: Array element size */
		    int32 *n_el,	/* Out: #array elements allocated/read */
		    FILE *fp,		/* In: File to read */
		    int32 sw,		/* In: Byteswap iff (swap != 0) */
		    uint32 *ck);	/* In/Out: Accumulated checksum */

/*
 * Read and verify checksum at the end of binary file.  Fails fatally if there is
 * a mismatch.
 */
void bio_verify_chksum (FILE *fp,	/* In: File to read */
			int32 byteswap,	/* In: Byteswap iff (swap != 0) */
			uint32 chksum);	/* In: Value to compare with checksum in file */


#endif
