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
 * approx_cont_mgau.c
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2003 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.17  2006/03/17  23:35:42  egouvea
 * mdef_is_cisenone failed if only ci phones were used. Changed condition to return 0 if senid is higher than n_sen, satisfying the loops in approx_cont_mgau
 * 
 * Revision 1.16  2006/02/22 14:52:07  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Changed a->sen to a->senscr, it makes applications in the new architecture better.
 *
 * Revision 1.15.4.2  2005/09/25 18:51:50  arthchan2003
 * Add a FIXME in approx_cont_mgau.c. Yes, sorting is too slow.
 *
 * Revision 1.15.4.1  2005/08/02 21:03:02  arthchan2003
 * Changes sen to senscr, it confuses some applications.
 *
 * Revision 1.15  2005/06/21 18:05:12  arthchan2003
 * Log. approx_cont_mgau_frame_eval has interface's changes. ci_senscr,
 * best_score is now wrapped up ascr_t. approx_cont_mgau_ci_eval is now
 * taking care of maxing ci senone score for a frame.
 *
 * Revision 1.5  2005/04/20 03:30:57  archan
 * Part of refactoring: move best scores inside approx_cont_mgau.h
 *
 * Revision 1.4  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 23-Jan-2004 Arthur Chan (archan@cs.cmu.edu)
 *             started
 */

#include "approx_cont_mgau.h"
#include <stdlib.h>



#define DEBUG_GSCORE 1

/** \file approx_cont_mgau.c
 *  \brief Implementation detail of approx_cont_mgau
 *  \warning You need to have some knowledge in fast GMM computation in order to modify this function. 
 */

/** Decide whether this frame should be skip or not. 
    @return 1 if it is skipped, 0 if it is not.
 */

static int32
approx_isskip(int32 frame,       /**< In: The frame index */
              fast_gmm_t * fg,       /**< In: The fast GMM computation structure */
              int32 best_cid       /**< In: best code book index. Obtained from Gaussian Selector. Rarely used. */
    )
{

    int32 ds_ratio;
    int32 cond_ds;
    int32 dist_ds;
    int32 isSameBestIdx;
    int32 *skip_count;



    ds_ratio = fg->downs->ds_ratio;
    cond_ds = fg->downs->cond_ds;
    dist_ds = fg->downs->dist_ds;

    isSameBestIdx = (fg->gaus->rec_bstcid == best_cid);
    skip_count = &(fg->downs->skip_count);

    assert(fg->downs->ds_ratio != 0);
    assert(!(cond_ds > 0 && dist_ds > 0));

    /*Consider cond_ds first if specified */
    if (cond_ds > 0) {
        if (isSameBestIdx) {
            if (*skip_count < ds_ratio - 1) {
                ++*skip_count;
                return 1;
            }
            else {
                *skip_count = 0;
                return 0;
            }
        }
        else
            return 0;
    }

    if (dist_ds > 0) {

    }

    /*Consider the effect of ds_ratio */
    if (frame % ds_ratio == 0)
        return 0;
    else
        return 1;
}




