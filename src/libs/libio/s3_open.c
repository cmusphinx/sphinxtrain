/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
