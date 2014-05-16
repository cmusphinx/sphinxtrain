/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * File: s3io.c
 * 
 * Description: 
 *     File I/O functions for SPHINX-III binary files.
 *
 * Author: 
 *     Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s3io.h>
#include <s3/swap.h>
#include <s3/s3.h>
#include <sys_compat/file.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/bio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_ATTRIB 128

static char *attrib[MAX_ATTRIB + 1] = { NULL };
static char *value[MAX_ATTRIB + 1] = { NULL };
static int32 alloc[MAX_ATTRIB];
static int32 n_attrib = 0;

void s3clr_fattr()
{
    int i;

    for (i = 0; attrib[i]; i++) {
	if (alloc[i]) {
	    if (attrib[i]) {
		free(attrib[i]);
	    }
	    if (value[i]) {
		free(value[i]);
	    }	    
	    alloc[i] = FALSE;
	}
    }
    assert(i == n_attrib);
    n_attrib = 0;
    attrib[0] = value[0] = NULL;
}

char *
s3get_gvn_fattr(char *a)
{
    int i;

    for (i = 0; attrib[i] && strcmp(attrib[i], a) != 0; i++);

    /* value for sentinel is null as well */
    assert((attrib[i] != NULL) || (value[i] == NULL));

    return value[i];
}

void
s3get_fattr(char ***out_attrib,
	    char ***out_value)
{
    *out_attrib = attrib;
    *out_value = value;
}

void
s3add_fattr(char *a,
	    char *v,
	    int dup)
{

    if (n_attrib == MAX_ATTRIB) {
	E_FATAL("Too many file attributes, increase MAX_ATTRIB\n");
    }

    if (dup) {
	attrib[n_attrib] = strdup(a);
	value[n_attrib] = strdup(v);
    }
    else {
	attrib[n_attrib] = a;
	value[n_attrib] = v;
    }
    alloc[n_attrib] = dup;

    ++n_attrib;

    attrib[n_attrib] = NULL;
    value[n_attrib] = NULL;
}

static int
rd_attr_val(FILE *fp,
	    char *attr,
	    char *val)
{
    fscanf(fp, "%s", attr);
    if (strcmp(attr, "endhdr") == 0) {
	fgetc(fp);	/* throw away newline */
	val[0] = '\0';
	return 0;
    }
    fscanf(fp, " %[^\n]", val);
    return 1;
}

static int
rd_bin_hdr(FILE *fp,
	   uint32 *swap)
{
    char id[32];
    char attrib[256];
    char value[8192];

    if (fscanf(fp, "%31s", id) != 1) {
	E_ERROR_SYSTEM("Unable to read the file ID");

	return S3_ERROR;
    }
    if (strcmp(id, "s3") == 0) {
    }
    else {
	E_ERROR("No SPHINX-III file ID at beginning of file\n");

	return S3_ERROR;
    }

    s3clr_fattr();
    while (rd_attr_val(fp, attrib, value)) {
	s3add_fattr(attrib, value, TRUE);
    }
    
    switch (swap_check(fp)) {
    case -1:
	E_ERROR("Error reading byte order magic number\n");

	return S3_ERROR;
    case 1:
	*swap = TRUE;
	break;
    case 0:
	*swap = FALSE;
	break;
    default:
	E_FATAL("unexpected result from swap_check()\n");
	break;
    }

    return S3_SUCCESS;
}

static int
wr_bin_hdr(FILE *fp)
{
    uint32 i;
    long padding;
    int ret;

    if (fprintf(fp, "s3\n") != 3) {
	E_ERROR_SYSTEM("Error while writing SPHINX-III ID\n");
	goto error_loc;
    }

    for (i = 0; attrib[i]; i++) {
	ret = fprintf(fp, "%s %s\n", attrib[i], value[i]);
	if (ret != (strlen(attrib[i]) + 
		    strlen(value[i]) + 2)) {
	    E_ERROR_SYSTEM("Error while attrib/value pair\n");
	    goto error_loc;
	}
    }

    /* Align to an 8-byte boundary (guarantees natural alignment for
     * whatever follows) */
    padding = ftell(fp) + strlen("endhdr\n");
    padding = 8 - (padding & 7);
    if (padding != 8) {
	fwrite("        ", 1, padding, fp);
    }

    ret = fprintf(fp, "endhdr\n");
    if (ret != strlen("endhdr\n")) {
	E_ERROR_SYSTEM("Error while endhdr marker\n");
	goto error_loc;
    }

    if (swap_stamp(fp) != S3_SUCCESS) {
	goto error_loc;
    }

    return S3_SUCCESS;

error_loc:
    return S3_ERROR;
}

