/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * ms_mgau.c -- Essentially a wrapper that wrap up gauden and
 * senone. It supports multi-stream. 
 *
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * HISTORY
 * $Log$
 * Revision 1.2  2006/02/22  16:56:01  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Added ms_mgau.[ch] into the trunk. It is a wrapper of ms_gauden and ms_senone
 * 
 * Revision 1.1.2.4  2005/09/25 18:55:19  arthchan2003
 * Added a flag to turn on and off precomputation.
 *
 * Revision 1.1.2.3  2005/08/03 18:53:44  dhdfu
 * Add memory deallocation functions.  Also move all the initialization
 * of ms_mgau_model_t into ms_mgau_init (duh!), which entails removing it
 * from decode_anytopo and friends.
 *
 * Revision 1.1.2.2  2005/08/02 21:05:38  arthchan2003
 * 1, Added dist and mgau_active as intermediate variable for computation. 2, Added ms_cont_mgau_frame_eval, which is a multi stream version of GMM computation mainly s3.0 family of tools. 3, Fixed dox-doc.
 *
 * Revision 1.1.2.1  2005/07/20 19:37:09  arthchan2003
 * Added a multi-stream cont_mgau (ms_mgau) which is a wrapper of both gauden and senone.  Add ms_mgau_init and model_set_mllr.  This allow eliminating 600 lines of code in decode_anytopo/align/allphone.
 *
 *
 *
 */

#include <string.h>

#include <sphinxbase/cmd_ln.h>

#include <ms_mgau.h>
#include <ms_mllr.h>
#include <cb2mllr_io.h>

/* Wrong place to put it */
int32
model_set_mllr(ms_mgau_model_t * msg, const char *mllrfile,
	       const char *cb2mllrfile, feat_t * fcb, mdef_t * mdef,
	       cmd_ln_t *config)
{
    float32 ****A, ***B, ***H;
    int32 *cb2mllr;
    int32 gid, sid, nclass;
    uint8 *mgau_xform;

    gauden_mean_reload(msg->g, cmd_ln_str_r(config, "-mean"));
    gauden_var_reload(msg->g, cmd_ln_str_r(config, "-var"));

    if (ms_mllr_read_regmat(mllrfile, &A, &B, &H,
                            fcb->stream_len, feat_n_stream(fcb),
                            &nclass) < 0)
        E_FATAL("ms_mllr_read_regmat failed\n");

    if (cb2mllrfile && strcmp(cb2mllrfile, ".1cls.") != 0) {
        uint32 ncb, nmllr;

        cb2mllr_read(cb2mllrfile, &cb2mllr, &ncb, &nmllr);
        if (nmllr != nclass)
            E_FATAL
                ("Number of classes in cb2mllr does not match mllr (%d != %d)\n",
                 ncb, nclass);
        if (ncb != msg->s->n_sen)
            E_FATAL
                ("Number of senones in cb2mllr does not match mdef (%d != %d)\n",
                 ncb, msg->s->n_sen);
    }
    else
        cb2mllr = NULL;


    mgau_xform = (uint8 *) ckd_calloc(msg->g->n_mgau, sizeof(uint8));

    /* Transform each non-CI mixture Gaussian */
    for (sid = 0; sid < msg->s->n_sen; sid++) {
        int32 class = 0;

        if (cb2mllr)
            class = cb2mllr[sid];
        if (class == -1)
            continue;

        if (mdef->cd2cisen[sid] != sid) {       /* Otherwise it's a CI senone */
            gid = msg->s->mgau[sid];
            if (!mgau_xform[gid]) {
                ms_mllr_norm_mgau(msg->g->mean[gid],
				  msg->g->var[gid],
				  msg->g->n_density,
				  A, B, H,
				  fcb->stream_len, feat_n_stream(fcb),
                                  class);
                mgau_xform[gid] = 1;
            }
        }
    }

    gauden_dist_precompute(msg->g, cmd_ln_float32_r(config, "-varfloor"));
    ckd_free(mgau_xform);

    ms_mllr_free_regmat(A, B, H, feat_n_stream(fcb));
    ckd_free(cb2mllr);

    return S3_SUCCESS;
}

