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
 * File: topo_read.c
 * 
 * Description: 
 * 	Read an ASCII model topology file.  This file contains
 *	an adjacency matrix with non-zero elements for all
 *	allowable transitions where a row represents a source state
 *	and a column represents a sink state.
 *
 *	If the adjacency matrix contains values that are all equal,
 *	the matrix can be normalized to obtain a uniform transition
 *	probability matrix.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

/* try to put header files in local to global order to
   try to flush out hidden dependencies */
#include <s3/topo_read.h>

#include <s3/read_line.h>
#include <s3/common.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define BIG_STR_LEN	4096
#define TOPO_FILE_VERSION	"0.1"

/*********************************************************************
 *
 * Function: 
 *	topo_read
 * 
 * Description: 
 * 	This routine reads an ASCII transition matrix which may then be
 *	used to determine the topology of the models used in the system.
 *
 * Traceability: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	S3_SUCCESS is returned upon successful completion
 *	S3_ERROR is returned upon an error condition
 * 
 * Global Outputs: 
 *	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/
int32
topo_read(float32 ***tmat,
	  uint32 *n_state_pm,
	  const char *topo_file_name)
{
    float32 **out;
    FILE *fp;
    char buf[BIG_STR_LEN];
    uint32 n_state;
    uint32 i, j;
    uint32 n_read;
    float32 row_sum;

    assert(topo_file_name != NULL);

    fp = fopen(topo_file_name, "r");
    if (fp == NULL) {
	E_WARN_SYSTEM("Unable to open %s for reading\n", topo_file_name);

	goto error;
    }
    
    if (read_line(buf, BIG_STR_LEN, &n_read, fp) == NULL) {
	E_ERROR("EOF encounted while reading version number in %s!?\n", topo_file_name);

	goto error;
    }

    if (strcmp(buf, TOPO_FILE_VERSION) != 0) {
	E_ERROR("Topo file version in %s is %s.  Expected %s\n",
		topo_file_name, buf, TOPO_FILE_VERSION);

	goto error;
    }

    if (read_line(buf, BIG_STR_LEN, &n_read, fp) == NULL) {
	E_ERROR("EOF encountered while reading n_state in %s!?\n", topo_file_name);

	goto error;
    }

    sscanf(buf, "%d", &n_state);

    out = (float **)ckd_calloc_2d(n_state-1, n_state, sizeof(float32));

    for (i = 0; i < n_state-1; i++) {
	row_sum = 0.0;
	for (j = 0; j < n_state; j++) {
	    fscanf(fp, "%f", &out[i][j]);
	    row_sum += out[i][j];
	}
	for (j = 0; j < n_state; j++) {
	    out[i][j] /= row_sum;
	}
    }
    
    *tmat = out;
    *n_state_pm = n_state;

    fclose(fp);
    return S3_SUCCESS;

error:    
    if (fp) fclose(fp);
    return S3_ERROR;
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