/** 

    Update the senone score given index, return the number of
    gaussians compute, This took care of Gaussian level of
    optimization. This will called Feature Level Optimization routine

   Gaussian Level:

   ^^^^^^^^^^^^^^^

   Shortlist of Gaussians was determined using Gaussian-Selection
   (Bochierri 93) in gs_mgau_shortlist or Sub-VQ-based Gaussian
   Selection (Ravi 98) in subvq_mgau_shortlist. Note that the term
   "shortlist" was also used in (P. Douglas 99) which is basically are
   variant of (Bochierri 93) with a clever scheme which resolves the
   back-off problem.  We have plans to further enhance schemes of
   Gaussian Selection by combining them using machine learning
   techniques.

   Feature Component Level:

   ^^^^^^^^^^^^^^^^^^^^^^^^

   SVQ is used for feature level optimization only if svq4svq is set
   to 1.  This use the sum of sub-vector scores will be used as the
   gaussian scores.

   Safe Guarding abnomal scores:

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   We discover that even using our Gaussian Selection routine, the
   code can break because of some gaussian score is extremely low, we
   tried to detect them and avoid them by asserting the score and
   checked them at the end of a routines. 
   
   @see approx_cont_mgau_frame_eval
   
*/
static int32
approx_mgau_eval(gs_t * gs,       /**< In: The Gaussian Selector. If Null, Gaussian Selection is not used.*/
                 subvq_t * svq,       /**< In: Sub VQ map, If Null, SVQ is not used.*/
                 mgau_model_t * g,       /**< In: The set of senones */
                 fast_gmm_t * fastgmm,        /**< In: The structure fast GMM computation */
                 int32 s,        /**< In: senone index*/
                 int32 * senscr,       /**< Out: The array of senone scores */
                 float32 * feat,       /**< In: feature vector */
                 int32 best_cid,        /**< In: The best codebok index used in Gaussian Selector*/
                 int32 svq_beam,        /**< In: Beam for Sub-vector quantizor */
                 int32 fr        /**< In: the frame number in question */
    )
{
    int32 ng = 0;
    int32 *mgau_sl;

#if _DEBUG_GSCORE_
    int32 i;
#endif

    if (gs && fastgmm->gs4gs) {
        /* If GS is used, make sure best_cid > 0 */
        assert(best_cid > 0);
        ng = gs_mgau_shortlist(gs, s, mgau_n_comp(g, s), feat, best_cid);
        mgau_sl = gs->mgau_sl;
    }
    else if (svq) {
        ng = subvq_mgau_shortlist(svq, s, mgau_n_comp(g, s), svq_beam);
        mgau_sl = svq->mgau_sl;
    }
    else {
        ng = mgau_n_comp(g, s);
        mgau_sl = NULL;
    }

#if _DEBUG_GSCORE_
    for (i = 0; i < mgau_veclen(g); i++) {
        fprintf(stderr, "%f ", feat[i]);
    }
    fprintf(stderr, "\n");
    fflush(stderr);
    E_INFO("Full computation: Idx %d using subvq, Senscr %d ng %d\n", s,
           senscr[s], ng);
    senscr[s] = mgau_eval(g, s, NULL, feat, fr, 1);
    E_INFO("Full computationIdx %d using normal, Senscr %d ng %d\n", s,
           senscr[s], ng);
    senscr[s] = subvq_mgau_eval(g, svq, s, mgau_n_comp(g, s), mgau_sl);
    E_INFO("Partial Computation: Idx %d using subvq, Senscr %d ng %d\n", s,
           senscr[s], ng);
    senscr[s] = mgau_eval(g, s, mgau_sl, feat, fr, 1);
    E_INFO("Partial Computation: Idx %d using normal, Senscr %d ng %d\n",
           s, senscr[s], ng)
#endif
        /* This safe guard the algorithmic error of other 3rd party converter */
        if (ng == 0) {

#if _DEBUG_GSCORE_
        E_INFO
            ("short list has 0 element, turn to compute all, please check the Gaussian Selection algorithm\n");
#endif

        mgau_sl = NULL;
        ng = mgau_n_comp(g, s);
    }

    if (svq && fastgmm->svq4svq)
        senscr[s] = subvq_mgau_eval(g, svq, s, mgau_n_comp(g, s), mgau_sl);
    else
        senscr[s] = mgau_eval(g, s, mgau_sl, feat, fr, 1);

    /*This routine safe guard the computation such that no abnormality will occur */
    if (senscr[s] < S3_LOGPROB_ZERO + 100000) {
        /* a range of value which recomputation is necessary , 100000 is a
           magic number tuned by using the Communicator, WSK5k and ICSI
           tasks. It is a reasonable magic :-) */
        if (mgau_sl == NULL) {

#if _DEBUG_GSCORE_
            E_INFO
                ("WARNING!! Score is S3_LOGPROB_ZERO even computing full gaussians! %d\n",
                 s);
#endif

        }
        else {
            mgau_sl = NULL;
            ng += mgau_n_comp(g, s);
            if (svq && fastgmm->svq4svq)
                senscr[s] =
                    subvq_mgau_eval(g, svq, s, mgau_n_comp(g, s), NULL);
            else
                senscr[s] = mgau_eval(g, s, NULL, feat, fr, 1);

        }
    }

    return ng;
}