int
s3close(FILE *fp)
{
    s3clr_fattr();

    return fclose(fp);
}

FILE *
s3open(const char *file_name,
       const char *mode,
       uint32 *swap)
{
    FILE *fp = NULL;

    if (mode[0] == 'r' && mode[1] == 'b') {
	/* Read a binary file */

	fp = fopen(file_name, mode);
	if (fp == NULL) {
	    E_ERROR_SYSTEM("Unable to open %s for reading",
			  file_name);
	    return NULL;
	}

	if (rd_bin_hdr(fp, swap) != S3_SUCCESS) {
	    E_ERROR("Error reading header for %s\n", file_name);

	    goto error_loc;
	}

	return fp;
    }
    else if (mode[0] == 'w' && mode[1] == 'b') {
	/* Write a binary file */

	fp = fopen(file_name, mode);
	if (fp == NULL) {
	    E_ERROR_SYSTEM("Unable to open %s for writing", file_name);
	    
	    goto error_loc;
	}

	if (wr_bin_hdr(fp) != S3_SUCCESS) {
	    goto error_loc;
	}
	
	return fp;
    }
    else if (mode[0] == 'a' && mode[1] == 'b') {
	/* Append to a binary file. */

	fp = fopen(file_name, mode);
	if (fp == NULL) {
	    E_ERROR_SYSTEM("Unable to open %s for appending", file_name);
	    
	    goto error_loc;
	}

	if (ftell(fp) == 0) {
	    /* Write a header when no data */
	    if (wr_bin_hdr(fp) != S3_SUCCESS) {
		goto error_loc;
	    }
	}
	
	return fp;
    }
    
error_loc:
    if (fp)
	fclose(fp);

    return NULL;
}

int
bio_fread_intv_3d(void ****arr,
	       size_t e_sz,
	       uint32 s,
	       uint32 e,
	       uint32 *d1,
	       uint32 *d2,
	       uint32 *d3,
	       FILE *fp,
	       uint32 swap,
	       uint32 *chksum)
{
    uint32 l_d1;
    uint32 l_d2;
    uint32 l_d3;
    uint32 s_d1;
    uint32 n;
    void *raw;
    size_t ret;

    ret = bio_fread(&l_d1, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in bio_fread_3d");
	}
	return S3_ERROR;
    }

    if ((s >= l_d1) || (e >= l_d1)) {
	if (s >= l_d1) {
	    E_ERROR("Start, %u, is outside the interval [0 %u]\n",
		    s, l_d1-1);
	}
	if (e >= l_d1) {
	    E_ERROR("End, %u, is outside the interval [0 %u]\n",
		    e, l_d1-1);
	}
	
	exit(-1);
    }

    ret = bio_fread(&l_d2, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in bio_fread_3d");
	}
	return S3_ERROR;
    }
    ret = bio_fread(&l_d3, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in bio_fread_3d");
	}
	return S3_ERROR;
    }

    /* # of rows in subinterval */
    s_d1 = e - s + 1;
    
    if (fseek(fp, (long)(s*l_d2*l_d3*e_sz + sizeof(uint32)), SEEK_CUR) < 0) {
	E_FATAL_SYSTEM("Can't seek in file");
    }

    n = s_d1 * l_d2 * l_d3;
    raw = ckd_calloc(n, sizeof(float32));
    if (bio_fread(raw, e_sz, n, fp, swap, chksum) != n) {
	E_FATAL_SYSTEM("Can't read");
    }

    *arr = ckd_alloc_3d_ptr(s_d1, l_d2, l_d3, raw, e_sz);

    *d1 = s_d1;
    *d2 = l_d2;
    *d3 = l_d3;
    
    return S3_SUCCESS;
}

/* Macro to byteswap an int variable.  x = ptr to variable */
#define MYSWAP_INT(x)   *(x) = ((0x000000ff & (*(x))>>24) | \
                                (0x0000ff00 & (*(x))>>8) | \
                                (0x00ff0000 & (*(x))<<8) | \
                                (0xff000000 & (*(x))<<24))
/* Macro to byteswap a float variable.  x = ptr to variable */
#define MYSWAP_FLOAT(x) MYSWAP_INT((int *) x)

