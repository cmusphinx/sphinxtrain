/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

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
	    E_WARN_SYSTEM("Unable to open %s for reading",
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
	    E_WARN_SYSTEM("Unable to open %s for writing", file_name);
	    
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
	    E_WARN_SYSTEM("Unable to open %s for appending", file_name);
	    
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

size_t
s3read(void *pointer,
       size_t size,
       size_t num_items,
       FILE *stream,
       uint32 swap,
       uint32 *chksum)
{
    size_t ret;
    size_t i;
    unsigned char  *i8;
    uint16 *i16;
    uint32 *i32;
    uint32 sum;

    ret = fread(pointer, size, num_items, stream);
    if ((long)ret < 0) {
	return ret;
    }

    if (swap) {
	switch (size) {
	case 1:
	    /* nothing to do */
	    break;

	case 2:
	    for (i = 0, i16 = (uint16 *)pointer; i < ret; i++)
		SWAP_INT16(&i16[i]);
	    break;

	case 4:
	    for (i = 0, i32 = (uint32 *)pointer; i < ret; i++)
		SWAP_INT32(&i32[i]);
	    break;

	default:
	    E_FATAL("Unimplemented size %u for swapping\n", size);
	}
    }
    
    sum = *chksum;

    /* update checksum */
    switch (size) {
    case 1:
	for (i = 0, i8 = (unsigned char *)pointer; i < ret; i++) {
	    sum = (sum << 5 | sum >> 27) + i8[i];
	}
	break;
    case 2:
	for (i = 0, i16 = (uint16 *)pointer; i < ret; i++) {
	    sum = (sum << 10 | sum >> 22) + i16[i];
	}
	break;
    case 4:
	for (i = 0, i32 = (uint32 *)pointer; i < ret; i++) {
	    sum = (sum << 20 | sum >> 12) + i32[i];
	}
	break;

    default:
	E_WARN("Unimplemented size %u for checksum\n", size);
    }
    
    *chksum = sum;

    return ret;
}

int
s3read_3d(void ****arr,
	  size_t e_sz,
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
    uint32 n;
    void *raw;
    size_t ret;

    ret = s3read(&l_d1, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_3d");
	}
	return S3_ERROR;
    }
    ret = s3read(&l_d2, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_3d");
	}
	return S3_ERROR;
    }
    ret = s3read(&l_d3, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_3d");
	}
	return S3_ERROR;
    }

    if (s3read_1d(&raw, e_sz, &n, fp, swap, chksum) != S3_SUCCESS) {
	return S3_ERROR;
    }

    assert(n == l_d1 * l_d2 * l_d3);

    *arr = ckd_alloc_3d_ptr(l_d1, l_d2, l_d3, raw, e_sz);
    *d1 = l_d1;
    *d2 = l_d2;
    *d3 = l_d3;
    
    return S3_SUCCESS;
}

int
s3read_intv_3d(void ****arr,
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

    ret = s3read(&l_d1, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_3d");
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

    ret = s3read(&l_d2, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_3d");
	}
	return S3_ERROR;
    }
    ret = s3read(&l_d3, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_3d");
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
    if (s3read(raw, e_sz, n, fp, swap, chksum) != n) {
	E_FATAL_SYSTEM("Can't read");
    }

    *arr = ckd_alloc_3d_ptr(s_d1, l_d2, l_d3, raw, e_sz);

    *d1 = s_d1;
    *d2 = l_d2;
    *d3 = l_d3;
    
    return S3_SUCCESS;
}

int
s3read_2d(void ***arr,
	  size_t e_sz,
	  uint32 *d1,
	  uint32 *d2,
	  FILE *fp,
	  uint32 swap,
	  uint32 *chksum)
{
    uint32 l_d1, l_d2;
    uint32 n;
    size_t ret;
    void *raw;
    
    ret = s3read(&l_d1, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_2d");
	}
	return S3_ERROR;
    }
    ret = s3read(&l_d2, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_2d");
	}
	return S3_ERROR;
    }
    if (s3read_1d(&raw, e_sz, &n, fp, swap, chksum) != S3_SUCCESS)
	return S3_ERROR;

    assert(n == l_d1*l_d2);

    *d1 = l_d1;
    *d2 = l_d2;
    *arr = ckd_alloc_2d_ptr(l_d1, l_d2, raw, e_sz);

    return S3_SUCCESS;
}

