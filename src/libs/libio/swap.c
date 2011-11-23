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
 * File: swap.c
 * 
 * Description: 
 * 	Reads the byte-order magic number from a binary file header.
 *	Determines whether swapping should be done given the value.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <s3/swap.h>

#include <sphinxbase/pio.h>
#include <sphinxbase/bio.h>

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
