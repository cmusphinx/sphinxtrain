/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: s2_read_seno.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/s2_write_seno.h>

#include <s3/s2_param.h>
#include <s3/ckd_alloc.h>
#include <s3/s2io.h>
#include <s3/err.h>
#include <s3/s3.h>
#include <s3/model_inventory.h>
#include <s3/common.h>

#include <s2/log.h>

#include <sys_compat/file.h>


static int
write_seno_cluster(int32 **seno_cluster,
		  const char *seno_dir,
		  const char *base_name,
		  const char **ext,
		  uint32 n_weights)
{
    unsigned int f;
    int n_written;
    char seno_filename[MAXPATHLEN];

    E_INFO("writing %d mixture weights for %s\n", n_weights, base_name);

    for (f = 0; f < S2_N_FEATURE; f++) {
	sprintf(seno_filename,
		"%s/%s.%s",
		seno_dir, base_name, ext[f]);

	n_written = awriteint(seno_filename, seno_cluster[f], n_weights);
	if (n_written != n_weights) {
	    fflush(stdout);
	    fprintf(stderr, "%s(%d): had %d weights in %s but only wrote %d\n",
		    __FILE__, __LINE__,
		    n_weights, seno_filename, n_written);
	    fflush(stderr);

	    /* can't reasonably proceed after this kind of wreckage */
	    exit(1);
	}
    }

    return S3_SUCCESS;
}

static void
norm_log_mixw(float32 ***new_mixw,		/* the mixture weight matrix for all shared states */
	      int32 **old_seno,			/* mixture weights for states in a given CI phone */
	      uint32 ci_id,		/* the CI phone associated with the input weights */
	      uint32 *cluster_offset,	/* mixture weight array offsets */
	      uint32 *state_of)		/* the model state associated with each senone.  Used only
						 * for CI initialization of CD weights. */
{
    int32 cd_n_seno;			/* # of context dependent senones */
    uint32 s_new_mixw_org;	/* the first weight id under this CI_ID in the output matrix */
    uint32 s_new_mixw_next;	/* the first weight id of the next ci in the output matrix */
    uint32 s_new_mixw_ci_begin;	/* the first CI weight of this CI_ID in the output matrix */
    uint32 s_new_mixw_ci_next;	/* the next CI weight after the last CI weight for this CI_ID */
    uint32 s_in_ci_org;		/* the first weight id of the ci weights in the input matrix */
    uint32 f;			/* a feature stream id */
    uint32 s_old_seno;		/* a weight id in the input matrix */
    uint32 s_new_mixw;		/* a weight id in the output matrix */
    uint32 cw;			/* a codeword index */

    s_new_mixw_org = cluster_offset[ci_id];
    s_new_mixw_next = cluster_offset[ci_id+1];

    cd_n_seno = s_new_mixw_next - s_new_mixw_org;

    E_INFO("ci_id == %d, cd_n_seno == %d\n", ci_id, cd_n_seno);
    E_INFO("cluster_offset[%d] == %d\n", ci_id, cluster_offset[ci_id]);
    E_INFO("cluster_offset[%d] == %d\n", ci_id+1, cluster_offset[ci_id+1]);

    if (cd_n_seno < 0) {
	/* can't reasonably proceed after this kind of wreckage */

	E_ERROR("Fewer than zero CD senones?!?  cd_n_seno == %d = %d - %d\n",
		cd_n_seno, s_new_mixw_next, s_new_mixw_org);
    }

    s_in_ci_org = cd_n_seno;

    s_new_mixw_ci_begin = ci_id * (S2_N_STATE-1);
    s_new_mixw_ci_next = s_new_mixw_ci_begin + (S2_N_STATE-1);

    E_INFO("converting weights to log(weights)\n");
    
    E_INFO("%d CD senones, plus %d CI senones == %d\n",
	   s_in_ci_org, (s_new_mixw_ci_next - s_new_mixw_ci_begin),
	   (s_in_ci_org + (s_new_mixw_ci_next - s_new_mixw_ci_begin)));
	
    for (f = 0; f < S2_N_FEATURE; f++) {
	for (s_new_mixw = s_new_mixw_org, s_old_seno = 0;
	     s_old_seno < s_in_ci_org; s_old_seno++, s_new_mixw++) {

	    vector_normalize(new_mixw[s_new_mixw][f], S2_N_CODEWORD);
	    for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		old_seno[f][s_old_seno * S2_N_CODEWORD + cw]
		    = LOG(new_mixw[s_new_mixw][f][cw]);
	    }
	}

	for (s_new_mixw = s_new_mixw_ci_begin, s_old_seno = s_in_ci_org;
	     s_new_mixw < s_new_mixw_ci_next; s_new_mixw++, s_old_seno++) {

	    vector_normalize(new_mixw[s_new_mixw][f], S2_N_CODEWORD);
	    for (cw = 0; cw < S2_N_CODEWORD; cw++) {
		old_seno[f][s_old_seno * S2_N_CODEWORD + cw]
		    = LOG(new_mixw[s_new_mixw][f][cw]);
	    }
	}

    }
}
	 
