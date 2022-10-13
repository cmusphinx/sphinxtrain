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
 * fast_algo_struct.c -- Various forms of pruning beam
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
 * Revision 1.7  2006/02/22  16:39:43  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Initialize beam->n_ciphone properly, 2, use ckd_free instead of free, use float64 for subvqbeam and cipbeam.  3, Add a proper free function for fast_gmm_free
 * 
 * Revision 1.5.4.4  2005/10/17 04:43:57  arthchan2003
 * Free fast_gmm_t.
 *
 * Revision 1.5.4.3  2005/09/25 19:35:26  arthchan2003
 * Change realloc to calloc. Could be permanent if we found that there is no need to reallocate the array.
 *
 * Revision 1.5.4.2  2005/07/24 01:29:54  arthchan2003
 * Set #ci phone.
 *
 * Revision 1.5.4.1  2005/07/03 22:53:15  arthchan2003
 * 1, Changed free to ckd_free, 2, Join from HEAD, using float64 instead of float32.
 *
 * Revision 1.6  2005/06/30 13:08:44  egouvea
 * Beams in linear scale have to be float64, since they can be easily defined as < 1e-40
 *
 * Revision 1.5  2005/06/21 18:26:38  arthchan2003
 * Log. fast_algo_struct.c go through major changes in the gentle
 * refactoring process. It is the location of several wrapper structures
 * that control fast search.  That includes beam_t for storing beams and
 * scores. pl_t for storing structure for phoneme lookahead, histprune_t
 * for storing structures for histogram pruning. Lastly
 * fast_algo_struct_t, for storing structures for fast GMM
 * computation.
 *
 * Log. General Remark All of them now has consistent inteface, _init,
 * _report and _free.  They are respectively used for allocation,
 * reporting and deallocation of the routine. Doxygen documentation are
 * fixed for all structures.
 *
 * Log. Individual changes; beam_t start to own bestscore, bestwordscore,
 * wordbestscores, wordbestexits. They were owned by kb_t. histprune_t
 * now wrapped up maxwpf, maxhmmpdf, maxhistpf and
 * hmm_hist_binsize. Currently, the beam size determination routine is
 * controlled by search implementation modules.  It is done because
 * wrapping that operation up means we may need to introduce a bridge
 * between beam_t and histprune_t.  pl_t is now owning heuristic type,
 * the phoneme lookahead beam size. It also wrapped up phoneme heuristic
 * computation.
 *
 * Revision 1.6  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.5  2005/04/20 03:33:54  archan
 * Remove pl_win and pl_win_strt, Now consider them as the parameters of the search abstraction in srch.c
 *
 * Revision 1.4  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 09-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "fast_algo_struct.h"
#include "logs3.h"


beam_t *
beam_init(float64 hmm, float64 ptr, float64 wd, float64 wdend,
          int32 ptranskip, int32 n_ciphone, logmath_t *logmath)
{
    beam_t *beam;

    beam = (beam_t *) ckd_calloc(1, sizeof(beam_t));

    beam->hmm = logs3(logmath, hmm);
    beam->ptrans = logs3(logmath, ptr);
    beam->word = logs3(logmath, wd);
    beam->wordend = logs3(logmath, wdend);
    beam->ptranskip = ptranskip;
    beam->bestscore = MAX_NEG_INT32;
    beam->bestwordscore = MAX_NEG_INT32;
    beam->n_ciphone = n_ciphone;

    beam->wordbestscores = (int32 *) ckd_calloc(n_ciphone, sizeof(int32));
    beam->wordbestexits = (int32 *) ckd_calloc(n_ciphone, sizeof(int32));

    return beam;
}

void
beam_report(beam_t * b)
{
    E_INFO_NOFN("Initialization of beam_t, report:\n");
    E_INFO_NOFN("Parameters used in Beam Pruning of Viterbi Search:\n");
    E_INFO_NOFN("Beam=%d\n", b->hmm);
    E_INFO_NOFN("PBeam=%d\n", b->ptrans);
    E_INFO_NOFN("WBeam=%d (Skip=%d)\n", b->word, b->ptranskip);
    E_INFO_NOFN("WEndBeam=%d \n", b->wordend);
    E_INFO_NOFN("No of CI Phone assumed=%d \n", b->n_ciphone);
    E_INFO_NOFN("\n");
}

