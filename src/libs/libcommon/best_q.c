/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: best_q.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/best_q.h>

#include <s3/metric.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>
#include <s3/cmd_ln.h>
#include <s3/div.h>

#include <stdio.h>
#include <string.h>

float64
best_q(float32 ****mixw,
/* ADDITION FOR CONTINUOUS_TREES 21 May 98 */
       float32 ****means,
       float32 ****vars,
       uint32  *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
       uint32 n_model,
       uint32 n_state,
       uint32 n_stream,
       uint32 n_density,
       float32 *stwt,
       uint32 **dfeat,
       uint32 n_dfeat,
       quest_t *all_q,
       uint32 n_all_q,
       pset_t *pset,
       uint32 *id,
       uint32 n_id,
       float32 ***dist,
/* ADDITION FOR CONTINUOUS_TREES 21 May 98 */
       float64 node_wt_ent,  /* Weighted entropy of node */
/* END ADDITION FOR CONTINUOUS_TREES */
       quest_t **out_best_q)
{
    float32 ***yes_dist;
/* ADDITION FOR CONTINUOUS_TREES */
    float32 ***yes_means=0;
    float32 ***yes_vars=0;
    float32 varfloor=0;
    float64 y_ent;
/* END ADDITION FOR CONTINUOUS_TREES */
    float64 yes_dnom, yes_norm;
    uint32 *yes_id;
    float32 ***no_dist;
/* ADDITION FOR CONTINUOUS_TREES */
    float32 ***no_means=0;
    float32 ***no_vars=0;
    float64 n_ent;
/* END ADDITION FOR CONTINUOUS_TREES */
    float64 no_dnom, no_norm;
    uint32 *no_id;
    uint32 n_yes, n_b_yes = 0;
    uint32 n_no, n_b_no = 0;
    uint32 i, j, k, q, b_q=0, s;
    uint32 ii;
    float64 einc, b_einc = -1.0e+50;

/* ADDITION FOR CONTINUOUS_TREES; 20 May 98 */
    char*  type;
    uint32 continuous, sumveclen=0;

    type = (char *)cmd_ln_access("-ts2cbfn");
    if (strcmp(type,".semi.")!=0 && strcmp(type,".cont.") != 0)
        E_FATAL("Type %s unsupported; trees can only be built on types .semi. or .cont.\n",type);
    if (strcmp(type,".cont.") == 0)
        continuous = 1;
    else
        continuous = 0;

    if (continuous == 1) {
        varfloor = *(float32 *)cmd_ln_access("-varfloor");
        /* Allocating for sumveclen is overallocation, but it eases coding */
        for (ii=0,sumveclen=0;ii<n_stream;ii++) sumveclen += veclen[ii];
        yes_means = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
        yes_vars = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
        no_means = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
        no_vars = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
    }
/* END ADDITIONS FOR CONTINUOUS_TREES */

    n_yes = n_no = 0;

    yes_dist = (float32 ***)ckd_calloc_3d(n_state, n_stream, n_density, sizeof(float32));
    no_dist = (float32 ***)ckd_calloc_3d(n_state, n_stream, n_density, sizeof(float32));

    for (q = 0; q < n_all_q; q++) {
	memset(&yes_dist[0][0][0], 0, sizeof(float32) * n_state * n_stream * n_density);
	memset(&no_dist[0][0][0], 0, sizeof(float32) * n_state * n_stream * n_density);

/* ADDITION FOR CONTINUOUS_TREES; If continuous hmm initialize means and vars to zero */
        if (continuous == 1) {
	    memset(&yes_means[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
	    memset(&yes_vars[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
	    memset(&no_means[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
	    memset(&no_vars[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
        }
/* END ADDITION FOR CONTINUOUS_TREES */

	n_yes = n_no = 0;

	for (ii = 0; ii < n_id; ii++) {
	    i = id[ii];
	    if (eval_quest(&all_q[q], dfeat[i], n_dfeat)) {
	        for (s = 0; s < n_state; s++) {
		    for (j = 0; j < n_stream; j++) {
			for (k = 0; k < n_density; k++) {
			    yes_dist[s][j][k] += mixw[i][s][j][k];
			}
		    }
		}
 /* MODIFICATION FOR CONTINUOUS_TREES: ADDITIONS FOR CONTINUOUS CASE */
                if (continuous == 1) {
	            for (s = 0; s < n_state; s++) {
		        for (j = 0; j < n_stream; j++) {
			    for (k = 0; k < veclen[j]; k++) {
                                yes_means[s][j][k] += mixw[i][s][j][0] * means[i][s][j][k];
                                yes_vars[s][j][k] += mixw[i][s][j][0] * (vars[i][s][j][k] + means[i][s][j][k]*means[i][s][j][k]);
			    }
		        }
		    }
		}
/* END MODIFICATION FOR CONTINUOUS_TREES */
		++n_yes;
	    }
	    else {
		for (s = 0; s < n_state; s++) {
		    for (j = 0; j < n_stream; j++) {
			for (k = 0; k < n_density; k++) {
			    no_dist[s][j][k] += mixw[i][s][j][k];
			}
		    }
		}
 /* MODIFICATION FOR CONTINUOUS_TREES: ADDITIONS FOR CONTINUOUS CASE */
                if (continuous == 1) {
	            for (s = 0; s < n_state; s++) {
		        for (j = 0; j < n_stream; j++) {
			    for (k = 0; k < veclen[j]; k++) {
                                no_means[s][j][k] += mixw[i][s][j][0] * means[i][s][j][k];
                                no_vars[s][j][k] += mixw[i][s][j][0] * (vars[i][s][j][k] + means[i][s][j][k]*means[i][s][j][k]);
			    }
		        }
		    }
		}
/* END MODIFICATION FOR CONTINUOUS_TREES */
		++n_no;
	    }
	}
	
	if ((n_yes == 0) || (n_no == 0)) {
	    /* no split.  All satisfy or all don't satisfy */
	    continue;
	}


	for (s = 0, einc = 0; s < n_state; s++) {
	    for (k = 0, yes_dnom = 0; k < n_density; k++) {
		yes_dnom += yes_dist[s][0][k];
	    }
	
	    if (yes_dnom == 0)
	      break;

	    yes_norm = 1.0 / yes_dnom;
	
	    for (j = 0; j < n_stream; j++) {
		for (k = 0; k < n_density; k++) {
		    yes_dist[s][j][k] *= yes_norm;
		}
	    }
	
	    for (k = 0, no_dnom = 0; k < n_density; k++) {
		no_dnom += no_dist[s][0][k];
	    }
	
	    if (no_dnom == 0)
	      break;

	    no_norm = 1.0 / no_dnom;
	    
	    for (j = 0; j < n_stream; j++) {
		for (k = 0; k < n_density; k++) {
		    no_dist[s][j][k] *= no_norm;
		}
	    }

/* MODIFICATION FOR CONTINUOUS_TREES: Do appropriate operations for discrete and
   continuous */
            if (continuous == 1) {
                y_ent = 0;
                n_ent = 0;
	        for (j = 0; j < n_stream; j++) {
                    if (yes_dnom != 0) {
   		        for (k = 0; k < veclen[j]; k++) {
                            yes_means[s][j][k] *= yes_norm;
                            yes_vars[s][j][k] = yes_vars[s][j][k]*yes_norm - 
                                          yes_means[s][j][k]*yes_means[s][j][k];
                            if (yes_vars[s][j][k] < varfloor) yes_vars[s][j][k] = varfloor;
                        }
                    }
                    if (no_dnom != 0) {
   		        for (k = 0; k < veclen[j]; k++) {
                            no_means[s][j][k] *= no_norm;
                            no_vars[s][j][k] = no_vars[s][j][k]*no_norm - 
                                            no_means[s][j][k]*no_means[s][j][k];
                            if (no_vars[s][j][k] < varfloor) no_vars[s][j][k] = varfloor;
                        }
                    }
                    y_ent +=  yes_dnom * ent_cont(yes_means[s][j],yes_vars[s][j],veclen[j]);
                    n_ent +=  no_dnom * ent_cont(no_means[s][j],no_vars[s][j],veclen[j]);
                }
                einc += (float64)stwt[s] * (y_ent + n_ent);
            }
            else {
	        einc += (float64)stwt[s] * wt_ent_inc(yes_dist[s], yes_dnom,
	    				     no_dist[s], no_dnom,
					     dist[s], n_stream, n_density);
            }
	}
/* END MODIFICATION FOR CONTINUOUS_TREES */

/* ADDITION FOR CONTINUOUS_TREES; In current code this is true only for continous HMM */
        if (continuous == 1) {
            einc -=  node_wt_ent;
        }
/* END ADDITION FOR CONTINUOUS_TREES */

	if (s < n_state) {
	  /* Ended iteration over states prematurely; assume 'bad' question */
	  continue;
	}
	
	if (einc > b_einc) {
	    b_einc = einc;
	    b_q = q;
	    n_b_yes = n_yes;
	    n_b_no = n_no;
	}
    }

    if ((n_b_yes == 0) || (n_b_no == 0)) {
	/* No best question */
	*out_best_q = NULL;

	return 0;
    }

    yes_id = (uint32 *)ckd_calloc(n_b_yes, sizeof(uint32));
    no_id  = (uint32 *)ckd_calloc(n_b_no, sizeof(uint32));

    memset(&yes_dist[0][0][0], 0, sizeof(float32) * n_state * n_stream * n_density);
    memset(&no_dist[0][0][0], 0, sizeof(float32) * n_state * n_stream * n_density);
    n_yes = n_no = 0;

    for (ii = 0; ii < n_id; ii++) {
	i = id[ii];
	if (eval_quest(&all_q[b_q], dfeat[i], n_dfeat)) {
	    for (s = 0; s < n_state; s++) {
		for (j = 0; j < n_stream; j++) {
		    for (k = 0; k < n_density; k++) {
			yes_dist[s][j][k] += mixw[i][s][j][k];
		    }
		}
	    }
	    yes_id[n_yes] = i;
	    ++n_yes;
	}
	else {
	    for (s = 0; s < n_state; s++) {
		for (j = 0; j < n_stream; j++) {
		    for (k = 0; k < n_density; k++) {
			no_dist[s][j][k] += mixw[i][s][j][k];
		    }
		}
	    }
	    no_id[n_no] = i;
	    ++n_no;
	}
    }
    
    ckd_free_3d((void ***)yes_dist);
    ckd_free((void *)yes_id);
    ckd_free_3d((void ***)no_dist);
    ckd_free((void *)no_id);

/* ADDITION FOR CONTINUOUS_TREES */
    if (continuous == 1) {
        ckd_free_3d((void ***)yes_means);
        ckd_free_3d((void ***)yes_vars);
        ckd_free_3d((void ***)no_means);
        ckd_free_3d((void ***)no_vars);
    }
/* END ADDITION FOR CONTINUOUS_TREES */

    *out_best_q = &all_q[b_q];

    return b_einc;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
