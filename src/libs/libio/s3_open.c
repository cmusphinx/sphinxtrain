/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * File: s3_open.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s3_open.h>

#include <s3/bcomment_io.h>
#include <s3/swap.h>
#include <s3/s3.h>

#include <string.h>

FILE *
s3_open_bin_read(const char *file_name,
		 const char *in_version,
		 const char **out_comment)
{
    FILE *fp;
    char version[MAX_VERSION_LEN];

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
	E_WARN_SYSTEM("Unable to open %s for reading",
		      file_name);

	return NULL;
    }

    if (fscanf(fp, "%s", version) != 1) {
	E_ERROR("Premature EOF while reading %s\n", file_name);

	goto error;
    }
    
    if (strcmp(version, in_version) != 0) {
	E_ERROR("version mismatch.  %s != %s (expected)\n",
		__FILE__, __LINE__,
		version, in_version);

	goto error;
    }
    
    if (bcomment_read(out_comment, fp) != S3_SUCCESS) {
	goto error;
    }
    
    if (swap_check(fp) != S3_SUCCESS) {
	goto error;
    }

    return fp;

    error:

    fclose(fp);
    return NULL;
}

FILE *
s3_open_bin_write(const char *file_name,
		  const char *version,
		  const char *comment)
{
    FILE *fp;

    fp = fopen(file_name, "wb");
    if (fp == NULL) {
	E_WARN_SYSTEM("Unable to open %s for writing", file_name);

	goto error;
    }

    if ((unsigned)fprintf(fp, "%s\n", version) !=
	strlen(version)+1) {
	E_ERROR("unable to write version id in %s",
		file_name);
    
	goto error;
    }

    if (bcomment_write(fp, comment) != S3_SUCCESS) {
	goto error;
    }
    
    if (swap_stamp(fp) != S3_SUCCESS) {
	goto error;
    }
	
    return fp;
    
    error:
    
    fclose(fp);
    return NULL;
}

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
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
