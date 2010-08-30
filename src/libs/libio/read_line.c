/* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights 
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
 * File: read_line.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/read_line.h>
#include <s3/err.h>

#include <string.h>

/**
 * Reads the line from the file skipping the comments.
 * Trims whitespaces in the begining and in the end
 *
 *  @buf buffer to read into
 *  @max_len size of the buffer
 *  @n_read line counter, may be NULL
 *  @fp file to read from
 */
char *
read_line(char *buf,
	  size_t max_len,
	  uint32 *n_read,
	  FILE *fp)
{
    char *out;
    char *start;
    char *end;
    int read = 0;
    
    if (n_read != NULL)
	read = *n_read;

    do {
	out = fgets(buf, max_len, fp);
	read++;
    } while ((out != NULL) && (out[0] == '#'));

    if (strlen(buf) == (max_len-1)) {
	E_WARN("line %d may be truncated because it's longer than max_len %d\n",
	       read, max_len);
    }
    
    if (n_read != NULL)
	*n_read = read;
    
    if (out == NULL)
	return out;
    
    start = out;
    end = out + strlen(out) - 1;

    while (*start == ' ' || 
	   *start == '\t') {
	start++;
    }

    while ((end >= start) &&
	   (*end == ' ' || 
           *end == '\t' ||
	   *end == '\r' ||
	   *end == '\n'))
	end--;
    *(++end) = 0;
    
    memmove(out, start, end - start + 1);
    
    return out;
}
