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
 * File: s2_map_read.c
 * 
 * Description: 
 *	
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s2_read_map.h>

#include <s3/ckd_alloc.h>
#include <s3/acmod_set.h>
#include <s3/s2_param.h>
#include <s3/prim_type.h>
#include <s3/s3.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>


static int
extract_state_class(char *str,
		    uint32 *state_id,
		    uint32 *tying_class)
{
    char *tmp;

    tmp = strchr(str, '<');

    if ( tmp == NULL ) {
	E_ERROR("expected '<' in %s\n",
		str);
	return S3_ERROR;
    }

    ++tmp;

    *state_id = atoi(tmp);

    tmp = strchr(tmp, '>');

    if ( tmp == NULL ) {
	E_ERROR("expected '>' in %s\n",
		str);
	return S3_ERROR;
    }

    ++tmp;

    *tying_class = atoi(tmp);

    return S3_SUCCESS;
}

/* Converts the state map, smap, into a global
   one-to-one mapping from model states onto
   consecutive integers from 0 to some max */
int
s2_convert_smap_to_global(acmod_set_t *acmod_set,
			  uint32 **smap,
			  uint32 **out_state_of,
			  uint32 *cluster_size)
{
    uint32 i;
    uint32 t;
    uint32 offset;
    uint32 n_ci = acmod_set_n_ci(acmod_set);
    uint32 n = acmod_set_n_multi(acmod_set) + n_ci;
    acmod_id_t b;
    acmod_id_t id;
    uint32 state;
    uint32 *state_of;
    uint32 ci_seno;
    
    offset = n_ci * (S2_N_STATE-1);

    E_INFO("|CI states| == %d\n", offset);

    /* convert the cluster_size matrix into an offset table */
    for (i = 0; i < n_ci; i++) {
	t = cluster_size[i];
	cluster_size[i] = offset;
	offset += t;

	E_INFO("|%s| == %d (%d)\n",
	       acmod_set_id2name(acmod_set, i),
	       t, offset);
    }

    cluster_size[i] = offset;	/* total # of states */

    state_of = ckd_calloc(offset, sizeof(uint32));

    /* map the ci phones to unshared distribution */
    for (id = 0; id < n_ci; id++) {
	for (state = 0; state < S2_N_STATE-1; state++) {
	    ci_seno = id * (S2_N_STATE-1) + state;
	    smap[id][state] = ci_seno;
	    state_of[ci_seno] = state;
	}
    }

    /* use the ci phone offsets to convert ci phone relative
       mappings to a global one-to-one mapping onto
       consecutive integers from 0 to some max */
    for (; id < n; id++) {
	for (state = 0; state < S2_N_STATE-1; state++) {
	    if (smap[id][state] == TYING_NO_ID) {
		uint32 base_id;

		base_id = acmod_set_base_phone(acmod_set, id);
		
		E_WARN("%s<%d> is unmapped, approximating with CI state <%d>.\n",
		       acmod_set_id2name(acmod_set, id),
		       state, state);

		smap[id][state] = smap[base_id][state];

		/* no state_of[] assignment need bee done since it was done
		   above */
	    }
	    else {
		b = acmod_set_base_phone(acmod_set, id);
		
		smap[id][state] += cluster_size[b];
		state_of[ smap[id][state] ] = state;
	    }
	}
    }

    *out_state_of = state_of;

    return S3_SUCCESS;
}

int
s2_read_seno_mapping_file(uint32 **smap,
			  uint32 *cluster_size,
			  const char *seno_mapping_file,
			  acmod_set_t *acmod_set)
{
    char big_str[1024];
    FILE *fp;
    acmod_id_t base;
    acmod_id_t left;
    acmod_id_t right;
    word_posn_t posn;
    acmod_id_t triacmod_id;
    uint32 state_id;
    uint32 tying_class;
    uint32 n_map = 0;
    
    fp = fopen(seno_mapping_file, "r");
    if (fp == NULL) {
	E_WARN_SYSTEM("can't open senone mapping file %s",
		      seno_mapping_file);
	return S3_ERROR;
    }

    /* read in the mapping for triphones */
    while (fgets(big_str, 1024, fp) != NULL) {
	big_str[strlen(big_str)-1] = '\0';

	acmod_set_s2_parse_triphone(acmod_set,
				    &base,
				    &left,
				    &right,
				    &posn,
				    big_str);

	triacmod_id = acmod_set_tri2id(acmod_set,
				       base,
				       left,
				       right,
				       posn);

	assert(triacmod_id != NO_ACMOD);

	extract_state_class(big_str, &state_id, &tying_class);
	
	if (cluster_size[base] < tying_class)
	    cluster_size[base] = tying_class;

	assert(state_id < S2_N_STATE-1);
	
	/* tying class in seno map file is 1 based, want zero based */
	smap[triacmod_id][state_id] = tying_class-1;
	++n_map;
    }

    return S3_SUCCESS;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:30:32  egouvea
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
 * Revision 1.8  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.7  1996/01/23  18:12:42  eht
 * Changes to remove either:
 * 	unused local variables
 * 	broken printf() format specifications
 * 	missing function prototypes in header files
 *
 * Revision 1.6  1995/10/12  17:42:40  eht
 * Get SPHINX-II header files from <s2/...>
 *
 * Revision 1.5  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.4  1995/10/09  15:08:24  eht
 * changed ckd_alloc interface to remove need for __FILE__, __LINE__
 * arguments
 *
 * Revision 1.3  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.2  1995/09/07  19:31:16  eht
 * For unmapped CD states, approximate them with the corresponding
 * CI state.
 *
 * Revision 1.1  1995/05/22  19:19:38  eht
 * Initial revision
 *
 *
 */
