/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * Description: Main code for mk_mllr_class. Converts cb2mllr mapping
 * from text form to binary form.
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>
#include <sphinxbase/ckd_alloc.h>

#include <s3/s3cb2mllr_io.h>
#include <s3/s3.h>

#include <stdio.h>

int
main(int argc, char *argv[])
{
    int32 i;
    int32 n_map=0;
    int32 n_class=0;
    int32 *mllr_map;
    char line[128];

    parse_cmd_ln(argc, argv);

    if (cmd_ln_str("-nmap")) {
	n_map = cmd_ln_int32("-nmap");
    }
    else {
	E_FATAL("Specify # of state -> MLLR class mappings using -nmap\n");
    }
    if (cmd_ln_str("-nclass")) {
	n_class = cmd_ln_int32("-nclass");
    }
    else {
	E_FATAL("Specify # of MLLR class mappings using -nclass\n");
    }
    if (cmd_ln_str("-cb2mllrfn") == NULL) {
	E_FATAL("Specify output file using -cb2mllrfn\n");
    }

    mllr_map = (int32 *)ckd_calloc(n_map, sizeof(int32));

    for (i = 0; i < n_map; i++) {
	if (fgets(line, 128, stdin) == NULL) {
	    E_FATAL("Ran out of mappings at %d, but expected %d\n",
		    i, n_map);
	}
	mllr_map[i] = atoi(line);
    }
    if (fgets(line, 128, stdin) != NULL) {
	E_WARN("Expected EOF after %d mappings, but still more data\n", n_map);
    }
    
    if (s3cb2mllr_write(cmd_ln_str("-cb2mllrfn"),
			mllr_map,
			n_map,
			n_class) != S3_SUCCESS) {
	return 1;
    }

    return 0;
}
