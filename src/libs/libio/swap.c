/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