int get_length(char *file,
               int *byterev)
{
  FILE            *fp;
  int             length;
  int             n;
  struct stat statbuf;

  *byterev = 0;
  if (stat(file, &statbuf) < 0) {
      printf("stat_retry(%s) failed\n", file);
      return -1;
  }
  if ((fp = fopen(file, "rb")) == NULL) {
      printf("fopen(%s,rb) failed\n", file);
      return -1;
  }

  /* Read #floats in header */
  if (fread(&length, sizeof(int), 1, fp) != 1) {
      fclose (fp);
      return -1;
  }

  /* Check if length matches file size */
  if ((length*sizeof(float) + 4) != statbuf.st_size) {
      n = length;
      MYSWAP_INT(&n);

      if ((n*sizeof(float) + 4) != statbuf.st_size) {
          printf("Header size field: %d(%08x); filesize: %d(%08x)\n",
                  length, length, (int)statbuf.st_size, (int)statbuf.st_size);
          fflush(stdout);
          fclose (fp);
          return -1;
      }

      length = n;
      *byterev = 1;
  }
  if (length < 0) {
      printf("Header size field: %d\n",  length); fflush(stdout);
      fclose (fp);
      return -1;
  }
  fclose(fp);
  return length;
}

int
areadfloat (char *file,
            float **data_ref,
            int *length_ref)
{
  FILE            *fp;
  int             length;
  int             ret, byterev;
  int             offset;
  float           *data;
  int dummy_length;

  /* Get the file length and whether the file needs to be byte-reversed */
  if ((length = get_length(file, &byterev)) < 0) {
      return -1;
  }
  if ((fp = fopen(file, "rb")) == NULL) {
      printf("fopen(%s,rb) failed\n", file);
      return -1;
  }

  /* Read #floats in header, so we start reading at the right position */
  if (fread(&dummy_length, sizeof(int), 1, fp) != 1) {
      fclose (fp);
      return -1;
  }

  /* Just get the file size if we were not given a buffer. */
  if (data_ref == NULL) {
          fclose(fp);
          if (length_ref)
            *length_ref = length;
          return length;
  }

  if ((data = (float *) calloc (length,sizeof(float))) == NULL)
  {
    fprintf (stderr, "areadfloat: %s: can't alloc data\n", file); 
    fflush(stdout);
    fclose (fp);
    return -1;
  }
  if ((ret = fread(data,sizeof(float),length,fp)) != length)
  {
    fprintf (stderr,"areadfloat: %s: expected %d, got %d\n",file,length,ret);
    fflush(stdout);
    fclose (fp);
    free (data);
    return -1;
  }
  fclose (fp);
  *data_ref = (float *) data;
  if (byterev==1) 
     for(offset = 0; offset < length; offset++) MYSWAP_FLOAT(*data_ref+offset);
  *length_ref = length;
  return length;
}

int
areadfloat_part (char *file,
                 int s_coeff,
                 int e_coeff,
                 float **data_ref,
                 int *length_ref)
{
    static char p_file[MAXPATHLEN] = "";
    static FILE *fp = NULL;
    static int len;
    static int byterev;
    int r_len;
    float *r_buf;
    int i;
    int dummy_length;

    assert(s_coeff <= e_coeff);
    if (strcmp(file, p_file) != 0) {
        if (fp) {
            fclose(fp);
        }
        /* Get the file length and whether the file needs to be
           byte-reversed */
        if ((len = get_length(file, &byterev)) < 0) {
            return -1;
        }
        fp = fopen(file, "rb");
        if (fp == NULL) {
            fprintf(stderr, "areadfloat_part: unable to open %s for reading;", file);
            perror("");
            *data_ref = NULL;
            *length_ref = 0;

            return -1;
        }
        strcpy(p_file, file);

        if (fread(&dummy_length, sizeof(int), 1, fp) != 1) {
            fprintf(stderr, "areadfloat_part: unable to read length from %s;", file);
            perror("");

            *data_ref = NULL;
            *length_ref = 0;

            return -1;
        }
    }

    if (s_coeff >= len) {
        fprintf(stderr, "areadfloat_part: start of data beyond end of file\n");
        *data_ref = NULL;
        *length_ref = 0;

        return 0;
    }

    if (e_coeff >= len) {
        fprintf(stderr, "areadfloat_part: end of data beyond end of file; resetting\n");

        e_coeff = len-1;
    }

    if (fseek(fp, s_coeff * sizeof(float) + sizeof(int), SEEK_SET) < 0) {
        fprintf(stderr, "areadfloat_part: seek fail;");
        perror("");
        fprintf(stderr, "offset == %zu in %s\n",
                s_coeff * sizeof(float) + sizeof(int), file);

        *data_ref = NULL;
        *length_ref = 0;
        return -1;
    }

    r_len = e_coeff - s_coeff + 1;

    assert(s_coeff + r_len <= len);
    /* Just get the file size if we were not given a buffer. */
    if (data_ref == NULL) {
        if (length_ref)
            *length_ref = r_len;
        return r_len;
    }

    r_buf = calloc(r_len, sizeof(float));
    if (fread(r_buf, sizeof(float), r_len, fp) != r_len) {
        fprintf(stderr, "areadfloat_part: unable to read %d coeff @ %d from %s\n",
                r_len, s_coeff, file);
        free(r_buf);
        *data_ref = NULL;
        *length_ref = 0;
        return -1;
    }

  if (byterev==1) 
    for (i = 0; i < r_len; i++) {
        MYSWAP_FLOAT(&r_buf[i]);
    }

    *data_ref = r_buf;
    *length_ref = r_len;

    return r_len;
}




