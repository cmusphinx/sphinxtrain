/* ====================================================================
 * Copyright (c) 1989-2000 Carnegie Mellon University.  All rights 
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
/*
	30 May 1989 David R. Fulmer (drf) updated to do byte order
		conversions when necessary.
 */

#include <s2/byteorder.h>
#include <s3/s2io.h>

#include <sys_compat/file.h>
#include <stdio.h>
#include <stdlib.h>

int
areadchar (char *file,
	   char **data_ref,
	   int *length_ref)
{
  int             fd;
  int             length;
  char           *data;

  if ((fd = open (file, O_RDONLY, 0644)) < 0)
  {
    fprintf (stderr, "areadchar: %s: can't open\n", file);
    return -1;
  }
  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "areadchar: %s: can't read length (empty file?)\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&length);
  if (!(data = malloc ((unsigned) length)))
  {
    fprintf (stderr, "areadchar: %s: can't alloc data\n", file);
    close (fd);
    return -1;
  }
  if (read (fd, data, length) != length)
  {
    fprintf (stderr, "areadchar: %s: can't read data\n", file);
    close (fd);
    free (data);
    return -1;
  }
  close (fd);
  *data_ref = data;
  *length_ref = length;
  return length;
}
