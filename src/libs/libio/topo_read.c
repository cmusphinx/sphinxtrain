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

#include <sphinxbase/pio.h>
#include <s3/common.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

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
    lineiter_t *li = NULL;
    uint32 n_state;
    uint32 i, j;
    float32 row_sum;

    assert(topo_file_name != NULL);

    fp = fopen(topo_file_name, "r");
    if (fp == NULL) {
	E_ERROR_SYSTEM("Unable to open %s for reading\n", topo_file_name);

	goto error;
    }
    
    li = lineiter_start_clean(fp);
    
    if (li == NULL) {
	E_ERROR("EOF encounted while reading version number in %s!?\n", topo_file_name);

	goto error;
    }

    if (strcmp(li->buf, TOPO_FILE_VERSION) != 0) {
	E_ERROR("Topo file version in %s is %s.  Expected %s\n",
		topo_file_name, li->buf, TOPO_FILE_VERSION);

	goto error;
    }

    li = lineiter_next(li);
    if (li == NULL) {
	E_ERROR("EOF encountered while reading n_state in %s!?\n", topo_file_name);

	goto error;
    }

    sscanf(li->buf, "%d\n", &n_state);

    /* Support Request 1504066: robust reading of topo file in
       SphinxTrain
	   
       When user put 
       0.1
       1.0 1.0 1.0 0.0
       1.0 1.0 1.0 0.0
       1.0 1.0 1.0 1.0 
       
       instead of 
       
       0.1
       4
       1.0 1.0 1.0 0.0
       1.0 1.0 1.0 0.0
       1.0 1.0 1.0 1.0
       
       topo_read will misread 1.0 into n_state as 1.  And the 
       generated transition matrix will corrupt bw as well. This 
       problem is now fixed. 
    */

    if(n_state==1) {
        E_ERROR("n_state =1, if you are using a transition matrix with more than 1 state, this error might show that there is format issue in your input topology file.  You are recommended to use perl/make_topology.pl to generate the topo file instead.\n");
	goto error;
    }

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
    lineiter_free(li);
    return S3_SUCCESS;

error:    
    if (fp) fclose(fp);
    lineiter_free(li);
    return S3_ERROR;
}
