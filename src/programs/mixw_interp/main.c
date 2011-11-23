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
 * File: main.c
 * 
 * Description: 
 *	This file reads two mixing weight file and interoplates them
 *      using log function 
 * 
 * Author: 
 *	Vipul Parikh (vparikh@cs.cmu.edu)
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/common.h>
#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <sys_compat/file.h>
#include <s3/s3.h>

#include <sphinxbase/err.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

static int interp_mixw(void);


static int
initialize(int argc,
	   char *argv[])
{

    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    return S3_SUCCESS;
}

int
interp_mixw()
{
    float32 ***SImixw, ***SDmixw, SIsum, SDsum;
    float32 silambda;
    uint32 n_SImixw, n_SDmixw; 
    uint32 n_SIfeat, n_SDfeat; 
    uint32 n_SIdensity, n_SDdensity; 
    const char *SIfn, *SDfn, *outfn;
    uint32 i, j, k;

    SIfn = cmd_ln_str("-SImixwfn");
    E_INFO("Reading %s\n", SIfn);

    if (s3mixw_read(SIfn,
		    &SImixw,
		    &n_SImixw,
		    &n_SIfeat,
		    &n_SIdensity) != S3_SUCCESS) {
	return S3_ERROR;
    }

    printf("SImixw %u %u %u\n", n_SImixw, n_SIfeat, n_SIdensity);

    SDfn = cmd_ln_str("-SDmixwfn");
    E_INFO("Reading %s\n", SDfn);

    if (s3mixw_read(SDfn,
		    &SDmixw,
		    &n_SDmixw,
		    &n_SDfeat,
		    &n_SDdensity) != S3_SUCCESS) {
	return S3_ERROR;
    }

    silambda = cmd_ln_float32("-SIlambda");
    if (silambda < 0 || silambda > 1.0)
	E_FATAL("silamda value outside range (0,1.0)\n");

    printf("SDmixw %u %u %u\n", n_SDmixw, n_SDfeat, n_SDdensity);
    printf("Interpolating with SI weight %f\n",silambda);

    if ((n_SImixw != n_SDmixw) | (n_SIfeat != n_SDfeat) | 
                                        (n_SIdensity != n_SDdensity)) {
        E_INFO("Mixing weight files are not compatible.\n");
        return S3_ERROR;
    }

    for (i = 0; i < n_SDmixw; i++) {
	for (j = 0; j < n_SDfeat; j++) {
            float32 SIwt = 0;
            float32 SDwt = 0;

            for (k = 0, SDsum = 0; k < n_SDdensity; k++) {
                SDsum += SDmixw[i][j][k];
            }
            /*if (j == 0) printf("%f\t",SDsum);*/
            if (SDsum != 0) {
                SDwt = (1.0 - silambda)/SDsum;
            }
		
            for (k = 0, SIsum = 0; k < n_SIdensity; k++) {
                SIsum += SImixw[i][j][k];
            }
            /*if (j == 0) printf("%f\n",SIsum);*/
            if (SIsum != 0) {
                SIwt = silambda/SIsum;
            }

            for (k = 0; k < n_SDdensity; k++) {
                SDmixw[i][j][k] = SImixw[i][j][k]*SIwt + SDmixw[i][j][k]*SDwt;
            }
	}
    }

    outfn = cmd_ln_str("-outmixwfn");
    E_INFO("Writing %s\n", outfn);
    if (s3mixw_write(outfn,
                     SDmixw,
                     n_SDmixw,
                     n_SDfeat,
                     n_SDdensity) != S3_SUCCESS) {
        return S3_ERROR;
    }

    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    if (initialize(argc, argv) != S3_SUCCESS) {
	E_ERROR("Errors initializing.\n");
	return 1;
    }

    if (interp_mixw() != S3_SUCCESS)
	return 1;

    return 0;
}

