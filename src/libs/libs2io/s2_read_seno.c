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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
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
 * File: s2_read_seno.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s2_read_seno.h>

#include <s3/s2_read_map.h>
#include <s3/s2_param.h>
#include <s3/model_inventory.h>
#include <s3/common.h>
#include <s3/s2io.h>
#include <s3/s3.h>

#include <math.h>

#include <s2/log.h>

#include <sys_compat/file.h>

#include <stdio.h>


static int
read_seno_cluster(int32 **seno_cluster,
		  const char *seno_dir,
		  const char *base_name,
		  const char **ext,
		  uint32 n_weights)
{
    unsigned int f;
    int n_read;
    char seno_filename[MAXPATHLEN];

    E_INFO("reading mixing weights for %s\n",
	   base_name);

    for (f = 0; f < S2_N_FEATURE; f++) {
	sprintf(seno_filename,
		"%s/%s.%s",
		seno_dir, base_name, ext[f]);

	areadint(seno_filename, &seno_cluster[f], &n_read);
	if (n_read != n_weights) {
	    E_FATAL("expected %d weights in %s but got %d\n",
		    n_weights, seno_filename, n_read);
	}
    }

    return S3_SUCCESS;
}

static void
exp_norm_floor_mixw(float32 ***out,		/* the mixture weight matrix for all shared states */
		    float32 weight_floor,	/* smoothing floor for mixture weights */
		    int32 **in,			/* mixture weights for states in a given CI phone */
		    uint32 ci_id,	/* the CI phone associated with the input weights */
		    uint32 ci_only,	/* Do CI initialization of non-CI mixture weights */
		    uint32 *cluster_offset,	/* mixture weight array offsets */
		    uint32 *state_of)	/* the model state associated with each senone.  Used only
						 * for CI initialization of CD weights. */
{
    uint32 cd_n_seno;	/* # of context dependent senones */
    uint32 s_out_org;	/* the first weight id under this CI_ID in the output matrix */
    uint32 s_out_next;	/* the first weight id of the next ci in the output matrix */
    uint32 s_out_ci_begin;	/* the first CI weight of this CI_ID in the output matrix */
    uint32 s_out_ci_next;	/* the next CI weight after the last CI weight for this CI_ID */
    uint32 s_in_ci_org;		/* the first weight id of the ci weights in the input matrix */
    uint32 f;		/* a feature stream id */
    uint32 s_in;	/* a weight id in the input matrix */
    uint32 s_out;	/* a weight id in the output matrix */
    uint32 cw;		/* a codeword index */

    s_out_org = cluster_offset[ci_id];
    s_out_next = cluster_offset[ci_id+1];

    if (!ci_only)
	cd_n_seno = s_out_next - s_out_org;
    else
	cd_n_seno = 0;

    s_in_ci_org = cd_n_seno;

    s_out_ci_begin = ci_id * (S2_N_STATE-1);
    s_out_ci_next = s_out_ci_begin + (S2_N_STATE-1);

    E_INFO("converting log(weights) to weights\n");
    
    for (f = 0; f < S2_N_FEATURE; f++) {
	for (s_out = s_out_ci_begin, s_in = s_in_ci_org;
	     s_out < s_out_ci_next; s_out++, s_in++) {
	    printf("%d %d :\n", f, s_in);
	    for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		out[s_out][f][cw] = EXP(in[f][s_in*S2_N_CODEWORD + cw]);
		printf("%f %.4e ", out[s_out][f][cw], EXP(in[f][s_in*S2_N_CODEWORD + cw]));
	    }
	    printf("\n");
	    vector_normalize(out[s_out][f], S2_N_CODEWORD);
	    vector_floor(out[s_out][f], S2_N_CODEWORD, weight_floor);
	    vector_normalize(out[s_out][f], S2_N_CODEWORD);
	}

	if (ci_only) {
	    /* clone the CD mixture weights from the CI ones */
	    for (s_out = s_out_org; s_out < s_out_next; s_out++) {
		
		/* figure out which CI weights we need */
		s_in = s_out_ci_begin + state_of[s_out];
		
		for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		    out[s_out][f][cw] = out[s_in][f][cw];
		}
	    }
	}
	else {
	    for (s_out = s_out_org, s_in = 0;
		 s_in < cd_n_seno; s_in++, s_out++) {
		for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		    out[s_out][f][cw] = EXP(in[f][s_in * S2_N_CODEWORD + cw]);
		}
		vector_normalize(out[s_out][f], S2_N_CODEWORD);
		vector_floor(out[s_out][f], S2_N_CODEWORD, weight_floor);
		vector_normalize(out[s_out][f], S2_N_CODEWORD);
	    }
	}

    }
}

