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
 * 
 * Author: 
 * 
 *********************************************************************/

#include "cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>
#include <s3/s3cb2mllr_io.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <stdio.h>

int
main(int argc, char *argv[])
{
    int32 i;
    int32 n_map=0;
    int32 n_class=0;
    uint32 *mllr_map;
    char line[128];

    parse_cmd_ln(argc, argv);

    if (cmd_ln_access("-nmap")) {
	n_map = *(int32 *)cmd_ln_access("-nmap");
    }
    else {
	E_FATAL("Specify # of state -> MLLR class mappings using -nmap\n");
    }
    if (cmd_ln_access("-nclass")) {
	n_class = *(int32 *)cmd_ln_access("-nclass");
    }
    else {
	E_FATAL("Specify # of MLLR class mappings using -nclass\n");
    }
    if (cmd_ln_access("-cb2mllrfn") == NULL) {
	E_FATAL("Specify output file using -cb2mllrfn\n");
    }

    mllr_map = (uint32 *)ckd_calloc(n_map, sizeof(uint32));

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
    
    if (s3cb2mllr_write((const char *)cmd_ln_access("-cb2mllrfn"),
			mllr_map,
			n_map,
			n_class) != S3_SUCCESS) {
	return 1;
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:14  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/07  09:01:29  eht
 * Initial revision
 * 
 *
 */