int32 *ci;

static int
intcmp(const void *v1, const void *v2)
{
    return (ci[*(int32 *) v2] - ci[*(int32 *) v1]);
}


/** This function compute the dynamic beam using histogram-based CI
    senone evaluation. This will probably be another paper on speed
    up.
    @see approx_cont_mgau_frame_eval
    @see approx_cont_mgau_ci_eval
  */

static int32
approx_compute_dyn_ci_pbeam(mdef_t * mdef,      /**< In: model definition */
                            fast_gmm_t * fastgmm,       /**< In: fast gmm computasion structure. */
                            mgau_model_t * g,      /**< In: Gaussian distribution */
                            int32 * ci_occ,       /**< In/Out: An array of occupancies of ci senones*/
                            uint8 * sen_active,      /**< In: An array of activeness of senones */
                            int32 * cache_ci_senscr,     /**< In: CD senone score, the user should precompute it using approx_cong_mgau_ci_eval*/
                            s3senid_t * cd2cisen      /** In: a mapping from CD senone to CI senone */
    )
{
    int s;
    int32 *idx;
    int32 pbest;
    int32 total;
    int32 is_ciphone;

    ci = cache_ci_senscr;

    idx = fastgmm->gmms->idx;

    for (s = 0; s < g->n_mgau; s++) {
        is_ciphone = mdef_is_cisenone(mdef, s);
        if (is_ciphone) {       /*Initialize the ci_occ in the fast_gmm_struct */
            ci_occ[s] = 0;
        }
        else {
            if (!sen_active || sen_active[s]) {
                ci_occ[cd2cisen[s]]++;
            }
        }
    }
    for (s = 0; s < mdef->n_ci_sen; s++)
        idx[s] = s;
    /* ARCHAN: we only have around 200 ci phones so it should be fine
       for sorting. How about Chinese then? Hmm. We'll think about that
       later...... */

    /* FIXME : sorting is too slow */
    qsort(idx, mdef->n_ci_sen, sizeof(int32), intcmp);

    total = 0;
    pbest = cache_ci_senscr[idx[0]];
    fastgmm->gmms->dyn_ci_pbeam = fastgmm->gmms->ci_pbeam;
    for (s = 0;
         s < mdef->n_ci_sen
         && cache_ci_senscr[idx[s]] > pbest + fastgmm->gmms->ci_pbeam;
         s++) {
        total += ci_occ[idx[s]];
        if (total > fastgmm->gmms->max_cd) {
            fastgmm->gmms->dyn_ci_pbeam = cache_ci_senscr[idx[s]] - pbest;
            break;
        }
    }
    /*  E_INFO("The dynamic beam %d\n",fastgmm->gmms->dyn_ci_pbeam); */
    return fastgmm->gmms->dyn_ci_pbeam;
}


/** In this function,
  1, It only compute the ci-phones score.
  2, The score is not normalize, this routine is supposed to be used before approx_cont_mgau_frame_eval,
  The best score is determined by the later function. 
*/