ms_mgau_model_t *
ms_mgau_init(const char *meanfile,
             const char *varfile, float64 varfloor,
             const char *mixwfile, float64 mixwfloor,
             int32 precomp, const char *senmgau, const char *lambdafile,
	     int32 _topn, logmath_t *logmath, mdef_t *mdef)
{
    /* Codebooks */
    int32 i;
    ms_mgau_model_t *msg;
    gauden_t *g;
    senone_t *s;
    mgau2sen_t *m2s;

    msg = (ms_mgau_model_t *) ckd_calloc(1, sizeof(ms_mgau_model_t));


    msg->g = NULL;
    msg->s = NULL;
    msg->i = NULL;

    msg->g = gauden_init(meanfile, varfile, varfloor, precomp, logmath);

    msg->s = senone_init(mixwfile, senmgau, mixwfloor, logmath, mdef);

    g = ms_mgau_gauden(msg);
    s = ms_mgau_senone(msg);

    /* Verify senone parameters against gauden parameters */
    if (s->n_feat != g->n_feat)
        E_FATAL("#Feature mismatch: gauden= %d, senone= %d\n", g->n_feat,
                s->n_feat);
    if (s->n_cw != g->n_density)
        E_FATAL("#Densities mismatch: gauden= %d, senone= %d\n",
                g->n_density, s->n_cw);
    if (s->n_gauden > g->n_mgau)
        E_FATAL("Senones need more codebooks (%d) than present (%d)\n",
                s->n_gauden, g->n_mgau);
    if (s->n_gauden < g->n_mgau)
        E_ERROR("Senones use fewer codebooks (%d) than present (%d)\n",
                s->n_gauden, g->n_mgau);
    /* Initialize mapping from mixture Gaussian to senones */
    msg->mgau2sen =
        (mgau2sen_t **) ckd_calloc(g->n_mgau, sizeof(mgau2sen_t *));
    for (i = 0; i < s->n_sen; i++) {
        m2s = (mgau2sen_t *) ckd_calloc(1, sizeof(mgau2sen_t));
        m2s->sen = i;
        m2s->next = msg->mgau2sen[s->mgau[i]];
        msg->mgau2sen[s->mgau[i]] = m2s;
    }

    /* CD/CI senone interpolation weights file, if present */
    if (lambdafile != NULL) {
        msg->i = interp_init(lambdafile, logmath);
        /* Verify interpolation weights size with senones */
        if (msg->i->n_sen != s->n_sen)
            E_FATAL("Interpolation file has %d weights; but #senone= %d\n",
                    msg->i->n_sen, s->n_sen);
    }
    else
        msg->i = NULL;


    msg->topn = _topn;
    E_INFO("The value of topn: %d\n", msg->topn);
    if (msg->topn == 0 || msg->topn > msg->g->n_density) {
        E_WARN
            ("-topn argument (%d) invalid or > #density codewords (%d); set to latter\n",
             msg->topn, msg->g->n_density);
        msg->topn = msg->g->n_density;
    }

    msg->dist = (gauden_dist_t ***)
        ckd_calloc_3d(g->n_mgau, g->n_feat, msg->topn,
                      sizeof(gauden_dist_t));
    msg->mgau_active = ckd_calloc(g->n_mgau, sizeof(int8));

    return msg;
}

void
ms_mgau_free(ms_mgau_model_t * msg)
{
    int32 i;
    gauden_t *g;

    if (msg == NULL)
        return;

    g = ms_mgau_gauden(msg);
    for (i = 0; i < g->n_mgau; ++i) {
        mgau2sen_t *m2s = msg->mgau2sen[i];
        while (m2s) {
            mgau2sen_t *tmp = m2s->next;
            ckd_free(m2s);
            m2s = tmp;
        }
    }
    ckd_free(msg->mgau2sen);
    gauden_free(msg->g);
    senone_free(msg->s);
    ckd_free_3d((void *) msg->dist);
    ckd_free(msg->mgau_active);
    ckd_free(msg);
}

int32
ms_cont_mgau_frame_eval(ascr_t * ascr,
                        ms_mgau_model_t * msg,
                        mdef_t * mdef, float32 ** feat, int32 frame)
{
    int32 gid;
    int32 s;
    int32 topn;
    int32 best;
    gauden_t *g;
    senone_t *sen;
    interp_t *interp;

    topn = ms_mgau_topn(msg);
    g = ms_mgau_gauden(msg);
    sen = ms_mgau_senone(msg);
    interp = ms_mgau_interp(msg);

    /*
     * Evaluate gaussian density codebooks and senone scores for input codeword.
     * Evaluate only active codebooks and senones.
     */

    if (interp) {
        for (s = 0; s < mdef->n_ci_sen; s++)
            ascr->sen_active[s] = 1;
    }

    /* Flag all active mixture-gaussian codebooks */

    for (gid = 0; gid < g->n_mgau; gid++)
        msg->mgau_active[gid] = 0;

    for (s = 0; s < ascr->n_sen; s++) {
        if (ascr->sen_active[s]) {
            msg->mgau_active[sen->mgau[s]] = 1;
        }
    }

    /* Compute topn gaussian density values (for active codebooks) */
    for (gid = 0; gid < g->n_mgau; gid++) {
        if (msg->mgau_active[gid])
            gauden_dist(g, gid, topn, feat, msg->dist[gid]);
    }

    if (interp) {
        for (s = 0; s < ascr->n_sen; s++) {
            if (ascr->sen_active[s]) {
                if (s >= mdef->n_ci_sen) {
                    interp_cd_ci(interp, ascr->senscr, s,
                                 mdef->cd2cisen[s]);
                }
            }
        }
    }

    best = (int32) 0x80000000;
    for (s = 0; s < ascr->n_sen; s++) {
        if (ascr->sen_active[s]) {
            ascr->senscr[s] =
                senone_eval(sen, s, msg->dist[sen->mgau[s]], topn);
            if (best < ascr->senscr[s])
                best = ascr->senscr[s];
        }
    }


    /* Normalize senone scores (interpolation above can only lower best score) */
    for (s = 0; s < ascr->n_sen; s++) {
        if (ascr->sen_active[s])
            ascr->senscr[s] -= best;
    }

    return best;
}
