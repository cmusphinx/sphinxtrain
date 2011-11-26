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
 */
/*********************************************************************
 *
 * File: best_q.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>

#include <s3/best_q.h>
#include <s3/metric.h>
#include <s3/s3.h>
#include <s3/div.h>

#include <stdio.h>
#include <string.h>

float64
best_q(float32 ****mixw,
       float32 ****means,
       float32 ****vars,
       uint32  *veclen,
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
       float64 node_wt_ent,  /* Weighted entropy of node */
       quest_t **out_best_q)
{
    float32 ***yes_dist;
    float32 ***yes_means=0;
    float32 ***yes_vars=0;
    float32 varfloor=0;
    float64 y_ent;
    float64 yes_dnom, yes_norm;
    uint32 *yes_id;
    float32 ***no_dist;
    float32 ***no_means=0;
    float32 ***no_vars=0;
    float64 n_ent;
    float64 no_dnom, no_norm;
    uint32 *no_id;
    uint32 n_yes, n_b_yes = 0;
    uint32 n_no, n_b_no = 0;
    uint32 i, j, k, q, b_q=0, s;
    uint32 ii;
    float64 einc, b_einc = -1.0e+50;

    const char*  type;
    uint32 continuous, sumveclen=0;

    type = cmd_ln_str("-ts2cbfn");
    if (strcmp(type,".semi.")!=0 && strcmp(type,".cont.") != 0)
        E_FATAL("Type %s unsupported; trees can only be built on types .semi. or .cont.\n",type);
    if (strcmp(type,".cont.") == 0)
        continuous = 1;
    else
        continuous = 0;

    if (continuous == 1) {
        varfloor = cmd_ln_float32("-varfloor");
        /* Allocating for sumveclen is overallocation, but it eases coding */
        for (ii=0,sumveclen=0;ii<n_stream;ii++) sumveclen += veclen[ii];
        yes_means = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
        yes_vars = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
        no_means = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
        no_vars = (float32 ***)ckd_calloc_3d(n_state,n_stream,sumveclen,sizeof(float32));
    }

    n_yes = n_no = 0;

    yes_dist = (float32 ***)ckd_calloc_3d(n_state, n_stream, n_density, sizeof(float32));
    no_dist = (float32 ***)ckd_calloc_3d(n_state, n_stream, n_density, sizeof(float32));

    for (q = 0; q < n_all_q; q++) {
	memset(&yes_dist[0][0][0], 0, sizeof(float32) * n_state * n_stream * n_density);
	memset(&no_dist[0][0][0], 0, sizeof(float32) * n_state * n_stream * n_density);

        if (continuous == 1) {
	    memset(&yes_means[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
	    memset(&yes_vars[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
	    memset(&no_means[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
	    memset(&no_vars[0][0][0], 0, sizeof(float32) * n_state * n_stream * sumveclen);
        }

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

        if (continuous == 1) {
            einc -=  node_wt_ent;
        }

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

    if (continuous == 1) {
        ckd_free_3d((void ***)yes_means);
        ckd_free_3d((void ***)yes_vars);
        ckd_free_3d((void ***)no_means);
        ckd_free_3d((void ***)no_vars);
    }

    *out_best_q = &all_q[b_q];

    return b_einc;
}


static int
next_subset(uint32 *s,
	    uint32 n,
	    uint32 r)
{
    int i; 
 
    i = r-1; 
 
    for (i = r-1; (i >= 0) && (s[i] == n-r+i); i--); 
 
    if (i < 0) 
        return FALSE; 
 
    s[i]++; 
 
    for (++i; i < r; i++) 
        s[i] = s[i-1] + 1; 
 
    return TRUE;
}


float64
two_class(float32 ****mixw_occ,
          float32 ****means,
          float32 ****vars,
          uint32 *veclen,
	  uint32 n_dist,
	  uint32 n_state,
	  uint32 n_stream,
	  uint32 n_density,
	  float32 *stwt,
	  uint32 *bclust,
	  float32 mwfloor)
{
    uint32 *clust;
    uint32 *set;
    float32 **a_dist;
    float32 **b_dist;
    float32 ***m_dist;
    float64 einc, b_einc;
    float64 norm;
    float64 a_wt, b_wt;
    uint32 i, s, j, k, l;
    float32 ***m_means=0;
    float32 ***m_vars=0;
    float32 **a_means=0;
    float32 **a_vars=0;
    float32 **b_means=0;
    float32 **b_vars=0;
    float32 varfloor=0;
    float64 root_ent=0;
    float64 a_ent;
    float64 b_ent;
    char*  type;
    uint32 continuous,sumveclen=0;

    type = (char *)cmd_ln_str("-ts2cbfn");
    if (strcmp(type,".semi.")!=0 && strcmp(type,".cont.") != 0)
        E_FATAL("Type %s unsupported; trees can only be built on types .semi. or .cont.\n",type);
    if (strcmp(type,".cont.") == 0)
        continuous = 1;
    else
        continuous = 0;

    if (continuous == 1) {
        varfloor = cmd_ln_float32("-varfloor");
        for(i=0,sumveclen=0; i < n_stream; i++) sumveclen += veclen[i];
        m_means = (float32 ***)ckd_calloc_3d(n_state, n_stream, sumveclen, sizeof(float32));
        m_vars = (float32 ***)ckd_calloc_3d(n_state, n_stream, sumveclen, sizeof(float32));
        a_means = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        a_vars = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        b_means = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        b_vars = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        root_ent = 0;
    }

    clust = ckd_calloc(n_dist, sizeof(uint32));
    set = ckd_calloc(n_dist-1, sizeof(uint32));

    m_dist = (float32 ***)ckd_calloc_3d(n_state, n_stream, n_density, sizeof(float32));

    /* Create 1-class merged distribution */
    for (s = 0; s < n_state; s++) {

	for (i = 0; i < n_dist; i++) {
	    for (j = 0; j < n_stream; j++) {
		for (k = 0; k < n_density; k++) {
		    m_dist[s][j][k] += mixw_occ[i][s][j][k];
		}
	    }
	}
        if (continuous == 1) {
	    for (i = 0; i < n_dist; i++) {
	        for (j = 0; j < n_stream; j++) {
		    for (k = 0; k < veclen[j]; k++) {
                        m_means[s][j][k] += mixw_occ[i][s][j][0] * means[i][s][j][k];
                        m_vars[s][j][k] += mixw_occ[i][s][j][0] * (vars[i][s][j][k] + means[i][s][j][k]*means[i][s][j][k]);
                    }
                }
            }
        }
	

	for (k = 0, norm = 0; k < n_density; k++) {
	    norm += m_dist[s][0][k];
	}

	norm = 1.0 / norm;
	
	for (j = 0; j < n_stream; j++) {
	    for (k = 0; k < n_density; k++) {
		m_dist[s][j][k] *= norm;
		if (m_dist[s][j][k] < mwfloor)
		    m_dist[s][j][k] = mwfloor;
	    }
	}
        if (continuous == 1) {
            for (j = 0; j < n_stream; j++) {
                for (k = 0; k < veclen[j]; k++) {
                    m_means[s][j][k] *= norm;
                    m_vars[s][j][k] = m_vars[s][j][k]*norm -
                                        m_means[s][j][k]*m_means[s][j][k];
                    if (m_vars[s][j][k] < varfloor) m_vars[s][j][k] = varfloor;
                }
                /* 1 / norm is the count */
                root_ent += (1.0/norm) * (float64)stwt[s] * ent_cont(m_means[s][j],m_vars[s][j],veclen[j]);
            }
        }
    }
    
    a_dist = (float32 **)ckd_calloc_2d(n_stream, n_density, sizeof(float32));
    b_dist = (float32 **)ckd_calloc_2d(n_stream, n_density, sizeof(float32));

    /*
     * Explore all subsets S of length 1 .. (n_dist-1)
     *
     * 	dist i in S => class A
     * 	dist i not in S => class B
     */
    for (i = 1, b_einc = -1.0e+50; i < n_dist; i++) {

	/* Generate the initial subset definition [0..i-1] */
	for (j = 0; j < i; j++) {
	    set[j] = j;
	}

	/* Check all subsets of length(i) */
	do {
	    for (j = 0; j < i; j++) {
		clust[set[j]] = TRUE;
	    }

	    for (s = 0, einc = 0; s < n_state; s++) {

		/* Given cluster definition, create A and B
		   distributions */
		for (j = 0; j < n_dist; j++) {
		    if (clust[j]) {
			for (k = 0; k < n_stream; k++) {
			    for (l = 0; l < n_density; l++) {
				a_dist[k][l] += mixw_occ[j][s][k][l];
			    }
			}
		    }
		    else {
			for (k = 0; k < n_stream; k++) {
			    for (l = 0; l < n_density; l++) {
				b_dist[k][l] += mixw_occ[j][s][k][l];
			    }
			}
		    }
                    if (continuous == 1) {
		        if (clust[j]) {
			    for (k = 0; k < n_stream; k++) {
			        for (l = 0; l < veclen[k]; l++) {
				    a_means[k][l] += mixw_occ[j][s][k][0] * 
							means[j][s][k][l];
				    a_vars[k][l] += mixw_occ[j][s][k][0] *
				       		(vars[j][s][k][l] + 
					   means[j][s][k][l]*means[j][s][k][l]);
			        }
			    }
		        }
		        else {
			    for (k = 0; k < n_stream; k++) {
			        for (l = 0; l < veclen[k]; l++) {
				    b_means[k][l] += mixw_occ[j][s][k][0] * 
							means[j][s][k][l];
				    b_vars[k][l] += mixw_occ[j][s][k][0] *
				       		(vars[j][s][k][l] + 
					   means[j][s][k][l]*means[j][s][k][l]);
		    	        }
		    	    }
		        }
		    }
		}

		for (l = 0, a_wt = 0; l < n_density; l++) {
		    a_wt += a_dist[0][l];
		}

		norm = 1.0 / a_wt;

		for (k = 0; k < n_stream; k++) {
		    for (l = 0; l < n_density; l++) {
			a_dist[k][l] *= norm;
		    }
		}
                if (continuous == 1) {
                    a_ent = 0;
		    for (k = 0; k < n_stream; k++) {
		        for (l = 0; l < veclen[k]; l++) {
		            a_means[k][l] *= norm;
                            a_vars[k][l] *= norm;
                            a_vars[k][l] -= a_means[k][l]*a_means[k][l];
                            if (a_vars[k][l] < varfloor)
                                a_vars[k][l] = varfloor;
		        }
                        a_ent += ent_cont(a_means[k],a_vars[k],veclen[k]);
		    }
                    a_ent *= a_wt;
                    einc += (float64)stwt[s] * a_ent;
		}

		for (k = 0, b_wt = 0; k < n_density; k++) {
		    b_wt += b_dist[0][k];
		}

		norm = 1.0 / b_wt;

		for (k = 0; k < n_stream; k++) {
		    for (l = 0; l < n_density; l++) {
			b_dist[k][l] *= norm;
		    }
		}
                if (continuous == 1) {
                    b_ent = 0;
		    for (k = 0; k < n_stream; k++) {
		        for (l = 0; l < veclen[k]; l++) {
		            b_means[k][l] *= norm;
                            b_vars[k][l] *= norm;
                            b_vars[k][l] -= b_means[k][l]*b_means[k][l];
                            if (b_vars[k][l] < varfloor)
                                b_vars[k][l] = varfloor;
		        }
                        b_ent += ent_cont(b_means[k],b_vars[k],veclen[k]);
		    }
                    b_ent *= b_wt;
                    einc += (float64)stwt[s] * b_ent;
		}
		/* Compute the weighted entropy increase of merging the
		   two clusters */
                if (continuous != 1) {
		    einc += (float64)stwt[s] * wt_ent_inc(a_dist, a_wt,
		    			     b_dist, b_wt,
					     m_dist[s], n_stream, n_density);
                }

                if (continuous == 1) {
		    memset(&a_means[0][0], 0, sizeof(float32) * n_stream * sumveclen);
		    memset(&a_vars[0][0], 0, sizeof(float32) * n_stream * sumveclen);
		    memset(&b_means[0][0], 0, sizeof(float32) * n_stream * sumveclen);
		    memset(&b_vars[0][0], 0, sizeof(float32) * n_stream * sumveclen);
                }

		memset(&a_dist[0][0], 0, sizeof(float32) * n_stream * n_density);
		memset(&b_dist[0][0], 0, sizeof(float32) * n_stream * n_density);
	    }
            if (continuous == 1) {
                einc -= root_ent;
            }
	    
	    /* Maximize the weighted entropy increase of the merge */
	    if (einc > b_einc) {
		b_einc = einc;
		
		for (j = 0; j < n_dist; j++) {
		    bclust[j] = clust[j];
		}
	    }

	    /* Zero clust for next iteration */
	    memset(clust, 0, sizeof(uint32) * n_dist);
	} while (next_subset(set, n_dist, i));
    }

    ckd_free((void *)clust);
    ckd_free((void *)set);

    ckd_free_2d((void **)a_dist);
    ckd_free_2d((void **)b_dist);
    ckd_free_3d((void ***)m_dist);

    if (continuous == 1) {
        ckd_free_3d((void ***)m_means);
        ckd_free_3d((void ***)m_vars);
        ckd_free_3d((void ***)a_means);
        ckd_free_3d((void ***)a_vars);
        ckd_free_3d((void ***)b_means);
        ckd_free_3d((void ***)b_vars);
    }

    return b_einc;
}
