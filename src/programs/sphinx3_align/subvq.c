/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
/*
 * subvq.c
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.12  2006/02/22  17:43:31  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH:
 * 1, vector_gautbl_free is not appropiate to be used in this case because it will free a certain piece of memory twice.
 * 2, Fixed dox-doc.
 * 
 * Revision 1.11.4.1  2005/10/17 04:44:45  arthchan2003
 * Free subvq_t correctly.
 *
 * Revision 1.11  2005/06/21 19:01:33  arthchan2003
 * Added $ keyword.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Updated subvq_free () to free allocated memory
 * 
 * 17-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added handling of a single sub-vector in subvq_mgau_shortlist().
 * 
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changes due to moving subvq_t.{frm_sen_eval,frm_gau_eval} to cont_mgau.h.
 * 
 * 14-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_t.{frm_sen_eval,frm_gau_eval}.  Changed subvq_frame_eval to
 * 		return the normalization factor.
 * 
 * 06-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_subvec_eval_logs3().
 * 
 * 14-Oct-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed ci_active flags input to sen_active in subvq_frame_eval().
 * 
 * 21-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix in subvq_init() that used g for allocating working areas, even
 *		though g wasn't defined.
 * 
 * 20-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_gautbl_eval_logs3() and used it in subvq_frame_eval().
 * 
 * 12-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <math.h>
#include <string.h>

#include "subvq.h"
#include "s3types.h"

/*
 * Precompute variances/(covariance-matrix-determinants) to simplify Mahalanobis distance
 * calculation.  Also, calculate 1/(det) for the original codebooks, based on the VQ vars.
 * (See comment in libmisc/vector.h.)
 */
static void
subvq_maha_precomp(subvq_t * svq, float64 floor)
{
    int32 s;
    float32 *lrd;
    vector_gautbl_t *gautbl;

    E_INFO("Precomputing Mahalanobis distance invariants\n");

    lrd = (float32 *) ckd_calloc(svq->n_sv * svq->vqsize, sizeof(float32));

    for (s = 0; s < svq->n_sv; s++) {
        gautbl = &(svq->gautbl[s]);

        vector_gautbl_var_floor(gautbl, floor);

        gautbl->lrd = lrd;
        lrd += svq->vqsize;
        vector_gautbl_maha_precomp(gautbl);
    }
}


static void
subvq_map_compact(subvq_t * vq, mgau_model_t * g)
{
    int32 r, c, c2, s;

    if (g) {
        if ((g->n_mgau != vq->origsize.r)
            || (g->max_comp != vq->origsize.c))
            E_FATAL
                ("Model size conflict: %d x %d (SubVQ) vs %d x %d (Original)\n",
                 vq->origsize.r, vq->origsize.c, g->n_mgau, g->max_comp);
    }

    /*
     * Compress map entries by removing invalid ones.  NOTE: There's not much of a consistency
     * check to ensure that the entries remaining do map correctly on to the valid entries in
     * the original (parent) mixture Gaussian model g.  The only check we have is to verify
     * the NUMBER of valid entries (components) in each mixture.
     */
    for (r = 0; r < vq->origsize.r; r++) {
        for (c = 0, c2 = 0; c < vq->origsize.c; c++) {
            if (vq->map[r][c][0] < 0) {
                /* All ought to be < 0 */
                for (s = 1; s < vq->n_sv; s++) {
                    if (vq->map[r][c][s] >= 0)
                        E_FATAL("Partially undefined map[%d][%d]\n", r, c);
                }
            }
            else {
                if (c2 != c) {
                    for (s = 0; s < vq->n_sv; s++) {
                        if (vq->map[r][c][s] < 0)
                            E_FATAL("Partially undefined map[%d][%d]\n", r,
                                    c);
                        vq->map[r][c2][s] = vq->map[r][c][s];
                    }
                }
                c2++;
            }
        }

        if (g && (c2 != mgau_n_comp(g, r)))
            E_FATAL
                ("Mixture %d: #Valid components conflict: %d (SubVQ) vs %d (Original)\n",
                 r, c2, mgau_n_comp(g, r));

        /* Invalidate the remaining map entries, for good measure */
        for (; c2 < vq->origsize.c; c2++) {
            for (s = 0; s < vq->n_sv; s++)
                vq->map[r][c2][s] = -1;
        }
    }
}


