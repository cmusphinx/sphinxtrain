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
 * File: main.c
 * 
 * Description: 
 * 	Copies parameters from an input file to and output file
 *
 * Author: 
 *     Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"
#include <s3/s3mixw_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3tmat_io.h>
#include <s3/gauden.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>

#include <s3/s3.h>

#include <stdio.h>

static int rd_parm(void);
static int cp_parm(void);
static int wr_parm(void);


static float32 ***imixw;
static uint32 n_mixw_i;
static float32 ***omixw;
static uint32 n_mixw_o;

static uint32 n_stream;
static uint32 n_density;

static vector_t ***igau;
static vector_t ****igau_full;
static uint32 n_cb_i;
static vector_t ***ogau;
static vector_t ****ogau_full;
static uint32 n_cb_o;
static uint32 *veclen;

static float32 ***itmat;
static uint32 n_tmat_i;
static float32 ***otmat;
static uint32 n_tmat_o;
static uint32 n_state_pm;



int
rd_mixw(const char *fn, uint32 n_o)
{
    if (s3mixw_read(fn,
		    &imixw,
		    &n_mixw_i,
		    &n_stream,
		    &n_density) != S3_SUCCESS)
	return S3_ERROR;

    n_mixw_o = n_o;

    omixw = (float32 ***)ckd_calloc_3d(n_mixw_o,
				       n_stream,
				       n_density,
				       sizeof(float32));

    return S3_SUCCESS;
}
int
cp_mixw(uint32 o,
	uint32 i)
{
    uint32 j, k;

    printf("mixw %u <= %u\n", o, i);


    for (j = 0; j < n_stream; j++) {
	for (k = 0; k < n_density; k++) {
	    omixw[o][j][k] = imixw[i][j][k];
	}
    }
    return S3_SUCCESS;
}
int
wr_mixw(const char *fn)
{
    if (s3mixw_write(fn,
		     omixw,
		     n_mixw_o,
		     n_stream,
		     n_density) != S3_SUCCESS)
	return S3_ERROR;

    ckd_free_3d((void ***)omixw);
    ckd_free_3d((void ***)imixw);

    return S3_SUCCESS;
}

int
rd_tmat(const char *fn, uint32 n_o)
{
    if (s3tmat_read(fn,
		    &itmat,
		    &n_tmat_i,
		    &n_state_pm) != S3_SUCCESS)
	return S3_ERROR;

    n_tmat_o = n_o;

    otmat = (float32 ***)ckd_calloc_3d(n_tmat_o,
				       n_state_pm-1,
				       n_state_pm,
				       sizeof(float32));

    return S3_SUCCESS;
}
int
cp_tmat(uint32 o,
	uint32 i)
{
    uint32 j, k;

    printf("tmat %u <= %u\n", o, i);

    for (j = 0; j < n_state_pm-1; j++) {
	for (k = 0; k < n_state_pm; k++) {
	    otmat[o][j][k] = itmat[i][j][k];
	}
    }
    return S3_SUCCESS;
}
int
wr_tmat(const char *fn)
{
    if (s3tmat_write(fn,
		     otmat,
		     n_tmat_o,
		     n_state_pm) != S3_SUCCESS)
	return S3_ERROR;

    ckd_free_3d((void ***)otmat);
    ckd_free_3d((void ***)itmat);

    return S3_SUCCESS;
}

int
rd_gau(const char *fn, uint32 n_o)
{
    if (s3gau_read(fn,
		   &igau,
		   &n_cb_i,
		   &n_stream,
		   &n_density,
		   &veclen) != S3_SUCCESS)
	return S3_ERROR;

    n_cb_o = n_o;
    
    ogau = (vector_t ***)gauden_alloc_param(n_cb_o,
					    n_stream,
					    n_density,
					    veclen);
    return S3_SUCCESS;
}
int
cp_gau(uint32 o,
       uint32 i)
{
    uint32 j, k, l;

    printf("gau %u <= %u\n", o, i);

    for (j = 0; j < n_stream; j++) {
	for (k = 0; k < n_density; k++) {
	    for (l = 0; l < veclen[j]; l++) {
		ogau[o][j][k][l] = igau[i][j][k][l];
	    }
	}
    }
    return S3_SUCCESS;
}
int
wr_gau(const char *fn)
{
    if (s3gau_write(fn,
		    (const vector_t ***)ogau,
		    n_cb_o,
		    n_stream,
		    n_density,
		    veclen) != S3_SUCCESS)
	return S3_ERROR;

    gauden_free_param(ogau);
    gauden_free_param(igau);

    return S3_SUCCESS;
}

int
rd_gau_full(const char *fn, uint32 n_o)
{
    if (s3gau_read_full(fn,
			&igau_full,
			&n_cb_i,
			&n_stream,
			&n_density,
			&veclen) != S3_SUCCESS)
	return S3_ERROR;

    n_cb_o = n_o;
    
    ogau_full = (vector_t ****)gauden_alloc_param_full(n_cb_o,
						       n_stream,
						       n_density,
						       veclen);
    return S3_SUCCESS;
}
int
cp_gau_full(uint32 o,
	    uint32 i)
{
    uint32 j, k, l, ll;

    printf("gau %u <= %u\n", o, i);

    for (j = 0; j < n_stream; j++) {
	for (k = 0; k < n_density; k++) {
	    for (l = 0; l < veclen[j]; l++) {
		for (ll = 0; ll < veclen[j]; ll++) {
		    ogau_full[o][j][k][l][ll] = igau_full[i][j][k][l][ll];
		}
	    }
	}
    }
    return S3_SUCCESS;
}
int
wr_gau_full(const char *fn)
{
    if (s3gau_write_full(fn,
		    (const vector_t ****)ogau_full,
		    n_cb_o,
		    n_stream,
		    n_density,
		    veclen) != S3_SUCCESS)
	return S3_ERROR;

    gauden_free_param_full(ogau_full);
    gauden_free_param_full(igau_full);

    return S3_SUCCESS;
}

