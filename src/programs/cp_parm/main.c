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
 * File: main.c
 * 
 * Description: 
 * 	Copies parameters from an input file to and output file
 *
 * Author: 
 *     Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "cmd_ln.h"
#include <s3/s3mixw_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3tmat_io.h>
#include <s3/gauden.h>
#include <s3/ckd_alloc.h>
#include <s3/feat.h>
#include <s3/cmd_ln.h>

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
static uint32 n_cb_i;
static vector_t ***ogau;
static uint32 n_cb_o;
static const uint32 *veclen;

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

static int
rd_parm()
{
    if (cmd_ln_access("-imixwfn")) {
	rd_mixw((const char *)cmd_ln_access("-imixwfn"),
		*(uint32 *)cmd_ln_access("-nmixwout"));
    }
    if (cmd_ln_access("-igaufn")) {
	rd_gau((const char *)cmd_ln_access("-igaufn"),
		*(uint32 *)cmd_ln_access("-ncbout"));
    }
    if (cmd_ln_access("-itmatfn")) {
	rd_tmat((const char *)cmd_ln_access("-itmatfn"),
		*(uint32 *)cmd_ln_access("-ntmatout"));
    }
    return S3_SUCCESS;
}

static int
cp_parm()
{
    FILE *fp;
    uint32 i, o;

    fp = fopen((char *)cmd_ln_access("-cpopsfn"), "r");
    if (fp == NULL) {
	E_ERROR_SYSTEM("Unable to open cpops file");

	return S3_ERROR;
    }
    while (fscanf(fp, "%u %u", &o, &i) == 2) {
	if (omixw) {
	    cp_mixw(o, i);
	}
	if (ogau) {
	    cp_gau(o, i);
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
	wr_mixw((const char *)cmd_ln_access("-omixwfn"));
    }
    if (ogau) {
	wr_gau((const char *)cmd_ln_access("-ogaufn"));
    }
    if (otmat) {
	wr_tmat((const char *)cmd_ln_access("-otmatfn"));
    }
    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{

    parse_cmd_ln(argc, argv);

    if (cmd_ln_access("-feat") != NULL) {
	feat_set(cmd_ln_access("-feat"));
    }
    else {
	E_FATAL("You need to set a feature extraction config using -feat\n");
    }

    rd_parm();
    cp_parm();
    wr_parm();

    return 0;
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
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