/*
 * At this point, a map entries is an index within the subvector; i.e., map[r][c][s] points to
 * a subvq codeword within subvector s.  In evaluating a complete density using it subvq
 * component scores, these maps are used, with 2 lookups for each sub-vector.  To reduce the
 * lookups, linearize the map entries by viewing the subvq scores as a linear array rather than
 * a 2-D array.
 */
static void
subvq_map_linearize(subvq_t * vq)
{
    int32 r, c, s;

    for (r = 0; r < vq->origsize.r; r++) {
        for (c = 0; (c < vq->origsize.c) && (vq->map[r][c][0] >= 0); c++) {
            for (s = 0; s < vq->n_sv; s++)
                vq->map[r][c][s] = (s * vq->vqsize) + vq->map[r][c][s];
        }
    }
}

subvq_t *
subvq_init(const char *file, float64 varfloor, int32 max_sv, mgau_model_t * g, cmd_ln_t *config, logmath_t * logmath)
{
    FILE *fp;
    char line[16384];
    int32 n_sv;                 /* #Subvectors in file, as opposed to that used */
    int32 s, k, l, n, r, c;
    char *strp;
    subvq_t *vq;

    E_INFO("Loading Mixture Gaussian sub-VQ file '%s' (vq_eval: %d)\n",
           file, cmd_ln_int32_r(config, "-vqeval"));
    if ((fp = fopen(file, "r")) == NULL) {
	    E_ERROR_SYSTEM("Failed to open subvq file");
	    return NULL;
    }

    vq = (subvq_t *) ckd_calloc(1, sizeof(subvq_t));
    vq->VQ_EVAL = cmd_ln_int32_r(config, "-vqeval");      /*Arthur : It nows work for arbitrary size of sub-vector */

    /* Read until "Sub-vectors" */
    for (;;) {
        if (fgets(line, sizeof(line), fp) == NULL)
            E_FATAL("Failed to read VQParam header\n");
        if (sscanf(line, "VQParam %d %d -> %d %d",
                   &(vq->origsize.r), &(vq->origsize.c), &(vq->n_sv),
                   &(vq->vqsize)) == 4)
            break;
    }

    if (g) {
        if ((g->n_mgau != vq->origsize.r)
            || (g->max_comp != vq->origsize.c))
            E_FATAL
                ("Model size conflict: %d x %d (SubVQ) vs %d x %d (Original)\n",
                 vq->origsize.r, vq->origsize.c, g->n_mgau, g->max_comp);
    }

    if (max_sv < 0)
        max_sv = vq->n_sv;
    if (max_sv < vq->n_sv)
        E_INFO("Using %d subvectors out of %d\n", max_sv, vq->n_sv);
    else if (max_sv > vq->n_sv) {
        E_WARN("#Subvectors specified(%d) > available(%d); using latter\n",
               max_sv, vq->n_sv);
        max_sv = vq->n_sv;
    }

    n_sv = vq->n_sv;
    vq->n_sv = max_sv;
    if (vq->n_sv < vq->VQ_EVAL) /* RAH, 5.9.01, sanity check to make sure VQ_EVAL isn't higher than the n_sv */
        vq->VQ_EVAL = vq->n_sv;
    vq->featdim = (int32 **) ckd_calloc(vq->n_sv, sizeof(int32 *));
    vq->gautbl =
        (vector_gautbl_t *) ckd_calloc(vq->n_sv, sizeof(vector_gautbl_t));
    vq->map =
        (int32 ***) ckd_calloc_3d(vq->origsize.r, vq->origsize.c, vq->n_sv,
                                  sizeof(int32));

    /* Read subvector sizes and feature dimension maps */
    for (s = 0; s < n_sv; s++) {
        if ((fgets(line, sizeof(line), fp) == NULL) ||
            (sscanf(line, "Subvector %d length %d%n", &k, &l, &n) != 2)
            || (k != s))
            E_FATAL("Error reading length(subvector %d)\n", s);

        if (s < vq->n_sv) {
            vq->gautbl[s].veclen = l;
            vq->featdim[s] =
                (int32 *) ckd_calloc(vq->gautbl[s].veclen, sizeof(int32));

            for (strp = line + n, c = 0; c < vq->gautbl[s].veclen; c++) {
                if (sscanf(strp, "%d%n", &(vq->featdim[s][c]), &n) != 1)
                    E_FATAL("Error reading subvector(%d).featdim(%d)\n", s,
                            c);
                strp += n;
            }

            vector_gautbl_alloc(&(vq->gautbl[s]), vq->vqsize,
                                vq->gautbl[s].veclen, logmath);
        }
    }

    /* Echo info for sanity check */
    E_INFO("Original #codebooks(states)/codewords: %d x %d\n",
           vq->origsize.r, vq->origsize.c);
    E_INFO("Subvectors: %d, VQsize: %d\n", vq->n_sv, vq->vqsize);
    for (s = 0; s < vq->n_sv; s++) {
        E_INFO("SV %d feature dims(%d): ", s, vq->gautbl[s].veclen);
        for (c = 0; c < vq->gautbl[s].veclen; c++)
            E_INFOCONT(" %2d", vq->featdim[s][c]);
        E_INFOCONT("\n");
    }

    /* Read VQ codebooks and maps for each subvector */
    for (s = 0; s < n_sv; s++) {
        E_INFO("Reading subvq %d%s\n", s,
               (s < vq->n_sv) ? "" : " (not used)");

        E_INFO("Reading codebook\n");
        if ((fgets(line, sizeof(line), fp) == NULL) ||
            (sscanf(line, "Codebook %d", &k) != 1) || (k != s))
            E_FATAL("Error reading codebook header\n", s);

        for (r = 0; r < vq->vqsize; r++) {
            if (fgets(line, sizeof(line), fp) == NULL)
                E_FATAL("Error reading row(%d)\n", r);

            if (s >= vq->n_sv)
                continue;

            for (strp = line, c = 0; c < vq->gautbl[s].veclen; c++) {
                if (sscanf(strp, "%f %f%n",
                           &(vq->gautbl[s].mean[r][c]),
                           &(vq->gautbl[s].var[r][c]), &k) != 2)
                    E_FATAL("Error reading row(%d) col(%d)\n", r, c);
                strp += k;
            }
        }

        E_INFO("Reading map\n");
        if ((fgets(line, sizeof(line), fp) == NULL) ||
            (sscanf(line, "Map %d", &k) != 1) || (k != s))
            E_FATAL("Error reading map header\n", s);

        for (r = 0; r < vq->origsize.r; r++) {
            if (fgets(line, sizeof(line), fp) == NULL)
                E_FATAL("Error reading row(%d)\n", r);

            if (s >= vq->n_sv)
                continue;

            for (strp = line, c = 0; c < vq->origsize.c; c++) {
                if (sscanf(strp, "%d%n", &(vq->map[r][c][s]), &k) != 1)
                    E_FATAL("Error reading row(%d) col(%d)\n", r, c);
                strp += k;
            }
        }

        fflush(stdout);
    }

    if ((fscanf(fp, "%s", line) != 1) || (strcmp(line, "End") != 0))
        E_FATAL("Error reading 'End' token\n");

    fclose(fp);

    subvq_maha_precomp(vq, varfloor);
    subvq_map_compact(vq, g);
    subvq_map_linearize(vq);

    n = 0;
    for (s = 0; s < n_sv; s++) {
        if (vq->gautbl[s].veclen > n)
            n = vq->gautbl[s].veclen;
    }
    assert(n > 0);
    vq->subvec = (float32 *) ckd_calloc(n, sizeof(float32));
    vq->vqdist =
        (int32 **) ckd_calloc_2d(vq->n_sv, vq->vqsize, sizeof(int32));
    vq->gauscore = (int32 *) ckd_calloc(vq->origsize.c, sizeof(int32));
    vq->mgau_sl = (int32 *) ckd_calloc(vq->origsize.c + 1, sizeof(int32));

    return vq;
}




