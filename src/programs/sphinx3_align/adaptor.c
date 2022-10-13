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
 * adaptor.c -- Wrapper for structures of adaptation. 
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
 * Revision 1.2  2006/02/22  14:48:13  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Added adapt_set_mllr (Moved from kb)
 * 
 * Revision 1.1.4.1  2005/09/18 01:12:31  arthchan2003
 * Add adapt_set_mllr which is responsible for single stream MLLR switching.
 *
 * Revision 1.1  2005/06/21 17:59:44  arthchan2003
 * Log: Implementation and Interface of adapt_am_t, a wrapper of
 * adaptation capabability of Sphinx 3.  It takes the responsibility from
 * kb_t to manage regA, regB and mllr_nclass.  Interfaces are not fully
 * completed. So "pointer" symtom code still appears in kb.c
 *
 * Revision 1.5  2005/06/19 19:41:21  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.4  2005/05/11 06:10:37  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 25-Mar-2005  Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First started. 
 */

#include <string.h>

#include "adaptor.h"
#include "mllr.h"
#include "cb2mllr_io.h"

#define ADAPT_FILE_NAME_LENGTH 1024

adapt_am_t *
adapt_am_init()
{
    adapt_am_t *ad = (adapt_am_t *) ckd_calloc(1, sizeof(adapt_am_t));

    ad->prevmllrfn =
        (char *) ckd_calloc(ADAPT_FILE_NAME_LENGTH, sizeof(char));
    ad->prevmllrfn[0] = '\0';

    return ad;
}

void
adapt_am_free(adapt_am_t * ad)
{
    if (ad) {
        if (ad->prevmllrfn) {
            ckd_free((void *) ad->prevmllrfn);
        }
        ckd_free((void *) ad);
    }
}

void
adapt_set_mllr(adapt_am_t * ad, mgau_model_t * g, const char *mllrfile,
	       const char *cb2mllrname, mdef_t * mdef, cmd_ln_t *config)
{
    int32 *cb2mllr;
    float32 varfloor;
    
    varfloor = cmd_ln_float32_r(config, "-varfloor");

    /* Reread the gaussian mean from the file again */
    E_INFO("Reloading mean\n");
    mgau_mean_reload(g, cmd_ln_str_r(config, "-mean"));

    /* Reread the gaussian variance from the file again */
    E_INFO("Reloading variance\n");
    mgau_var_reload(g, cmd_ln_str_r(config, "-var"));

    mgau_uninit_compact(g);     /* Delete uninitialized components */

    if (g->mgau[0].var && varfloor > 0.0)
        mgau_var_floor(g, varfloor);    /* Variance floor after above compaction */

#if MLLR_DEBUG
    /*This generates huge amount of information */
    /*    mgau_dump(g,1); */
#endif

    /* Read in the mllr matrix */
    mllr_read_regmat(mllrfile,
                     &(ad->regA),
                     &(ad->regB),
		     &(ad->regH),
		     &(ad->mllr_nclass), mgau_veclen(g));

    if (cb2mllrname && strcmp(cb2mllrname, ".1cls.") != 0) {
        uint32 ncb, nmllr;

        cb2mllr_read(cb2mllrname, &cb2mllr, &ncb, &nmllr);
        if (nmllr != ad->mllr_nclass)
            E_FATAL
                ("Number of classes in cb2mllr does not match mllr (%d != %d)\n",
                 ncb, ad->mllr_nclass);
        if (ncb != mdef->n_sen)
            E_FATAL
                ("Number of senones in cb2mllr does not match mdef (%d != %d)\n",
                 ncb, mdef->n_sen);
    }
    else
        cb2mllr = NULL;

    /* Transform mean and variance vectors */
    mllr_norm_mgau(g, ad->regA, ad->regB, ad->regH, ad->mllr_nclass, cb2mllr);
    ckd_free(cb2mllr);

    if (g->mgau[0].var && varfloor > 0.0)
        mgau_var_floor(g, varfloor);    /* Variance floor after above transform */

    /* Re-precompute variance things */
    mgau_precomp(g);

#if MLLR_DEBUG
    /*#if 1 */
    mllr_dump(ad->regA, ad->regB, ad->regH, mgau_veclen(g), g->mllr_class, cb2mllr);
    /*This generates huge amount of information */
    /*mgau_dump(kbcore_mgau(kb->kbcore),1); */
#endif

}