static void
exp_mixw(float32 ***out,		/* the mixture weight matrix for all shared states */
	 int32 **in,			/* mixture weights for states in a given CI phone */
	 uint32 ci_id,		/* the CI phone associated with the input weights */
	 uint32 ci_only,	/* Do CI initialization of non-CI mixture weights */
	 uint32 *cluster_offset,/* mixture weight array offsets */
	 uint32 *state_of)	/* the model state associated with each senone.  Used only
					 * for CI initialization of CD weights. */
{
    uint32 cd_n_seno;	/* # of context dependent senones */
    uint32 s_out_org;	/* the first weight id under this CI_ID in the output matrix */
    uint32 s_out_next;	/* the first weight id of the next ci in the output matrix */
    uint32 s_out_ci_begin;	/* the first CI weight of this CI_ID in the output matrix */
    uint32 s_out_ci_next;	/* the next CI weight after the last CI weight for this CI_ID */
    uint32 s_in_ci_org;		/* the first weight id of the ci weights in the input matrix */
    uint32 f;		/* a feature stream id */
    uint32 s_in;	/* a weight id in the input matrix */
    uint32 s_out;	/* a weight id in the output matrix */
    uint32 cw;		/* a codeword index */

    s_out_org = cluster_offset[ci_id];
    s_out_next = cluster_offset[ci_id+1];

    if (!ci_only)
	cd_n_seno = s_out_next - s_out_org;
    else
	cd_n_seno = 0;

    s_in_ci_org = cd_n_seno;

    s_out_ci_begin = ci_id * (S2_N_STATE-1);
    s_out_ci_next = s_out_ci_begin + (S2_N_STATE-1);

    E_INFO("converting log(weights) to weights\n");
    
    for (f = 0; f < S2_N_FEATURE; f++) {
	for (s_out = s_out_ci_begin, s_in = s_in_ci_org;
	     s_out < s_out_ci_next; s_out++, s_in++) {
	    printf("CI %d %u <- %u :\n", f, s_out, s_in);
	    for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		out[s_out][f][cw] = EXP(in[f][s_in*S2_N_CODEWORD + cw]);
	    }
	}

	if (ci_only) {
	    /* clone the CD mixture weights from the CI ones */
	    for (s_out = s_out_org; s_out < s_out_next; s_out++) {
		
		/* figure out which CI weights we need */
		s_in = s_out_ci_begin + state_of[s_out];
		
		for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		    out[s_out][f][cw] = out[s_in][f][cw];
		}
	    }
	}
	else {
	    for (s_out = s_out_org, s_in = 0;
		 s_in < cd_n_seno; s_in++, s_out++) {
		printf("CD %d %u <- %u :\n", f, s_out, s_in);
		for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		    out[s_out][f][cw] = EXP(in[f][s_in * S2_N_CODEWORD + cw]);
		}
	    }
	}
    }
}
	 
float32 ***
s2_read_seno_3(acmod_set_t *acmod_set,		/* the phone set of the model inventory */
	       uint32 *cluster_offset,	/* number of senones before each base phone cluster */
	       const char *seno_dir,		/* the directory containing the .ccode, .d2code, etc. files */
	       uint32 **in_smap,	/* An initial global state sharing map */
	       float32 weight_floor,		/* the least probability of the weights prior to renormalization */
	       uint32 *state_of)	/* the model state id's for each shared state */
{
    float ***out;
    uint32 i, f;
    uint32 n_ci;
    int32 **in_seno;
    uint32 n_states;
    uint32 n_base_states;
    uint32 n_base_weights;
    const char **seno_filename_ext;
    uint32 ci_initial_weights;

    seno_filename_ext = ckd_calloc(S2_N_FEATURE, sizeof(char *));

    seno_filename_ext[0] = cmd_ln_access("-cepsenoext");
    seno_filename_ext[1] = cmd_ln_access("-dcepsenoext");
    seno_filename_ext[2] = cmd_ln_access("-powsenoext");
    seno_filename_ext[3] = cmd_ln_access("-2dcepsenoext");
    
    n_ci = acmod_set_n_ci(acmod_set);
    n_states = cluster_offset[n_ci];

    out = (float32 ***)ckd_calloc_3d(n_states,
				     S2_N_FEATURE,
				     S2_N_CODEWORD,
				     sizeof(float32));

    E_INFO("%dK in mixture weights\n",
	    n_states * S2_N_CODEWORD * S2_N_FEATURE / 1024);
    E_INFO("%ldK in array overhead\n",
	   ((n_states * S2_N_FEATURE * sizeof(float *)) +
	    (S2_N_FEATURE * sizeof(float **))) / 1024);

    if (in_smap == NULL) {
	E_INFO("initializing mixing weights with prior CI weights\n");
	ci_initial_weights = TRUE;
    }
    else {
	ci_initial_weights = FALSE;
    }

    n_base_weights = (S2_N_STATE-1) * S2_N_CODEWORD;

    in_seno = ckd_calloc(S2_N_FEATURE, sizeof(int32 *));

    for (i = 0; i < n_ci; i++) {
	if (!ci_initial_weights) {
	    n_base_states = cluster_offset[i+1] - cluster_offset[i];

	    /* diff of cluster offsets just accounts for shared triphone
	       states, not ci phone states.  Add ci state count */
	    n_base_states += S2_N_STATE-1;

	    n_base_weights = n_base_states * S2_N_CODEWORD;
	}

	read_seno_cluster(in_seno,
			  seno_dir,
			  acmod_set_id2name(acmod_set, i),
			  seno_filename_ext,
			  n_base_weights);
	
        exp_mixw(out, in_seno, i, ci_initial_weights,
		 cluster_offset, state_of);

	for (f = 0; f < S2_N_FEATURE; f++) {
	    ckd_free(in_seno[f]);
	}
    }

    ckd_free(in_seno);

    return out;
}
	 