/*
 * Based on previously computed subvq scores (Mahalanobis distances), determine the active
 * components in the given mixture (using the vq->map).
 * Return value: #Candidates in the returned shortlist.
 */
int32
subvq_mgau_shortlist(subvq_t * vq, int32 m,     /* In: Mixture index */
                     int32 n,   /* In: #Components in specified mixture */
                     int32 beam)
{                               /* In: Threshold to select active components */
    int32 *gauscore;
    int32 *map;
    int32 i, v, bv, th, nc;
    int32 *sl;
    int32 *vqdist;
    int32 sv_id;

    vqdist = vq->vqdist[0];     /* Since map is linearized for efficiency, must also
                                   look at vqdist[][] as vqdist[] */
    gauscore = vq->gauscore;
    sl = vq->mgau_sl;
    /* Special case when vq->n_sv == 3; for speed */
    map = vq->map[m][0];
    bv = MAX_NEG_INT32;

    switch (vq->n_sv) {
    case 3:
        for (i = 0; i < n; i++) {
            if (vq->VQ_EVAL == 1) {
                v = (int32) vqdist[*map];       /* If we are not weighting the cep values, we need to adjust the subvqbeam */
                map += 3;
            }
            else {
                /* RAH, we are ignoring the delta-delta, scoring the delta twice, strangely this works better than weighting the scores  */
                /* I believe it has to do with the beam widths */
                if (vq->VQ_EVAL == 2) {
                    v = vqdist[*(map++)];
                    v += 2 * vqdist[*map];      /* RAH Count delta twice, we can keep the same subvqbeam as vq_eval = 3 if we double the delta */
                    map += 2;
                }
                else {
                    v = vqdist[*(map++)];       /* Standard way */
                    v += vqdist[*(map++)];      /*  */
                    v += vqdist[*(map++)];      /*  */
                }
            }

            gauscore[i] = v;

            if (bv < v)
                bv = v;
        }
        break;
    case 2:
        for (i = 0; i < n; i++) {
            v = vqdist[*(map++)];
            v += vqdist[*(map++)];
            gauscore[i] = v;

            if (bv < v)
                bv = v;
        }
        break;
    case 1:
        for (i = 0; i < n; i++) {
            v = vqdist[*(map++)];
            gauscore[i] = v;

            if (bv < v)
                bv = v;
        }
        break;
    default:
        for (i = 0; i < n; i++) {
            v = 0;
            for (sv_id = 0; sv_id < vq->n_sv; sv_id++) {
                v += vqdist[*(map++)];
            }
            gauscore[i] = v;

            if (bv < v)
                bv = v;
        }
    }

    th = bv + beam;
    nc = 0;
    for (i = 0; i < n; i++) {
        if (gauscore[i] >= th)
            sl[nc++] = i;
    }
    sl[nc] = -1;

    return nc;
}


