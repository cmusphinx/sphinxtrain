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
 * File: mk_s3mixw.c
 * 
 * Description: 
 * 	Make a SPHINX-3 mixing weight file from SPHINX-2 SDM models.
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/model_def_io.h>
#include <s3/acmod_set.h>
#include <s3/ckd_alloc.h>
#include <s3/cmd_ln.h>
#include <s3/s2_read_seno.h>
#include <s3/s3mixw_io.h>
#include <s3/s2_param.h>
#include <s3/s3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#define NO_STATE	0xffffffff

static char *ord_suff(uint32 i)
{
    return (i == 1 ? "st" :
	    (i == 2 ? "nd" :
	     (i == 3 ? "rd" : "th")));
}

int
main(int argc, char *argv[])
{
    model_def_t *mdef;
    model_def_entry_t *defn;
    uint32 n_defn;
    uint32 *cluster_offset;
    uint32 max_int;
    uint32 *state_of;
    uint32 max_state;
    uint32 sstate;
    int32 i;
    uint32 j;
    uint32 n_base_phone;
    acmod_id_t base;
    acmod_id_t p;
    float32 ***out;
    uint32 **smap;
    char comment[4192];
    time_t t;

    parse_cmd_ln(argc, argv);
    
    printf("%s(%d): Reading model definition file %s\n",
	   __FILE__, __LINE__, (const char *)cmd_ln_access("-moddeffn"));
    
    if (model_def_read(&mdef, cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	exit(1);
    }

    defn = mdef->defn;
    n_defn = mdef->n_defn;

    printf("%s(%d): %d models defined\n",
	   __FILE__, __LINE__, n_defn);

    smap = ckd_calloc(n_defn, sizeof(uint32 *));

    n_base_phone = acmod_set_n_ci(mdef->acmod_set);

    cluster_offset = ckd_calloc(n_base_phone+1, sizeof(uint32));

    max_int = 0;
    --max_int;	/* underflow offset values to max value */
    for (i = 0; i < n_base_phone; i++) {
	cluster_offset[i] = max_int;	
    }

    for (i = 0, max_state = 0; i < n_defn; i++) {
	for (j = 0; j < defn[i].n_state; j++) {
	    sstate = defn[i].state[j];
	    
	    if ((sstate != TYING_NON_EMITTING) &&
		(defn[i].state[j] > max_state)) max_state = defn[i].state[j];
	}
    }

    /* record the total # of senones */
    cluster_offset[n_base_phone] = max_state+1;

    state_of = ckd_calloc(max_state+1, sizeof(uint32));

    for (i = 0; i <= max_state; i++)
	state_of[i] = NO_STATE;
    
    for (i = 0; i < n_defn; i++) {
	p = defn[i].p;
	base = acmod_set_base_phone(mdef->acmod_set, defn[i].p);

	smap[i] = defn[i].state;
	
	for (j = 0; j < defn[i].n_state; j++) {
	    sstate = defn[i].state[j];

	    if (sstate != TYING_NON_EMITTING) {
		if (state_of[sstate] == NO_STATE)
		    state_of[sstate] = j;
		else if (state_of[sstate] != j) {
		    printf("%s %d appears as %d%s and %d%s model states\n",
			   acmod_set_id2name(mdef->acmod_set, acmod_set_base_phone(mdef->acmod_set, defn[i].p)),
			   sstate,
			   state_of[sstate],
			   ord_suff(state_of[sstate]),
			   j,
			   ord_suff(j));
		}

		if ((p != base) && (cluster_offset[base] > sstate)) {
		    cluster_offset[base] = sstate;
		}
	    }
	}
    }
    
    /* any untouched CLUSTER_OFFSET's implies a base phone
       without any CD states.  So offset is same as next
       one */
    for (i = (n_base_phone - 1); i >= 0 ; i--) {
	if (cluster_offset[i] == max_int)
	    cluster_offset[i] = cluster_offset[i+1];
    }

    fflush(stdout);
    for (i = 0; i < n_base_phone; i++) {
	if (cluster_offset[i] != max_int) {
	    fprintf(stderr, "%s(%d): %s offset %d\n",
		    __FILE__, __LINE__,
		    acmod_set_id2name(mdef->acmod_set, i), cluster_offset[i]);
	}
	else {
	    fprintf(stderr, "%s(%d): %s <no CD states>\n",
		    __FILE__, __LINE__,
		    acmod_set_id2name(mdef->acmod_set, i));
	}
    }
    fflush(stderr);

    printf("%s(%d): Reading senone weights in %s with floor %e\n",
	   __FILE__, __LINE__, (const char *)cmd_ln_access("-hmmdir"),
	   *(float32 *)cmd_ln_access("-floor"));
    
    out = s2_read_seno_3(mdef->acmod_set, cluster_offset,
			 cmd_ln_access("-hmmdir"),
			 (*(int32 *)cmd_ln_access("-ci2cd") ? NULL : smap),
			 *(float32 *)cmd_ln_access("-floor"),
			 state_of);

    t = time(NULL);
    sprintf(comment,
	    "Generated on %s\n\tmoddeffn: %s\n\tfloor: %e\n\thmmdir: %s\n\n\n\n\n\n\n\n\n",
	    ctime(&t),
	    (const char *)cmd_ln_access("-moddeffn"),
	    *(float32 *)cmd_ln_access("-floor"),
	    (const char *)cmd_ln_access("-hmmdir"));

    fflush(stdout);
    fprintf(stderr, "%s(%d): writing %s\n",
	    __FILE__, __LINE__,
	    (const char *)cmd_ln_access("-mixwfn"));
    fflush(stderr);
    
    if (s3mixw_write(cmd_ln_access("-mixwfn"),
		     out,
		     cluster_offset[n_base_phone],	/* total # states */
		     S2_N_FEATURE,
		     S2_N_CODEWORD) != S3_SUCCESS) {
	fflush(stdout);
	fprintf(stderr, "%s(%d): couldn't write mixture weight file\n",
		__FILE__, __LINE__);
	perror(cmd_ln_access("-mixwfn"));
	fflush(stderr);
    }
		     
    ckd_free(state_of);
    ckd_free(cluster_offset);

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
 * Revision 1.2  97/07/16  11:23:47  eht
 * Library reorganization changes
 * 
 * Revision 1.1  97/03/07  08:56:17  eht
 * Initial revision
 * 
 *
 */