int
areadint (char *file,
	  int **data_ref,
	  int *length_ref)
{
  int             fd;
  int             length;
  int             size;
  int             offset;
  char           *data;

  if ((fd = open (file, O_RDONLY | O_BINARY, 0644)) < 0)
  {
    fprintf (stderr, "areadint: %s: can't open\n", file);
    return -1;
  }
  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "areadint: %s: can't read length (empty file?)\n", file);
    close (fd);
    return -1;
  }
  SWAP_INT32(&length);
  /* Just get the file size if we were not given a buffer. */
  if (data_ref == NULL) {
	  close(fd);
	  if (length_ref)
	    *length_ref = length;
	  return length;
  }
  size = length * sizeof (int);
  if (!(data = malloc ((unsigned) size)))
  {
    fprintf (stderr, "areadint: %s: can't alloc data\n", file);
    close (fd);
    return -1;
  }
  if (read (fd, data, size) != size)
  {
    fprintf (stderr, "areadint: %s: can't read data\n", file);
    close (fd);
    free (data);
    return -1;
  }
  close (fd);
  *data_ref = (int *) data;
  for(offset = 0; offset < length; offset++)
    SWAP_INT32(*data_ref + offset);
  *length_ref = length;
  return length;
}

int
areadshort (char *file,
	    short **data_ref,
	    int *length_ref)
{
  int             fd;
  int             length;
  int             size;
  int             offset;
  char           *data;

  if ((fd = open (file, O_RDONLY | O_BINARY, 0644)) < 0)
  {
    fprintf (stderr, "areadshort: %s: can't open\n", file);
    perror("");
    return -1;
  }
  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "areadshort: %s: can't read length (empty file?)\n", file);
    perror("");
    close (fd);
    return -1;
  }
  SWAP_INT32(&length);
  /* Just get the file size if we were not given a buffer. */
  if (data_ref == NULL) {
	  close(fd);
	  if (length_ref)
	    *length_ref = length;
	  return length;
  }

  size = length * sizeof (short);
  if (!(data = malloc ((unsigned) size)))
  {
    fprintf (stderr, "areadshort: %s: can't alloc data\n", file);
    perror("");
    close (fd);
    return -1;
  }
  if (read (fd, data, size) != size)
  {
    fprintf (stderr, "areadshort: %s: can't read data\n", file);
    perror("");
    close (fd);
    free (data);
    return -1;
  }
  close (fd);
  *data_ref = (short *) data;
  for(offset = 0; offset < length; offset++)
    SWAP_INT16(*data_ref + offset);
  *length_ref = length;
  return length;
}

int
awritefloat (char *file,
	     float *data,
	     int length)
{
  int             fd;
  int             size;
  int             offset;

  if ((fd = open (file, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0644)) < 0)
  {
    fprintf (stderr, "awritefloat: %s: can't create\n", file);
    perror("");
    return -1;
  }

  SWAP_INT32(&length);
  if (write (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "awritefloat: %s: can't write length\n", file);
    perror("");
    close (fd);
    return -1;
  }
  SWAP_INT32(&length);

  for(offset = 0; offset < length; offset++)
    SWAP_FLOAT32(data + offset);
  size = length * sizeof (float);
  if (write (fd, (char *) data, size) != size)
  {
    fprintf (stderr, "awritefloat: %s: can't write data\n", file);
    perror("");
    close (fd);
    return (-1);
  }
  for(offset = 0; offset < length; offset++)
    SWAP_FLOAT32(data + offset);

  printf ("Wrote %d floats in %s.\n", length, file);
  close (fd);
  return length;
}