void
beam_free(beam_t * b)
{
    if (b) {
        if (b->wordbestscores) {
            free(b->wordbestscores);
        }
        if (b->wordbestexits) {
            free(b->wordbestexits);
        }
        free(b);
    }
}

pl_t *
pl_init(int32 pheurtype, float64 pl_beam, int32 n_ciphone, logmath_t *logmath)
{
    pl_t *pl;
    pl = (pl_t *) ckd_calloc(1, sizeof(pl_t));

    pl->pheurtype = pheurtype;
    pl->pl_beam = logs3(logmath, pl_beam);
    pl->n_ciphone = n_ciphone;
    pl->phn_heur_list = (int32 *) ckd_calloc(pl->n_ciphone, sizeof(int32));

    return pl;
}

void
pl_free(pl_t * pl)
{
    if (pl) {
        if (pl->phn_heur_list)
            ckd_free((void *) pl->phn_heur_list);

        ckd_free((void *) pl);
    }
}

void
pl_report(pl_t * pl)
{

    E_INFO_NOFN("Initialization of pl_t, report:\n");
    E_INFO_NOFN("Parameters used in phoneme lookahead:\n");
    E_INFO_NOFN("Phoneme look-ahead        type = %d\n", pl->pheurtype);
    E_INFO_NOFN("Phoneme look-ahead beam   size = %d\n", pl->pl_beam);
    E_INFO_NOFN("No of CI Phones assumed=%d \n", pl->n_ciphone);
    E_INFO_NOFN("\n");
}

/* Determine which set of phonemes should be active in next stage
   using the lookahead information*/
/* Notice that this loop can be further optimized by implementing
   it incrementally*/
/* ARCHAN and JSHERWAN Eventually, this is implemented as a function */
/* Notice that this loop can be further optimized by implementing it incrementally*/

#define _CHECKUNDERFLOW_ 1

static int32
NO_UFLOW_ADD(int32 a, int32 b)
{
    int32 c;
#ifdef _CHECKUNDERFLOW_
    c = a + b;
    c = (c > 0 && a < 0 && b < 0) ? MAX_NEG_INT32 : c;
#else
    c = a + b;
#endif
    return c;
}


void
pl_computePhnHeur(mdef_t * md, ascr_t * a, pl_t * pl, int32 heutype,
                  int32 win_strt, int32 win_efv)
{
    int32 nState;
    int32 i, j;
    int32 curPhn, curFrmPhnVar; /* variables for phoneme lookahead computation */
    int32 *ph_lst;

    nState = mdef_n_emit_state(md);
    ph_lst = pl->phn_heur_list;

    /* Initializing all the phoneme heuristics for each phone to be 0 */
    for (j = 0; j == md->cd2cisen[j]; j++) {
        curPhn = md->sen2cimap[j];      /*Just to save a warning */
        ph_lst[curPhn] = 0;
    }

    /* 20040503: ARCHAN, the code can be reduced to 10 lines, it is so
       organized such that there is no overhead in checking the
       heuristic type in the inner loop.  
     */
    /* One trick we use is to use sen2cimap to check phoneme ending boundary */


    if (heutype == 1) {         /* Taking Max */
        for (i = win_strt; i < win_efv; i++) {
            curPhn = 0;
            curFrmPhnVar = MAX_NEG_INT32;
            for (j = 0; j == md->cd2cisen[j]; j++) {
                if (curFrmPhnVar < a->cache_ci_senscr[i][j])
                    curFrmPhnVar = a->cache_ci_senscr[i][j];

                curPhn = md->sen2cimap[j];
                /* Update at the phone_end boundary */
                if (curPhn != md->sen2cimap[j + 1]) {
                    ph_lst[curPhn] =
                        NO_UFLOW_ADD(ph_lst[curPhn], curFrmPhnVar);
                    curFrmPhnVar = MAX_NEG_INT32;
                }
            }
        }
    }
    else if (heutype == 2) {
        for (i = win_strt; i < win_efv; i++) {
            curPhn = 0;
            curFrmPhnVar = MAX_NEG_INT32;
            for (j = 0; j == md->cd2cisen[j]; j++) {
                curFrmPhnVar =
                    NO_UFLOW_ADD(a->cache_ci_senscr[i][j], curFrmPhnVar);
                curPhn = md->sen2cimap[j];

                /* Update at the phone_end boundary */
                if (curPhn != md->sen2cimap[j + 1]) {
                    curFrmPhnVar /= nState;     /* ARCHAN: I hate to do division ! */
                    ph_lst[curPhn] =
                        NO_UFLOW_ADD(ph_lst[curPhn], curFrmPhnVar);
                    curFrmPhnVar = MAX_NEG_INT32;
                }
            }
        }
    }
    else if (heutype == 3) {
        for (i = win_strt; i < win_efv; i++) {
            curPhn = 0;
            curFrmPhnVar = MAX_NEG_INT32;
            for (j = 0; j == md->cd2cisen[j]; j++) {
                if (curPhn == 0 || curPhn != md->sen2cimap[j - 1])      /* dangerous hack! */
                    ph_lst[curPhn] =
                        NO_UFLOW_ADD(ph_lst[curPhn],
                                     a->cache_ci_senscr[i][j]);

                curPhn = md->sen2cimap[j];

                if (curFrmPhnVar < a->cache_ci_senscr[i][j])
                    curFrmPhnVar = a->cache_ci_senscr[i][j];

                /* Update at the phone_end boundary */
                if (md->sen2cimap[j] != md->sen2cimap[j + 1]) {
                    ph_lst[curPhn] =
                        NO_UFLOW_ADD(ph_lst[curPhn], curFrmPhnVar);
                    curFrmPhnVar = MAX_NEG_INT32;
                }
            }
        }
    }

#if 0
    for (j = 0; j == md->cd2cisen[j]; j++) {
        curPhn = md->cd2cisen[j];
        E_INFO("phoneme heuristics scores at phn %d is %d\n", j,
               kb->phn_list[mdef->sen2cimap[j]]);
    }
#endif


}


