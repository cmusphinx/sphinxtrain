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
 * File: main.c
 * 
 * Description: 
 *    Make a SPHINX-II phone file given a list of phones (in SPHINX-III
 *    format).  Ok, this sounds like a bizarre fn, but I did need it
 *    once.
 *
 * Author: 
 *    Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <s3/s2_param.h>
#include <s3/ckd_alloc.h>
#include <s3/s2_read_map.h>

#include <s3/cmd_ln.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char big_str[4096];
    uint32 i, j;
    char **base_str;
    char **left_str;
    char **right_str;
    char **posn_str;
    uint32 n_base;
    uint32 n_tri;
    uint32 n_total;
    acmod_id_t base;
    acmod_id_t left;
    acmod_id_t right;
    word_posn_t posn;
    char *tok=0;
    acmod_set_t *acmod_set;
    char *pmap = WORD_POSN_CHAR_MAP;
    const char *filler_attr[] = {"filler", NULL};
    const char *base_attr[] = {"base", NULL};
    const char *na_attr[] = {"n/a", NULL};

    parse_cmd_ln(argc, argv);

    fp = fopen(cmd_ln_access("-phonelstfn"), "r");
    if (fp == NULL) {
	E_FATAL_SYSTEM("Can't open phone list file %u:", 
		       cmd_ln_access("-phonelstfn"));
    }
	
    for (n_total = 0; fgets(big_str, 4096, fp); n_total++);

    rewind(fp);

    base_str = ckd_calloc(n_total, sizeof(char *));
    left_str = ckd_calloc(n_total, sizeof(char *));
    right_str = ckd_calloc(n_total, sizeof(char *));
    posn_str = ckd_calloc(n_total, sizeof(char *));

    for (i = 0, n_base = 0, n_tri = 0; i < n_total; i++) {
	char tmp[64];

	if (fscanf(fp, "%s", tmp) != 1) {
	    E_FATAL("Error reading phone list file\n");
	}
	base_str[i] = strdup(tmp);
	
	if (fscanf(fp, "%s", tmp) != 1) {
	    E_FATAL("Error reading phone list file\n");
	}
	left_str[i] = strdup(tmp);
	
	if (fscanf(fp, "%s", tmp) != 1) {
	    E_FATAL("Error reading phone list file\n");
	}
	right_str[i] = strdup(tmp);
	
	if (fscanf(fp, "%s", tmp) != 1) {
	    E_FATAL("Error reading phone list file\n");
	}
	posn_str[i] = strdup(tmp);
	
	if ((strcmp(left_str[i], "-") == 0) &&
	    (strcmp(right_str[i], "-") == 0)) {
	    n_base++;
	}
	else if ((strcmp(left_str[i], "-") != 0) &&
		 (strcmp(right_str[i], "-") != 0)) {
	    n_tri++;
	}
	else {
	    E_FATAL("Unhandled phone %s %s %s %s\n",
		    base_str[i], left_str[i], right_str[i], posn_str[i]);
	}
    }

    fclose(fp);

    E_INFO("%d n_base, %d n_tri\n", n_base, n_tri);

    acmod_set = acmod_set_new();

    acmod_set_set_n_ci_hint(acmod_set, n_base);
    acmod_set_set_n_tri_hint(acmod_set, n_tri);

    fp = fopen(cmd_ln_access("-s2phonefn"), "w");
    if (fp == NULL) {
	E_FATAL_SYSTEM("Unable to open %s for writing",
		       cmd_ln_access("-s2phonefn"));
    }

    for (i = 0; i < n_base; i++) {
	if ((base_str[i][0] == '+') || (strncmp(base_str[i], "SIL", 3) == 0)) {
	    base = acmod_set_add_ci(acmod_set, base_str[i], filler_attr);
	}
	else {
	    base = acmod_set_add_ci(acmod_set, base_str[i], base_attr);
	}

	free(left_str[i]);
	left_str[i] = NULL;
	free(right_str[i]);
	right_str[i] = NULL;
	free(posn_str[i]);
	posn_str[i] = NULL;

	fprintf(fp, "%s 0 0 %u %u\n", base_str[i], base, base);
    }

    for (; i < n_total; i++) {
	base = acmod_set_name2id(acmod_set, base_str[i]);
	left = acmod_set_name2id(acmod_set, left_str[i]);
	right = acmod_set_name2id(acmod_set, right_str[i]);

	for (j = 0; j < strlen(pmap); j++) {
	    if (posn_str[i][0] == pmap[j])
		break;
	}
	if (j < strlen(pmap)) {
	    posn = (word_posn_t)j;
	}
	else {
	    E_WARN("unknown word position %s; using 'i'\n",
		   tok);
	    
	    posn = WORD_POSN_INTERNAL;
	}

	acmod_set_add_tri(acmod_set, base, left, right, posn, na_attr);

	fprintf(fp, "%s -1 0 %u %u\n",
	       acmod_set_id2s2name(acmod_set, i),
	       base,
	       i);
    }

    fclose(fp);

    exit(0);
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2004/07/21  18:30:37  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.2  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/11/22 21:23:18  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:27:30  eht
 * Initial revision
 * 
 *
 */