float32 ***
s2_write_seno_3(float ***new_mixw,		/* the sphinx-3 mixture weights we are saving */
		acmod_set_t *acmod_set,		/* the phone set of the model inventory */
		uint32 *cluster_offset,	/* number of senones before each base phone cluster */
		const char *seno_dir,		/* the directory containing the .ccode, .d2code, etc. files */
		uint32 **in_smap,	/* An initial global state sharing map */
		uint32 *state_of)	/* the model state id's for each shared state */
{
    uint32 i, f;
    uint32 n_ci;
    int32 **old_seno;
    uint32 n_states;
    uint32 n_base_states;
    uint32 n_base_weights;
    const char **seno_filename_ext;

    seno_filename_ext = ckd_calloc(S2_N_FEATURE, sizeof(char *));

    seno_filename_ext[0] = cmd_ln_access("-cepsenoext");
    seno_filename_ext[1] = cmd_ln_access("-dcepsenoext");
    seno_filename_ext[2] = cmd_ln_access("-powsenoext");
    seno_filename_ext[3] = cmd_ln_access("-2dcepsenoext");
    
    n_ci = acmod_set_n_ci(acmod_set);
    n_states = cluster_offset[n_ci];

#if 0
    new_mixw = ckd_calloc_3d(n_states,
			     S2_N_FEATURE,
			     S2_N_CODEWORD,
			     sizeof(float));
#endif

    E_INFO("%dK in mixture weights\n",
	   n_states * S2_N_CODEWORD * S2_N_FEATURE / 1024);
    E_INFO("%dK in array overhead\n",
	   ((n_states * S2_N_FEATURE * sizeof(float *)) +
	    (S2_N_FEATURE * sizeof(float **))) / 1024);

    n_base_weights = (S2_N_STATE-1) * S2_N_CODEWORD;

    old_seno = (int **) ckd_calloc(S2_N_FEATURE, sizeof(int32 *));

    for (i = 0; i < n_ci; i++) {
	n_base_states = cluster_offset[i+1] - cluster_offset[i];

	/* diff of cluster offsets just accounts for shared triphone
	   states, not ci phone states.  Add ci state count */
	n_base_states += S2_N_STATE-1;

	n_base_weights = n_base_states * S2_N_CODEWORD;

	for (f = 0; f < S2_N_FEATURE; f++) {
	    old_seno[f] = (int *) ckd_calloc(n_base_weights, sizeof(int32));
	} 

	/* turn new weights into old weights */
        norm_log_mixw(new_mixw, old_seno, i, cluster_offset, state_of);

	write_seno_cluster(old_seno,
			   seno_dir,
			   acmod_set_id2name(acmod_set, i),
			   seno_filename_ext,
			   n_base_weights);
	
	for (f = 0; f < S2_N_FEATURE; f++) {
	    ckd_free(old_seno[f]);
	} 
   }

    ckd_free(old_seno);

    return new_mixw;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
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
 * Revision 1.6  1995/10/17  14:03:23  eht
 * Changed to port to Windows NT
 *
 * Revision 1.5  1995/10/12  17:42:40  eht
 * Get SPHINX-II header files from <s2/...>
 *
 * Revision 1.4  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.3  1995/10/09  15:08:24  eht
 * changed ckd_alloc interface to remove need for __FILE__, __LINE__
 * arguments
 *
 * Revision 1.2  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.1  1995/09/07  19:26:41  eht
 * Initial revision
 *
 *
 */