static int
rd_parm()
{
    if(cmd_ln_str("-imixwfn")   ==NULL&&
       cmd_ln_str("-igaufn")    ==NULL&&
       cmd_ln_str("-ifullgaufn")==NULL&&
       cmd_ln_str("-itmatfn")   ==NULL
       ) {
      E_INFO("Please specify one of the following: -imixwfn, -igaufn, -ifullgaufn, -itmatfn\n");
      return S3_ERROR;
    }

    if (cmd_ln_str("-imixwfn")) {
        if(cmd_ln_str("-nmixwout")==NULL){
	    E_INFO("Please specify -nmixwout\n");
	    return S3_ERROR;
        }
	rd_mixw(cmd_ln_str("-imixwfn"),
		cmd_ln_int32("-nmixwout"));
    }
    if (cmd_ln_str("-igaufn")) {
        if(cmd_ln_str("-ncbout")==NULL){
	    E_INFO("Please specify -ncbout\n");
	    return S3_ERROR;
        }
	rd_gau(cmd_ln_str("-igaufn"),
		cmd_ln_int32("-ncbout"));
    }
    if (cmd_ln_str("-ifullgaufn")) {
        if(cmd_ln_str("-ncbout")==NULL){
	    E_INFO("Please specify -ncbout\n");
	    return S3_ERROR;
        }
	rd_gau_full(cmd_ln_str("-ifullgaufn"),
		cmd_ln_int32("-ncbout"));
    }
    if (cmd_ln_str("-itmatfn")) {
        if(cmd_ln_str("-ntmatout")==NULL){
	    E_INFO("Please specify -ntmatout\n");
	    return S3_ERROR;
        }
	rd_tmat(cmd_ln_str("-itmatfn"),
		cmd_ln_int32("-ntmatout"));
    }
    return S3_SUCCESS;
}

static int
cp_parm()
{
    FILE *fp;
    uint32 i, o;
    uint32 max=0;

    /* Open the file first to see whether command-line parameters
       match
     */
    
    if(cmd_ln_str("-cpopsfn")==NULL) {
        E_INFO("Please specify -cpopsfn\n");
	return S3_ERROR;
    }
    fp = fopen(cmd_ln_str("-cpopsfn"), "r");
    if (fp == NULL) {
	E_INFO("Unable to open cpops file\n");

	return S3_ERROR;
    }
    while (fscanf(fp, "%u %u", &o, &i) == 2) {
        if(o+1>max) {
            max=o+1;
	}
    }

    if (omixw) {
        if(max != n_mixw_o) {
	    E_INFO("Mismatch between cp operation file (max out %d) and -nmixout (%d)\n",max, n_mixw_o);
	    return S3_ERROR;
	}
    }

    if (ogau) {
        if(max != n_cb_o) {
	    E_INFO("Mismatch between cp operation file (max out %d) and -ncbout (%d)\n",max, n_cb_o);
	    return S3_ERROR;
	}
    }

    if (ogau_full) {
        if(max != n_cb_o) {
	    E_INFO("Mismatch between cp operation file (max out %d) and -ncbout (%d)\n",max, n_cb_o);
	    return S3_ERROR;
	}
    }
	
    if (otmat) {
        if(max != n_tmat_o) {
	    E_INFO("Mismatch between cp operation file (max out %d) and -ntmatout (%d)\n",max, n_tmat_o);
	    return S3_ERROR;
	}
    }
    
    fclose(fp);
    
    fp = fopen(cmd_ln_str("-cpopsfn"), "r");
    while (fscanf(fp, "%u %u", &o, &i) == 2) {
	if (omixw) {
	    cp_mixw(o, i);
	}
	if (ogau) {
	    cp_gau(o, i);
	}
	if (ogau_full) {
	    cp_gau_full(o, i);
	}
	if (otmat) {
	    cp_tmat(o, i);
	}
    }
    fclose(fp);

    return S3_SUCCESS;
}

static int
wr_parm()
{
    if (omixw) {
        if(cmd_ln_str("-omixwfn") == NULL) {
	    E_INFO("Please specify -omixwfn\n");
	    return S3_ERROR;
        }
	wr_mixw(cmd_ln_str("-omixwfn"));
    }
    if (ogau) {
        if(cmd_ln_str("-ogaufn") == NULL) {
	    E_INFO("Please specify -ogaufn\n");
	    return S3_ERROR;
	}
	wr_gau(cmd_ln_str("-ogaufn"));
    }
    if (ogau_full) {
        if(cmd_ln_str("-ofullgaufn") == NULL) {
	    E_INFO("Please specify -ofullgaufn\n");
	    return S3_ERROR;
	}
	wr_gau_full(cmd_ln_str("-ofullgaufn"));
    }
    if (otmat) {
        if(cmd_ln_str("-otmatfn") == NULL) {
	    E_INFO("Please specify -otmatfn\n");
	    return S3_ERROR;
	}
	wr_tmat(cmd_ln_str("-otmatfn"));
    }
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{

    parse_cmd_ln(argc, argv);

    if(rd_parm()==S3_ERROR) {
	E_FATAL("Problem in reading input parameters.\n");
    }
    if(cp_parm()==S3_ERROR) {
	E_FATAL("Problem in copying parameters.\n");
    }
    if(wr_parm()==S3_ERROR) {
	E_FATAL("Problem in writing output parameters.\n");
    }
    ckd_free(veclen);

    return 0;
}

