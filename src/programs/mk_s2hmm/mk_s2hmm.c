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
#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/model_def_io.h>
#include <s3/state.h>
#include <s3/s3tmat_io.h>
#include <s3/s3mixw_io.h>
#include <s3/ckd_alloc.h>
#include <s3/vector.h>
#include <s3/cmd_ln.h>
#include <s3/err.h>
#include <s3/s3.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s2_write_hmm.h>
#include <s3/s2_write_seno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int 
write_seno(model_def_t *mdef,
	   float32 ***mixw,
	   uint32 n_mixw,
	   uint32 n_feat,
	   uint32 n_gauden,
	   const char *dir)
{
    uint32 *cluster_offset;
    uint32 **smap;
    uint32 n_base_phone;
    uint32 max_int;
    uint32 *state_of;
    uint32 sstate;
    acmod_id_t base;
    acmod_id_t p;
    int i, j;

    /*
     * Make the mapping table to go from the global senones to the
     * Sphinx-II local ones.
     */
    smap = (uint32 **)ckd_calloc_2d(acmod_set_n_acmod(mdef->acmod_set),
				    S2_N_STATE-1,
				    sizeof(uint32));

    n_base_phone = acmod_set_n_ci(mdef->acmod_set);

    cluster_offset = (uint32 *)ckd_calloc(n_base_phone+1, sizeof(uint32));

    max_int = 0;
    --max_int;	/* create max int for any unsigned int */
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
		    E_INFO("%s %d appears in model states %d and %d\n",
			   acmod_set_id2name(mdef->acmod_set,
					     acmod_set_base_phone(mdef->acmod_set,
								  mdef->defn[i].p)),
			   sstate,
			   state_of[sstate],
			   j);
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

    for (i = 0; i < n_base_phone; i++) {
	if (cluster_offset[i] != max_int) {
	    E_INFO("%s offset %d\n",
		   acmod_set_id2name(mdef->acmod_set, i),
		   cluster_offset[i]);
	}
	else {
	    E_INFO("%s <no CD states>\n",
		   acmod_set_id2name(mdef->acmod_set, i));
	}
    }
    
    /*
     * Output it in the old format.
     */

    s2_write_seno_3(mixw,
		    mdef->acmod_set,
		    cluster_offset,
		    dir,
		    smap,
		    state_of);
    
    return S3_SUCCESS;
}

int main(int argc, char *argv[])
{
    model_def_t *mdef;
    float32 ***tmat;
    uint32 n_tmat;
    uint32 n_state_pm;
    float32 ***mixw;
    uint32 n_mixw;
    uint32 n_stream;
    uint32 n_density;
    unsigned int i, j;
    float32 tprob_floor;
    const char *mtype;

    parse_cmd_ln(argc, argv);

    /* 
     * Read the new model
     */

    E_INFO("Reading: %s\n", cmd_ln_access("-moddeffn")); 

    if (model_def_read(&mdef, cmd_ln_access("-moddeffn")) != S3_SUCCESS) {
	return S3_ERROR;
    }

    tprob_floor = *(float32 *)cmd_ln_access("-tpfloor");

    E_INFO("Reading: %s and applying %e floor\n",
	   cmd_ln_access("-tmatfn"), tprob_floor);

    if (s3tmat_read(cmd_ln_access("-tmatfn"),
		    &tmat,
		    &n_tmat,
		    &n_state_pm) != S3_SUCCESS) {
	return S3_ERROR;
    }
    
    for (i = 0; i < n_tmat; i++)
	for (j = 0; j < n_state_pm-1; j++)
	    vector_nz_floor(tmat[i][j], n_state_pm, tprob_floor);

    mtype = (const char *)cmd_ln_access("-mtype");
    if (strcmp(mtype, "sdm") == 0) {
	E_INFO("Outputting shared distribution models\n");

	E_INFO("Reading: %s\n", cmd_ln_access("-mixwfn"));
	if (s3mixw_read((const char *)cmd_ln_access("-mixwfn"),
			&mixw,
			&n_mixw,
			&n_stream,
			&n_density) != S3_SUCCESS) {
	    E_ERROR_SYSTEM("Unable to read %s", cmd_ln_access("-mixwfn"));
	    
	    exit(1);
	}

	s2_write_hmm(tmat,
		     mdef->acmod_set,
		     cmd_ln_access("-hmmdir"));

	write_seno(mdef, mixw, n_mixw, n_stream, n_density,
		   cmd_ln_access("-hmmdir"));
    }
    else if (strcmp(mtype, "dhmm") == 0) {
	E_INFO("Outputting discrete models\n");

	if (s3mixw_read((const char *)cmd_ln_access("-mixwfn"),
			&mixw,
			&n_mixw,
			&n_stream,
			&n_density) != S3_SUCCESS) {
	    E_ERROR_SYSTEM("Unable to read %s", cmd_ln_access("-mixwfn"));
	    
	    exit(1);
	}

	s2_write_dhmm(tmat,
		      mixw,
		      mdef,
		      cmd_ln_access("-hmmdir"));
    }
    else {
	E_ERROR("Unhandled model type %s\n");
	exit(1);
    }
	
    exit(0);
}
