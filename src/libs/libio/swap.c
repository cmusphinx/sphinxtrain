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
 * File: swap.c
 * 
 * Description: 
 * 	Reads the byte-order magic number from a binary file header.
 *	Determines whether swapping should be done given the value.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <s3/bo_magic.h>
#include <s3/swap.h>
#include <s3/fread_retry.h>

#include <assert.h>
#include <string.h>


int32
swap_stamp(FILE *fp)
{
    uint32 magic = BYTE_ORDER_MAGIC;

    if (fwrite(&magic, sizeof(uint32), 1, fp) != 1) {
	E_ERROR("error while writing bo_magic\n");

	return S3_ERROR;
    }

    return S3_SUCCESS;
}

int32
swap_check(FILE *fp)
{
    uint32 magic;
    uint32 ret = -1;

    if (fread_retry(&magic, sizeof(uint32), 1, fp) != 1) {
	E_ERROR("error while reading bo_magic\n");

	ret = -1;
	
	return ret;
    }

    if (magic != BYTE_ORDER_MAGIC) {
	/* either need to swap or got bogus magic number */
	
	SWAP_INT32(&magic);
	
	if (magic == BYTE_ORDER_MAGIC) {
	    ret = 1;
	}
	else {
	    /* could not get magic number by swapping, so it is bogus */
	    E_ERROR("Expected to read 0x%x (or byte permuted) byte order indicator.  Instead read 0x%x\n",
		    BYTE_ORDER_MAGIC, SWAP_INT32(&magic));
	    
	    ret = -1;
	}
    }
    else {
	/* BYTE_ORDER_MAGIC was read; so no need to swap */
	ret = 0;
    }

    return ret;
}

int
swap_little_endian()
{
    uint32 l;
    char *c;

    l = 1;

    c = (char *)&l;

    if (*c) {
	return TRUE;
    }
    else {
	return FALSE;
    }
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