void
approx_cont_mgau_ci_eval(subvq_t * svq,
                         gs_t * gs,
                         mgau_model_t *g,
                         fast_gmm_t * fg,      /** In : The fast GMM structure */
                         mdef_t * mdef,      /** In : model definition */
                         float32 * feat,      /** In : the feature vector */
                         int32 * ci_senscr,      /** Input/Output : ci senone score, a one dimension array */
                         int32 * best_score,      /** Input/Output: the best score, a scalar */
                         int32 fr,       /** In : The frame number */
                         logmath_t * logmath
    )
{
    int32 s;
    s3senid_t *cd2cisen;
    int32 best_cid;
    int32 svq_beam;
    int32 n_cig;
    int32 n_cis;

    n_cis = 0;
    n_cig = 0;
    cd2cisen = mdef_cd2cisen(mdef);
    svq_beam = fg->gaus->subvqbeam;


    /*#ifdef APPROX_CONT_MGAU */
    /*Always turn on, so users can be the one who decide how fast/slow the recognizer can be */

    best_cid = -1;
#if 1
    if (gs)
        best_cid = gc_compute_closest_cw(gs, feat);
    if (svq)
        subvq_gautbl_eval_logs3(svq, feat, logmath);

    for (s = 0; mdef_is_cisenone(mdef, s); s++) {
        n_cig +=
            approx_mgau_eval(gs, svq, g, fg, s, ci_senscr, feat, best_cid,
                             svq_beam, fr);
        n_cis++;
    }
#else
    /*20050114 Not used */
    for (s = 0; mdef_is_cisenone(mdef, s); s++) {
        ci_senscr[s] = mgau_eval(g, s, NULL, feat, fr, 1);
        n_cig += mgau_n_comp(g, s);
        n_cis++;
    }

#endif

    *best_score = MAX_NEG_INT32;

    for (s = 0; mdef_is_cisenone(mdef, s); s++) {
        if (ci_senscr[s] > *best_score)
            *best_score = ci_senscr[s];
    }


    g->frm_ci_sen_eval = n_cis;
    g->frm_ci_gau_eval = n_cig;
}