void
subvq_subvec_eval_logs3(subvq_t * vq, float32 * feat, int32 s, logmath_t * logmath)
{
    int32 i;
    int32 *featdim;

    /* Extract subvector from feat */
    featdim = vq->featdim[s];
    for (i = 0; i < vq->gautbl[s].veclen; i++)
        vq->subvec[i] = feat[featdim[i]];

    /* Evaluate distances between extracted subvector and corresponding codebook */
    vector_gautbl_eval_logs3(&(vq->gautbl[s]), 0, vq->vqsize, vq->subvec,
                             vq->vqdist[s], logmath);
}


void
subvq_gautbl_eval_logs3(subvq_t * vq, float32 * feat, logmath_t * logmath)
{
    int32 s, i;
    int32 *featdim;

    for (s = 0; s < vq->n_sv; s++) {
        /* Extract subvector from feat */
        featdim = vq->featdim[s];
        for (i = 0; i < vq->gautbl[s].veclen; i++)
            vq->subvec[i] = feat[featdim[i]];

        /* Evaluate distances between extracted subvector and corresponding codebook */
        /* RAH, only evaluate the first VQ_EVAL set of features */
        if (s < vq->VQ_EVAL)
            vector_gautbl_eval_logs3(&(vq->gautbl[s]), 0, vq->vqsize,
                                     vq->subvec, vq->vqdist[s], logmath);
    }
}