histprune_t *
histprune_init(int32 maxhmm, int32 maxhist, int32 maxword,
               int32 hmmhistbinsize, int32 numNodes)
{
    histprune_t *h;
    int32 n;

    h = (histprune_t *) ckd_calloc(1, sizeof(histprune_t));
    h->maxwpf = maxword;
    h->maxhmmpf = maxhmm;
    h->maxhistpf = maxhist;
    h->hmm_hist_binsize = hmmhistbinsize;

    n = numNodes;
    n /= h->hmm_hist_binsize;

    h->hmm_hist_bins = n + 1;

    h->hmm_hist = (int32 *) ckd_calloc(h->hmm_hist_bins, sizeof(int32));


    return h;
}

void
histprune_zero_histbin(histprune_t * h)
{
    int32 *hmm_hist;
    int32 numhistbins;          /* Local version of number of histogram bins, don't expect it to change */
    int32 i;

    hmm_hist = h->hmm_hist;
    numhistbins = h->hmm_hist_bins;

    for (i = 0; i < numhistbins; i++)
        hmm_hist[i] = 0;

}


void
histprune_update_histbinsize(histprune_t * h,
                             int32 hmmhistbinsize, int32 numNodes)
{
    int32 n;
    h->hmm_hist_binsize = hmmhistbinsize;
    n = numNodes;
    n /= h->hmm_hist_binsize;

    h->hmm_hist_bins = n + 1;

    h->hmm_hist =
        (int32 *) ckd_realloc(h->hmm_hist,
                              h->hmm_hist_bins * sizeof(int32));
}

void
histprune_free(histprune_t * h)
{
    if (h != NULL) {
        if (h->hmm_hist != NULL) {
            ckd_free(h->hmm_hist);
        }
        free(h);
    }
}

void
histprune_showhistbin(histprune_t * hp, int32 nfr, char *uttid)
{
    int32 i, j, k;

    if (nfr == 0) {
        nfr = 1;
        E_WARN("Set number of frame to 1\n");
    }

    for (j = hp->hmm_hist_bins - 1; (j >= 0) && (hp->hmm_hist[j] == 0);
         --j);
    E_INFO("HMMHist[0..%d](%s):", j, uttid);
    for (i = 0, k = 0; i <= j; i++) {
        k += hp->hmm_hist[i];
        E_INFOCONT(" %d(%d)", hp->hmm_hist[i], (k * 100) / nfr);
    }
    E_INFOCONT("\n");
}


