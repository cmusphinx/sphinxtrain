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
 * File: mk_s2seno.c
 * 
 * Description: 
 *    Make SPHINX-2 senone files from compatible SPHINX-3 format files
 *
 * Author: 
 *    Eric H. Thayer
 *********************************************************************/

#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s2_read_map.h>
#include <s3/s2_write_seno.h>

#include <s3/cmd_ln.h>

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

int main(int argc, char *argv[])
{
    model_inventory_t *inv;
    time_t t;
    model_def_t *mdef;
    uint32 *cluster_offset;
    uint32 **smap;
    uint32 n_base_phone;
    uint32 max_int;
    uint32 *state_of;
    uint32 sstate;
    acmod_id_t base;
    acmod_id_t p;
    float32 ***mixw;
    uint32 n_mixw;
    uint32 n_feat;
    uint32 n_gauden;
    int i, j;

    parse_cmd_ln(argc, argv);

    inv = mod_inv_new();

    /* 
     * Read the new model
     */

    fflush(stdout);
    fprintf(stderr, "%s(%d): reading model definitions in %s\n",
	    __FILE__, __LINE__,
	    (char *)cmd_ln_access("-moddeffn")); 
    fflush(stderr);

    if (model_def_read(&mdef, cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	return S3_ERROR;
    }

    /*
     * Make the mapping table to go from the global senones to the
     * Sphinx-II local ones.
     */
    smap = (uint32 **)ckd_calloc_2d(acmod_set_n_acmod(mdef->acmod_set),
				    S2_N_STATE-1,
				    sizeof(uint32));

    n_base_phone = acmod_set_n_ci(mdef->acmod_set);

    cluster_offset = ckd_calloc(n_base_phone+1, sizeof(uint32));

    max_int = 0;
    --max_int;	/* underflow offset values to max value */
    for (i = 0; i < n_base_phone; i++) {
	cluster_offset[i] = max_int;	
    }

    /* record the total # of senones */
    cluster_offset[n_base_phone] = mdef->n_tied_state;

    state_of = ckd_calloc(mdef->n_tied_state, sizeof(uint32));
    for (i = 0; i < mdef->n_tied_state; i++)
	state_of[i] = NO_STATE;
    
    for (i = 0; i < mdef->n_defn; i++) {
	p = mdef->defn[i].p;
	base = acmod_set_base_phone(mdef->acmod_set, mdef->defn[i].p);

	smap[i] = mdef->defn[i].state;
	
	for (j = 0; j < mdef->defn[i].n_state; j++) {
	    sstate = mdef->defn[i].state[j];

	    /* skip CI states, because we want to compute the # of CD states */
	    if (sstate < mdef->n_tied_ci_state)
		continue;

	    if (sstate != TYING_NON_EMITTING) {
		if (state_of[sstate] == NO_STATE)
		    state_of[sstate] = j;
		else if (state_of[sstate] != j) {
		    printf("%s %d appears as %d%s and %d%s model states\n",
			   acmod_set_id2name(mdef->acmod_set,
					     acmod_set_base_phone(mdef->acmod_set,
								  mdef->defn[i].p)),
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
    /*
     * PWP: for this to work right for SEVERAL base phones without
     * any CD states, we must count backwards from the end.
     */
    for (i = (n_base_phone - 1); i >= 0 ; i--) {
	assert (i < n_base_phone);
	if (cluster_offset[i] == max_int)
	    cluster_offset[i] = cluster_offset[i+1];
    }

    fflush(stdout);
    for (i = 0; i < n_base_phone; i++) {
	if (cluster_offset[i] != max_int) {
	    fprintf(stderr, "%s(%d): %s offset %d\n",
		    __FILE__, __LINE__,
		    acmod_set_id2name(mdef->acmod_set, i),
		    cluster_offset[i]);
	}
	else {
	    fprintf(stderr, "%s(%d): %s <no CD states>\n",
		    __FILE__, __LINE__,
		    acmod_set_id2name(mdef->acmod_set, i));
	}
    }
    fflush(stderr);


    /* (end of map making) */



    inv->acmod_set = mdef->acmod_set;

    fflush(stdout);
    fprintf(stderr, "%s(%d): reading mixture weights in %s\n",
	    __FILE__, __LINE__,
	    (char *)cmd_ln_access("-mixwfn"));
    fflush(stderr);

    if (s3mixw_read(cmd_ln_access("-mixwfn"),
		    &mixw,
		    &n_mixw,
		    &n_feat,
		    &n_gauden) != S3_SUCCESS) {
	return S3_ERROR;
    }

    inv->mixw = mixw;
    inv->n_mixw = n_mixw;
    inv->mixw_acc = (float32 ***) ckd_calloc_3d(n_mixw, n_feat, n_gauden,
						sizeof(float32));

    /*
     * Output it in the old format.
     */

    s2_write_seno_3(inv->mixw,
		    mdef->acmod_set,
		    cluster_offset,
		    cmd_ln_access("-hmmdir"),
		    smap,
		    state_of);

    t = time(NULL);

    exit(0);
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/11/22  21:23:18  awb
 * *** empty log message ***
 * 
 * Revision 1.7  97/07/16  11:25:00  eht
 * Use newer ckd_alloc calls
 * 
 * Revision 1.6  97/03/07  08:58:15  eht
 * - deal w/ new i/o routines
 * 
 *
 */