#if 0
int32
subvq_frame_eval(subvq_t * vq, mgau_model_t * g, int32 beam,
                 float32 * feat, uint8 * sen_active, int32 * senscr)
{
    int32 s;
    int32 best, ns, ng;

    best = MAX_NEG_INT32;
    ns = 0;
    ng = 0;
    if (!vq) {
        /* No subvq model, use the original (SLOW!!) */
        for (s = 0; s < g->n_mgau; s++) {
            if ((!sen_active) || sen_active[s]) {
                senscr[s] = mgau_eval(g, s, NULL, feat);
                if (best < senscr[s])
                    best = senscr[s];
                ns++;
                ng += mgau_n_comp(g, s);
            }
            else
                senscr[s] = S3_LOGPROB_ZERO;
        }
    }
    else {
        /* Evaluate subvq model for given feature vector */
        subvq_gautbl_eval_logs3(vq, feat);

        /* Find mixture component shortlists using subvq scores, and evaluate senones */
        for (s = 0; s < g->n_mgau; s++) {
            if ((!sen_active) || sen_active[s]) {
                ng += subvq_mgau_shortlist(vq, s, mgau_n_comp(g, s), beam);

                senscr[s] = mgau_eval(g, s, vq->mgau_sl, feat);
                if (best < senscr[s])
                    best = senscr[s];

                ns++;
            }
            else
                senscr[s] = S3_LOGPROB_ZERO;
        }
    }

    /* Normalize senone scores */
    for (s = 0; s < g->n_mgau; s++)
        senscr[s] -= best;

    g->frm_sen_eval = ns;
    g->frm_gau_eval = ng;

    return best;
}
#endif
int32
subvq_mgau_eval(mgau_model_t * g, subvq_t * vq, int32 m, int32 n,
                int32 * active)
{
    mgau_t *mgau;
    int32 *map;
    int32 i, v, sv_id;
    int32 c;
    int32 *vqdist;
    int32 score;
    int32 last_active;

    float64 f;
    f = 1.0 / log(logmath_get_base(g->logmath));

    vqdist = vq->vqdist[0];
    score = S3_LOGPROB_ZERO;
    mgau = &(g->mgau[m]);
    map = vq->map[m][0];

    if (!active) {
        for (i = 0; i < n; i++) {
            v = 0;
            for (sv_id = 0; sv_id < vq->n_sv; sv_id++) {
                v += vqdist[*(map++)];
            }
            score = logmath_add(g->logmath, score, v + mgau->mixw[i]);
        }
    }
    else {
        last_active = 0;
        for (i = 0; active[i] >= 0; i++) {
            c = active[i];
        }
        for (i = 0; active[i] >= 0; i++) {
            c = active[i];
            map += (c - last_active) * vq->n_sv;
            v = 0;
            for (sv_id = 0; sv_id < vq->n_sv; sv_id++) {
                v += vqdist[*(map++)];
            }

            last_active = c + 1;
            score = logmath_add(g->logmath, score, v + mgau->mixw[i]);
        }
    }

    if (score == S3_LOGPROB_ZERO) {
        E_INFO("Warning!! Score is S3_LOGPROB_ZERO\n");
    }

    return score;

}


/* RAH, free memory allocated by subvq_init() */
void
subvq_free(subvq_t * s)
{
    int i;



    if (s) {

        for (i = 0; i < s->n_sv; i++) {

            if (i < s->n_sv) {
                /*vector_gautbl_free(&(s->gautbl[i])); */

                if (s->gautbl[i].mean != NULL)
                    ckd_free_2d((void **) (s->gautbl[i].mean));

                if (s->gautbl[i].var != NULL)
                    ckd_free_2d((void **) (s->gautbl[i].var));


                if (s->featdim[i])
                    ckd_free((void *) s->featdim[i]);
            }
        }

        /* This is tricky because this part of memory is actually allocated only once in .
           subvq_maha_precomp.  So multiple free is actually wrong. */

        if (s->gautbl[0].lrd != NULL)
            ckd_free((void *) (s->gautbl[0].lrd));


        if (s->featdim)
            ckd_free((void *) s->featdim);

        /* Free gaussian table */
        if (s->gautbl)
            ckd_free((void *) s->gautbl);


        /* Free map */
        if (s->map)
            ckd_free_3d((void ***) s->map);

        if (s->subvec)
            ckd_free((void *) s->subvec);

        if (s->vqdist)
            ckd_free_2d((void **) s->vqdist);

        if (s->gauscore)
            ckd_free((void *) s->gauscore);

        if (s->mgau_sl)
            ckd_free((void *) s->mgau_sl);


        ckd_free((void *) s);


    }
}
