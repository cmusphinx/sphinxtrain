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
 * File: two_class.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <s3/two_class.h>

#include <s3/ckd_alloc.h>
#include <s3/enum_subset.h>
#include <s3/metric.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>
#include <s3/cmd_ln.h>
#include <s3/div.h>

#include <string.h>

float64
two_class(float32 ****mixw_occ,
/* ADDITION FOR CONTINUOUS_TREES */
          float32 ****means,
          float32 ****vars,
          uint32 *veclen,
/* END ADDITION FOR CONTINUOUS_TREES */
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
/* ADDITION FOR CONTINUOUS_TREES */
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

    type = (char *)cmd_ln_access("-ts2cbfn");
    if (strcmp(type,".semi.")!=0 && strcmp(type,".cont.") != 0)
        E_FATAL("Type %s unsupported; trees can only be built on types .semi. or .cont.\n",type);
    if (strcmp(type,".cont.") == 0)
        continuous = 1;
    else
        continuous = 0;

    if (continuous == 1) {
        varfloor = *(float32 *)cmd_ln_access("-varfloor");
        for(i=0,sumveclen=0; i < n_stream; i++) sumveclen += veclen[i];
        m_means = (float32 ***)ckd_calloc_3d(n_state, n_stream, sumveclen, sizeof(float32));
        m_vars = (float32 ***)ckd_calloc_3d(n_state, n_stream, sumveclen, sizeof(float32));
        a_means = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        a_vars = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        b_means = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        b_vars = (float32 **)ckd_calloc_2d(n_stream, sumveclen, sizeof(float32));
        root_ent = 0;
    }
/* END OF ADDITIONS FOR CONTINUOUS_TREES */

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
/* ADDITION FOR CONTINUOUS_TREES FOR THE CONTINUOUS CASE */
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
/* END ADDITION FOR CONTINUOUS_TREES */
	

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
/* ADDITION FOR CONTINUOUS_TREES */ 
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
/* END ADDITION FOR CONTINUOUS_TREES */
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
/* ADDITIONS FOR CONTINUOUS_TREES to handle continuous HMMs */
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
/* END ADDITIONS FOR CONTINUOUS_TREES */
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
/* ADDITION FOR CONTINUOUS_TREES */
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
/* END ADDITION FOR CONTINUOUS_TREES */

		for (k = 0, b_wt = 0; k < n_density; k++) {
		    b_wt += b_dist[0][k];
		}

		norm = 1.0 / b_wt;

		for (k = 0; k < n_stream; k++) {
		    for (l = 0; l < n_density; l++) {
			b_dist[k][l] *= norm;
		    }
		}
/* ADDITION FOR CONTINUOUS_TREES */
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
/* END ADDITION FOR CONTINUOUS_TREES */

/* MODIFICATION FOR CONTINUOUS_TREES - Handle for continuous HMMs */
		/* Compute the weighted entropy increase of merging the
		   two clusters */
                if (continuous != 1) {
		    einc += (float64)stwt[s] * wt_ent_inc(a_dist, a_wt,
		    			     b_dist, b_wt,
					     m_dist[s], n_stream, n_density);
                }
/* END MODIFICATION FOR CONTINUOUS_TREES */
/* ADDITION FOR CONTINUOUS_TREES */

                if (continuous == 1) {
		    memset(&a_means[0][0], 0, sizeof(float32) * n_stream * sumveclen);
		    memset(&a_vars[0][0], 0, sizeof(float32) * n_stream * sumveclen);
		    memset(&b_means[0][0], 0, sizeof(float32) * n_stream * sumveclen);
		    memset(&b_vars[0][0], 0, sizeof(float32) * n_stream * sumveclen);
                }
/* END ADDITION FOR CONTINUOUS_TREES */

		memset(&a_dist[0][0], 0, sizeof(float32) * n_stream * n_density);
		memset(&b_dist[0][0], 0, sizeof(float32) * n_stream * n_density);
	    }
/* ADDITION FOR CONTINUOUS_TREES */
            if (continuous == 1) {
                einc -= root_ent;
            }
/* END ADDITION FOR CONTINUOUS_TREES */
	    
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

/* ADDITION FOR CONTINUOUS_TREES */
    if (continuous == 1) {
        ckd_free_3d((void ***)m_means);
        ckd_free_3d((void ***)m_vars);
        ckd_free_3d((void ***)a_means);
        ckd_free_3d((void ***)a_vars);
        ckd_free_3d((void ***)b_means);
        ckd_free_3d((void ***)b_vars);
    }
/* END ADDITION FOR CONTINUOUS_TREES */

    return b_einc;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:13  awb
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