int
s3read_1d(void **arr,
	  size_t e_sz,
	  uint32 *d1,
	  FILE *fp,
	  uint32 swap,
	  uint32 *chksum)
{
    uint32 l_d1;
    size_t ret;
    void *raw;

    ret = s3read(&l_d1, sizeof(uint32), 1, fp, swap, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_1d");
	}
	return S3_ERROR;
    }

    raw = ckd_calloc(l_d1, e_sz);
    ret = s3read(raw, e_sz, l_d1, fp, swap, chksum);
    if (ret != l_d1) {
	if (ret == 0) {
	    E_ERROR("Unable to read complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3read_1d");
	}
	return S3_ERROR;
    }

    *d1 = l_d1;
    *arr = raw;

    return S3_SUCCESS;
}

size_t
s3write(const void *pointer,
	size_t size,
	size_t num_items,
	FILE *stream,
	uint32 *chksum)
{
    uint32 sum;
    unsigned char *i8;
    uint16 *i16;
    uint32 *i32;
    size_t i;

    sum = *chksum;

    /* update checksum over the given data items */
    switch (size) {
    case 1:
	for (i = 0, i8 = (unsigned char *)pointer; i < num_items; i++) {
	    /* rotate prior checksum by 5 bits and add data */
	    sum = (sum << 5 | sum >> 27) + i8[i];
	}
	break;
    case 2:
	for (i = 0, i16 = (uint16 *)pointer; i < num_items; i++) {
	    /* rotate prior checksum by 10 bits and add data */
	    sum = (sum << 10 | sum >> 22) + i16[i];
	}
	break;
    case 4:
	for (i = 0, i32 = (uint32 *)pointer; i < num_items; i++) {
	    /* rotate prior checksum by 20 bits and add data */
	    sum = (sum << 20 | sum >> 12) + i32[i];
	}
	break;

    default:
	E_WARN("Unimplemented size %u for checksum\n", size);
    }

    *chksum = sum;

    return fwrite(pointer, size, num_items, stream);
}


int
s3write_3d(void ***arr,
	   size_t e_sz,
	   uint32 d1,
	   uint32 d2,
	   uint32 d3,
	   FILE *fp,
	   uint32 *chksum)
{
    size_t ret;

    /* write out first dimension 1 */
    ret = s3write(&d1, sizeof(uint32), 1, fp, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_3d");
	}
	return S3_ERROR;
    }

    /* write out first dimension 2 */
    ret = s3write(&d2, sizeof(uint32), 1, fp, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_3d");
	}
	return S3_ERROR;
    }

    /* write out first dimension 3 */
    ret = s3write(&d3, sizeof(uint32), 1, fp, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_3d");
	}
	return S3_ERROR;
    }

    /* write out the data in the array as one big block */
    if (s3write_1d(arr[0][0], e_sz, d1*d2*d3, fp, chksum) != S3_SUCCESS) {
	return S3_ERROR;
    }

    return S3_SUCCESS;
}

int
s3write_2d(void **arr,
	   size_t e_sz,
	   uint32 d1,
	   uint32 d2,
	   FILE *fp,
	   uint32 *chksum)
{
    size_t ret;

    ret = s3write(&d1, sizeof(uint32), 1, fp, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_2d");
	}
	return S3_ERROR;
    }

    ret = s3write(&d2, sizeof(uint32), 1, fp, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_2d");
	}
	return S3_ERROR;
    }

    if (s3write_1d(arr[0], e_sz, d1*d2, fp, chksum) != S3_SUCCESS) {
	return S3_ERROR;
    }

    return S3_SUCCESS;
}

int
s3write_1d(void *arr,
	   size_t e_sz,
	   uint32 d1,
	   FILE *fp,
	   uint32 *chksum)
{
    size_t ret;

    ret = s3write(&d1, sizeof(uint32), 1, fp, chksum);
    if (ret != 1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_1d");
	}

	return S3_ERROR;
    }

    ret = s3write(arr, e_sz, d1, fp, chksum);
    if (ret != d1) {
	if (ret == 0) {
	    E_ERROR_SYSTEM("Unable to write complete data");
	}
	else {
	    E_ERROR_SYSTEM("OS error in s3write_1d");
	}

	return S3_ERROR;
    }

    return S3_SUCCESS;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
