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
 * File: pset_io.c
 * 
 * Description: 
 *    Read a file which "defines" the simple questions for state
 *    clustering
 *
 * Author: 
 *    Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/pset_io.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

pset_t *
read_pset_file(const char *file_name,
	       acmod_set_t *acmod_set,
	       uint32 *n_pset)
{
    FILE *fp;
    char line[1024];
    char sav_line[1024];
    uint32 i, j, lc, n_phone, p;
    pset_t *out;
    uint32 n_ci;

    fp = fopen(file_name, "r");

    if (fp == NULL) {
	E_WARN_SYSTEM("Unable to open %s", file_name);

	return NULL;
    }

    for (lc = 0; fgets(line, 1024, fp) != NULL; lc++);
    
    out = ckd_calloc(lc, sizeof(pset_t));
    *n_pset = lc;

    rewind(fp);

    n_ci = acmod_set_n_ci(acmod_set);

    for (i = 0; i < lc; i++) {
	fgets(line, 1024, fp);
	line[strlen(line)-1] = '\0';

	strcpy (sav_line, line);
	
	strtok(line, " \t");

	for (n_phone = 0; strtok(NULL, " \t"); n_phone++);

	strcpy (line, sav_line);

	out[i].n_phone = n_phone;

	if (n_phone > 0) {
	    out[i].phone  = ckd_calloc(n_phone, sizeof(acmod_id_t));
	    out[i].member = ckd_calloc(n_ci, sizeof(uint32));
	
	    out[i].name = strdup(strtok(line, " \t"));
	    for (j = 0; j < n_phone; j++) {
		p = (uint32)acmod_set_name2id(acmod_set, strtok(NULL, " \t"));
		out[i].phone[j] = (acmod_id_t)p;
		out[i].member[p] = TRUE;
	    }
	}
	else {
	    out[i].name = strdup(strtok(line, " \t"));
	    out[i].posn = ckd_calloc(N_WORD_POSN, sizeof(uint32));

	    if (strcmp(out[i].name, "WDBNDRY_B") == 0) {
		out[i].posn[(int)WORD_POSN_BEGIN] = TRUE;
	    }
	    else if (strcmp(out[i].name, "WDBNDRY_E") == 0) {
		out[i].posn[(int)WORD_POSN_END] = TRUE;
	    }
	    else if (strcmp(out[i].name, "WDBNDRY_S") == 0) {
		out[i].posn[(int)WORD_POSN_SINGLE] = TRUE;
	    }
	    else if (strcmp(out[i].name, "WDBNDRY_I") == 0) {
		out[i].posn[(int)WORD_POSN_INTERNAL] = TRUE;
	    }
	    else {
		E_FATAL("Unknown null question %s\n", out[i].name);
	    }

	    out[i].phone = NULL;
	}
    }

    return out;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:30:31  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
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