int32
approx_cont_mgau_frame_eval(mdef_t * mdef,
                            subvq_t * svq,
                            gs_t * gs,
                            mgau_model_t * g,
                            fast_gmm_t * fastgmm,
                            ascr_t * a,
                            float32 * feat,
                            int32 frame,
                            int32 * cache_ci_senscr,
                            ptmr_t * tm_ovrhd,
                            logmath_t * logmath)
{
    int32 s;
    int32 best, ns, ng, n_cis, n_cig;
    int32 best_cid;
    int32 is_skip;
    int32 is_compute;
    int32 pbest;
    int32 is_ciphone;
    int32 svq_beam;
    int32 *ci_occ;
    s3senid_t *cd2cisen;

    int32 total;
    int32 dyn_ci_pbeam;
    float32 tighten_factor;

    int32 single_el_list[2];
    uint8 *sen_active;
    uint8 *rec_sen_active;
    int32 *senscr;

    best = MAX_NEG_INT32;
    pbest = MAX_NEG_INT32;
    ns = 0;
    n_cis = 0;
    n_cig = 0;
    ng = 0;
    total = 0;
    best_cid = -1;


    single_el_list[0] = -1;
    single_el_list[1] = -1;
    svq_beam = fastgmm->gaus->subvqbeam;

    ci_occ = fastgmm->gmms->ci_occu;
    cd2cisen = mdef_cd2cisen(mdef);

    sen_active = a->sen_active;
    rec_sen_active = a->rec_sen_active;
    senscr = a->senscr;

    ptmr_start(tm_ovrhd);
    if (gs)
        best_cid = gc_compute_closest_cw(gs, feat);
    if (svq)
        subvq_gautbl_eval_logs3(svq, feat, logmath);

    if (fastgmm->gmms->max_cd < mdef->n_sen - mdef->n_ci_sen)
        dyn_ci_pbeam = approx_compute_dyn_ci_pbeam(mdef, fastgmm, g,
                                                   ci_occ, sen_active,
                                                   cache_ci_senscr,
                                                   cd2cisen);
    else
        dyn_ci_pbeam = fastgmm->gmms->ci_pbeam;

    tighten_factor = fastgmm->gmms->tighten_factor;

    ptmr_stop(tm_ovrhd);

    is_skip = approx_isskip(frame, fastgmm, best_cid);
    fastgmm->gaus->rec_bstcid = best_cid;

    /* Use the original */

    /* If the frame is "skipped", tighten the beam. */
    if (is_skip) {
        dyn_ci_pbeam = (int32) ((float32) dyn_ci_pbeam * tighten_factor);
    }

    for (s = 0; s < g->n_mgau; s++) {
        is_compute = !sen_active || sen_active[s];
        is_ciphone = mdef_is_cisenone(mdef, s);

#if 0
        if (sen_active[s])
            E_INFO
                ("Sen active %d, rec_sen_active %d, sen id %d cisen id %d, best index %d, update time %d\n",
                 sen_active[s], rec_sen_active[s], s, cd2cisen[s],
                 g->mgau[s].bstidx, g->mgau[s].updatetime);
#endif

        /* Compute the score of the CI phone even if it is not active. */
        if (is_ciphone) {
            /*Just copied from the cache, we just do accouting here */
            /*E_INFO("At senone %d, CI phoneme score %d \n",s,cache_ci_senscr[s]); */
            senscr[s] = cache_ci_senscr[s];
            if (pbest < senscr[s])
                pbest = senscr[s];
            if (best < senscr[s])
                best = senscr[s];
            sen_active[s] = 1;
        }
        else {
            if (is_compute) {
                if ((senscr[cd2cisen[s]] >= pbest + dyn_ci_pbeam)) {
                    ng +=
                        approx_mgau_eval(gs, svq, g, fastgmm, s, senscr,
                                         feat, best_cid, svq_beam, frame);
                    ns++;
                }
                else {

                    /* 3.6 logic: 
                       Whenever it is possible, CD senone score is backed off to the
                       score computed by the best matching index. Otherwise, CI
                       scores will be used.
                     */
                    if (g->mgau[s].bstidx == NO_BSTIDX ||       /* If the gaussian was not computed before. or */
                        g->mgau[s].updatetime != frame - 1      /* It the gaussian was not updated in last frame */
                        ) {
                        /*Previous frames, senone s is not computed. 
                           or the best index is not trusted. 
                           Use CI score in these cases. */
#ifdef GAUDEBUG
                        E_INFO
                            ("USE CI SENONE SCORE at senone %d time %d\n",
                             s, frame);
#endif

                        senscr[s] = senscr[cd2cisen[s]];

                    }
                    else {
                        /*Don't change the bstidx and updatetime */
                        single_el_list[0] = g->mgau[s].bstidx;

                        /*ARCHAN: Please don't rewrite this two lines. I want to make a contrast between the two situations. */
                        if (is_skip)
                            senscr[s] = mgau_eval(g, s, single_el_list, feat, frame, 1);        /*Update the best idx, such that next frames can use it */
                        else
                            senscr[s] = mgau_eval(g, s, single_el_list, feat, frame, 0);        /*Not update the best index in a Gaussian */

                        ng++;   /* But don't increase the number of senone compute. It doesn't count. */

#ifdef GAUDEBUG
                        E_INFO
                            ("RECOMPUTE for senone %d USING BEST INDEX %d time %d, single Gauss score %d, ci score %d, last ci score %d. \n",
                             s, g->mgau[s].bstidx, frame, senscr[s],
                             senscr[cd2cisen[s]], g->mgau[s].bstscr);
#endif

                    }
                }
                if (best < senscr[s])
                    best = senscr[s];
            }
        }
        /*Make a copy to the most recent active list */
        rec_sen_active[s] = sen_active[s];
    }

    for (s = 0; s < g->n_mgau; s++) {
        if (sen_active[s])
            senscr[s] -= best;
    }


    /*Don't delete this line, it is very useful in analysing the performance */

#if APPROX_ANALYSE
    E_INFO("time: %d , cisen %d, sen: %d, gau: %d\n", frame, n_cis, ns,
           ng);
#endif

    g->frm_sen_eval = ns;
    g->frm_gau_eval = ng;
    ci_occ = NULL;
    return best;


}