float32 ***
s2_read_seno_2(acmod_set_t *acmod_set,		/* the phone set of the model inventory */
	       uint32 *cluster_offset,	/* number of senones before each base phone cluster */
	       const char *seno_dir,		/* the directory containing the .ccode, .d2code, etc. files */
	       const char *init_seno_map_filename,	/* An initial senone mapping */
	       float32 weight_floor,		/* the least probability of the weights prior to renormalization */
	       uint32 *state_of)	/* the model state id's for each shared state */
{
    float32 ***out;
    uint32 i, f;
    uint32 n_ci;
    uint32 *in_cluster_size;
    int32 **in_seno;
    uint32 n_states;
    uint32 n_base_states;
    uint32 n_base_weights;
    const char **seno_filename_ext;
    uint32 **in_smap;
    uint32 ci_initial_weights;

    in_smap = NULL;
    in_cluster_size = NULL;

    seno_filename_ext = ckd_calloc(S2_N_FEATURE, sizeof(char *));

    seno_filename_ext[0] = cmd_ln_access("-cepsenoext");
    seno_filename_ext[1] = cmd_ln_access("-dcepsenoext");
    seno_filename_ext[2] = cmd_ln_access("-powsenoext");
    seno_filename_ext[3] = cmd_ln_access("-2dcepsenoext");
    
    n_ci = acmod_set_n_ci(acmod_set);
    n_states = cluster_offset[n_ci];

    out = (float32 ***)ckd_calloc_3d(n_states,
				     S2_N_FEATURE,
				     S2_N_CODEWORD,
				     sizeof(float32));

    E_INFO("%dK in mixture weights\n",
	    n_states * S2_N_CODEWORD * S2_N_FEATURE / 1024);
    E_INFO("%ldK in array overhead\n",
	   ((n_states * S2_N_FEATURE * sizeof(float *)) +
	    (S2_N_FEATURE * sizeof(float **))) / 1024);

    if (init_seno_map_filename != NULL) {
	in_cluster_size = ckd_calloc(n_ci+1, sizeof(uint32));

	
	in_smap = (uint32 **)ckd_calloc_2d(acmod_set_n_acmod(acmod_set),
					   S2_N_STATE-1,
					   sizeof(uint32));
    
	s2_read_seno_mapping_file(in_smap,
				  in_cluster_size,
				  init_seno_map_filename,
				  acmod_set);

	ci_initial_weights = FALSE;
    }
    else {
	E_INFO("initializing mixing weights with prior CI weights\n");
	fflush(stderr);

	ci_initial_weights = TRUE;
    }

    n_base_weights = (S2_N_STATE-1) * S2_N_CODEWORD;

    in_seno = ckd_calloc(S2_N_FEATURE, sizeof(int32 *));

    for (i = 0; i < n_ci; i++) {
	if (!ci_initial_weights) {
	    n_base_states = cluster_offset[i+1] - cluster_offset[i];

	    /* diff of cluster offsets just accounts for shared triphone
	       states, not ci phone states.  Add ci state count */
	    n_base_states += S2_N_STATE-1;

	    n_base_weights = n_base_states * S2_N_CODEWORD;
	}

	read_seno_cluster(in_seno,
			  seno_dir,
			  acmod_set_id2name(acmod_set, i),
			  seno_filename_ext,
			  n_base_weights);
	
        exp_norm_floor_mixw(out, weight_floor, in_seno, i, ci_initial_weights,
			    cluster_offset, state_of);

	for (f = 0; f < S2_N_FEATURE; f++) {
	    ckd_free(in_seno[f]);
	}
    }

    ckd_free(in_seno);

    if (in_cluster_size != NULL)
	ckd_free(in_cluster_size);

    if (in_smap != NULL)
	ckd_free_2d((void **)in_smap);

    return out;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.11  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.10  1996/01/23  18:12:42  eht
 * Changes to remove either:
 * 	unused local variables
 * 	broken printf() format specifications
 * 	missing function prototypes in header files
 *
 * Revision 1.9  1995/12/15  18:37:07  eht
 * Added some type cases for memory alloc/free
 *
 * Revision 1.8  1995/10/17  14:03:23  eht
 * Changed to port to Windows NT
 *
 * Revision 1.7  1995/10/12  17:42:40  eht
 * Get SPHINX-II header files from <s2/...>
 *
 * Revision 1.6  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.5  1995/10/09  15:08:24  eht
 * changed ckd_alloc interface to remove need for __FILE__, __LINE__
 * arguments
 *
 * Revision 1.4  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.3  1995/08/15  13:45:33  eht
 * Made the diagnostic output less verbose
 *
 * Revision 1.2  1995/08/09  20:32:05  eht
 * No normalization of counts.
 *
 * Revision 1.1  1995/05/22  19:19:38  eht
 * Initial revision
 *
 *
 */