void
histprune_report(histprune_t * h)
{
    E_INFO_NOFN("Initialization of histprune_t, report:\n");
    E_INFO_NOFN("Parameters used in histogram pruning:\n");
    E_INFO_NOFN("Max.     HMM per frame=%d\n", h->maxhmmpf);
    E_INFO_NOFN("Max. history per frame=%d\n", h->maxhistpf);
    E_INFO_NOFN("Max.    word per frame=%d\n", h->maxwpf);
    E_INFO_NOFN("Size of histogram  bin=%d\n", h->hmm_hist_binsize);
    E_INFO_NOFN("No.  of histogram  bin=%d\n", h->hmm_hist_bins);
    E_INFO_NOFN("\n");
}

fast_gmm_t *
fast_gmm_init(int32 down_sampling_ratio,
              int32 mode_cond_ds,
              int32 mode_dist_ds,
              int32 isGS4GS,
              int32 isSVQ4SVQ,
              float64 subvqbeam,
              float64 cipbeam,
              float32 tighten_factor, int32 maxcd, int32 n_ci_sen,
              logmath_t *logmath)
{
    fast_gmm_t *fg;

    fg = (fast_gmm_t *) ckd_calloc(1, sizeof(fast_gmm_t));

    fg->rec_bst_senscr = 0;
    fg->last_feat = NULL;

    fg->gs4gs = isGS4GS;
    fg->svq4svq = isSVQ4SVQ;
    fg->downs = (downsampling_t *) ckd_calloc(1, sizeof(downsampling_t));
    fg->gmms = (gmm_select_t *) ckd_calloc(1, sizeof(gmm_select_t));
    fg->gaus = (gau_select_t *) ckd_calloc(1, sizeof(gau_select_t));

    fg->gmms->ci_pbeam = logs3(logmath, cipbeam);
    fg->gmms->tighten_factor = tighten_factor;
    if (fg->gmms->ci_pbeam < -10000000)
        E_INFO
            ("Virtually no CI phone beam is applied now. (ci_pbeam <-1000000)\n");
    fg->gmms->ci_occu = (int32 *) ckd_calloc(n_ci_sen, sizeof(int32));
    fg->gmms->idx = (int32 *) ckd_calloc(n_ci_sen, sizeof(int32));
    fg->gmms->max_cd = maxcd;

    fg->gaus->rec_bstcid = -1;

    fg->gaus->subvqbeam = logs3(logmath, subvqbeam);

    fg->downs->ds_ratio = down_sampling_ratio;
    fg->downs->cond_ds = mode_cond_ds;
    fg->downs->dist_ds = mode_dist_ds;
    fg->downs->skip_count = 0;

    if (fg->downs->cond_ds && fg->downs->dist_ds)
        E_FATAL("-cond_ds and -dist_ds cannot be specified together\n");



    return fg;
}

void
fast_gmm_free(fast_gmm_t * fg)
{
    if (fg) {
        if (fg->gmms->ci_occu)
            ckd_free(fg->gmms->ci_occu);
        if (fg->gmms->idx)
            ckd_free(fg->gmms->idx);

        if (fg->gmms)
            ckd_free(fg->gmms);

        if (fg->gaus)
            ckd_free(fg->gaus);

        if (fg->downs)
            ckd_free(fg->downs);

        ckd_free(fg);
    }
}


void
fast_gmm_report(fast_gmm_t * f)
{
    assert(f);
    E_INFO_NOFN("Initialization of fast_gmm_t, report:\n");
    E_INFO_NOFN("Parameters used in Fast GMM computation:\n");
    E_INFO_NOFN
        ("   Frame-level: Down Sampling Ratio %d, Conditional Down Sampling? %d, Distance-based Down Sampling? %d\n",
         f->downs->ds_ratio, f->downs->cond_ds, f->downs->dist_ds);
    E_INFO_NOFN("     GMM-level: CI phone beam %d. MAX CD %d\n",
                f->gmms->ci_pbeam, f->gmms->max_cd);
    E_INFO_NOFN
        ("Gaussian-level: GS map would be used for Gaussian Selection? =%d, SVQ would be used as Gaussian Score? =%d SubVQ Beam %d\n",
         f->gs4gs, f->svq4svq, f->gaus->subvqbeam);

    E_INFO_NOFN("\n");

}
