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
 * File: uttfile.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/uttfile.h>
#include <s3/fgets_wo_nl.h>
#include <s3/ckd_alloc.h>

#include <s3/err.h>
#include <s3/s3.h>

#include <string.h>
#include <assert.h>

uttfile_t *
uttfile_open(const char *fn)
{
    uttfile_t *uf;
    char tmp[32000];
    uint32 i;

    uf = (uttfile_t *)ckd_calloc(1, sizeof(uttfile_t));

    uf->fp = fopen(fn, "r");
    if (uf->fp == NULL) {
	E_ERROR_SYSTEM("Can't open file %s", fn);

	ckd_free(uf);

	return NULL;
    }

    for (i = 0; fgets_wo_nl(tmp, 32000, uf->fp) != NULL; i++);

    uf->len = i;

    rewind(uf->fp);
    
    /* uf->off == 0 by virtue of calloc */

    return uf;
}

void
uttfile_close(uttfile_t *uf)
{
    fclose(uf->fp);

    ckd_free(uf);
}

int
uttfile_data_at(uttfile_t *uf, uint32 off, char *buf, uint32 max_sz)
{
    uint32 i;

    if (off >= uf->len) {
	E_ERROR("Offset past end of file\n");

	return S3_ERROR;
    }

    if (off < uf->off) {
	rewind(uf->fp);

	uf->off = 0;
    }

    if (off == uf->off) {
	if (fgets_wo_nl(buf, max_sz, uf->fp) == NULL) {
	    E_ERROR("Unable to read data at offset %u\n", off);

	    return S3_ERROR;
	}
	else {
	    return 1;
	}
    }
    else {
	for (i = off; i < uf->off; i++) {
	    if (fgets_wo_nl(buf, max_sz, uf->fp) == NULL) {
		E_ERROR("Unable to read data at offset %u\n", off);
		
		return S3_ERROR;
	    }
	}

	return 1;
    }
}

int
uttfile_data_next(uttfile_t *uf, char *buf, uint32 max_sz)
{
    if (fgets_wo_nl(buf, max_sz, uf->fp) != NULL) {

	uf->off++;

	return 1;
    }
    else
	return 0;
}

const char *
id_of(const char *buf)
{
    uint32 i;
    char *op, *cp;
    static char id[128];
    
    op = strrchr(buf, '(');
    cp = strrchr(buf, ')');
    
    if (op && cp) {
	for (i = 0, ++op; op < cp; op++, i++) {
	    id[i] = *op;
	}
	id[i] = '\0';
    }
    else {
	E_ERROR("Unable to locate id field at end of line\n");
	    
	return NULL;
    }
    /* Fixed by awb 23/09/00, this didn't return anything before */
    return id;
} 

int
uttfile_data_for(uttfile_t *uf, const char *id, char *buf, uint32 max_sz)
{
    uint32 off, i;

    off = uf->off;
    for (i = off; i < uf->len; i++) {
	if (uttfile_data_at(uf, i, buf, max_sz) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	if (strcmp(id_of(buf), id) == 0) {
	    return TRUE;
	}
    }
    for (i = 0; i < off; i++) {
	if (uttfile_data_at(uf, i, buf, max_sz) != S3_SUCCESS) {
	    return S3_ERROR;
	}
	if (strcmp(id_of(buf), id) == 0) {
	    return TRUE;
	}
    }

    return FALSE;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:05:40  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 *
 */
