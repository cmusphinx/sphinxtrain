/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sphinxbase/bio.h>
#include <sphinxbase/pio.h>
#include <sphinxbase/feat.h>
#include <sphinxbase/filename.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/agc.h>
#include <sphinxbase/cmn.h>

#include "s3types.h"
#include "logs3.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"

/* ARCHAN: Dangerous routine :-)*/
#include "s3_align.h"
#include "adaptor.h"
#include "approx_cont_mgau.h"
#include "cont_mgau.h"
#include "s2_semi_mgau.h"
#include "ms_mgau.h"
#include "ms_mllr.h"
#include "ms_gauden.h"
#include "ms_senone.h"
#include "interp.h"
#include "cb2mllr_io.h"
#include "cmdln_macro.h"
#include "corpus.h"
#include "kbcore.h"

/** \file main_align.c
   \brief Main driver routine for time alignment.

*/

static arg_t defn[] = {
        waveform_to_cepstral_command_line_macro(),
        cepstral_to_feature_command_line_macro(),

        log_table_command_line_macro(),
        acoustic_model_command_line_macro(),
        speaker_adaptation_command_line_macro(),
        dictionary_command_line_macro(),
        common_application_properties_command_line_macro(),
        control_file_handling_command_line_macro(),
        hypothesis_file_handling_command_line_macro(),
        control_mllr_file_command_line_macro(),
        cepstral_input_handling_command_line_macro(),
    {"-lambda",
     ARG_STRING,
     NULL,
     "Interpolation weights (CD/CI senone) parameters input file"},

    /* align-specific argument */
    {"-beam",
     ARG_FLOAT64,
     "1e-64",
     "Main pruning beam applied to triphones in forward search"},
    {"-insent",
     REQARG_STRING,
     NULL,
     "Input transcript file corresponding to control file"},
    {"-outsent",
     ARG_STRING,
     NULL,
     "Output transcript file with exact pronunciation/transcription"},
    {"-outctl",
     ARG_STRING,
     NULL,
     "Output control file with ctl file entries for aligned files"},
    {"-stsegdir",
     ARG_STRING,
     NULL,
     "Output directory for state segmentation files; optionally end with ,CTL"},
    {"-phsegdir",
     ARG_STRING,
     NULL,
     "Output directory for phone segmentation files; optionally end with ,CTL"},
    {"-wdsegdir",
     ARG_STRING,
     NULL,
     "Output directory for word segmentation files; optionally end with ,CTL"},
    {"-s2stsegdir",
     ARG_STRING,
     NULL,
     "Output directory for Sphinx-II format state segmentation files; optionally end with ,CTL"},
    {"-s2cdsen",
     ARG_BOOLEAN,
     "no",
     "Output context-dependent senone indices in Sphinx-II state segmentations"},
    {"-phlabdir",
     ARG_STRING,
     NULL,
     "Output directory for xlabel style phone labels; optionally end with ,CTL"},
   /*{"-frate",
     ARG_INT32,
     ARG_STRINGIFY(DEFAULT_FRAME_RATE),
     "Frame rate (only requred for xlabel style phone labels)"},*/
    {"-insert_sil",
     ARG_INT32,
     "1",
     "Whether to insert optional silences and fillers between words."},
    fast_GMM_computation_command_line_macro(),
    {NULL, ARG_INT32, NULL, NULL}
};


/** These are the definition of HMMs */

static kbcore_t *kbc;           /* A kbcore structure */
static fe_t *fe;                /* Waveform data handling ('-adcin') */
static ascr_t *ascr;            /* An acoustic score structure.  */
static fast_gmm_t *fastgmm;     /* A fast GMM parameter structure.  */
static adapt_am_t *adapt_am;    /* An adaptation structure. */
/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static dict_t *dict;

static float32 ***feat = NULL;  /* Speech feature data */

static int32 ctloffset;

static const char *outsentfile;
static const char *outctlfile;
static FILE *outsentfp = NULL;
static FILE *outctlfp = NULL;

static const char *sentfile;
static FILE *sentfp = NULL;

static char *s2stsegdir = NULL;
static char *stsegdir = NULL;
static char *phsegdir = NULL;
static char *phlabdir = NULL;
static char *wdsegdir = NULL;


/* For profiling/timing */
enum { tmr_utt, tmr_gauden, tmr_senone, tmr_align };
ptmr_t timers[5];

static int32 tot_nfr;
static ptmr_t tm_utt;
static ptmr_t tm_ovrhd;


static void
models_init(cmd_ln_t *config)
{
    int32 cisencnt;

    kbc = New_kbcore(config);

    kbc->logmath = logs3_init(cmd_ln_float64_r(config, "-logbase"), 1,
                              cmd_ln_int32_r(config, "-log3table"));

    /* Initialize feaure stream type */
    kbc->fcb = feat_init(cmd_ln_str_r(config, "-feat"),
			 cmn_type_from_str(cmd_ln_str_r(config, "-cmn")),
			 cmd_ln_boolean_r(config, "-varnorm"),
			 agc_type_from_str(cmd_ln_str_r(config, "-agc")), 1,
			 cmd_ln_int32_r(config, "-ceplen"));

    s3_am_init(kbc);

    /* Initialize the front end if -adcin is specified */
    if (cmd_ln_exists_r(config, "-adcin") && cmd_ln_boolean_r(config, "-adcin")) {
        if ((fe = fe_init_auto_r(config)) == NULL) {
            E_FATAL("fe_init_auto_r() failed\n");
        }
    }

    assert(kbc);
    assert(kbc->mdef);
    assert(kbc->tmat);

    /* Dictionary */
    dict = dict_init(kbc->mdef, cmd_ln_str_r(config, "-dict"),
                     cmd_ln_str_r(config, "-fdict"), 
                     cmd_ln_int32_r(config, "-lts_mismatch"),
		     cmd_ln_boolean_r(config, "-mdef_fillers"),
		     /* Never do mdef filler phones. */
		     FALSE,
		     TRUE);




    for (cisencnt = 0; cisencnt == kbc->mdef->cd2cisen[cisencnt];
         cisencnt++);

    ascr = ascr_init(kbc->mdef->n_sen, 0,       /* No composite senone */
                     mdef_n_sseq(kbc->mdef), 0, /* No composite senone sequence */
                     1,         /* Phoneme lookahead window =1. Not enabled phoneme lookahead at this moment */
                     cisencnt);

    fastgmm = fast_gmm_init(cmd_ln_int32_r(config, "-ds"),
                            cmd_ln_int32_r(config, "-cond_ds"),
                            cmd_ln_int32_r(config, "-dist_ds"),
                            cmd_ln_int32_r(config, "-gs4gs"),
                            cmd_ln_int32_r(config, "-svq4svq"),
                            cmd_ln_float64_r(config, "-subvqbeam"),
                            cmd_ln_float64_r(config, "-ci_pbeam"),
                            cmd_ln_float64_r(config, "-tighten_factor"),
                            cmd_ln_int32_r(config, "-maxcdsenpf"),
                            kbc->mdef->n_ci_sen,
                            kbc->logmath);
    adapt_am = adapt_am_init();
}

static void models_free(void)
{
    if (adapt_am)
        adapt_am_free(adapt_am);
    if (fastgmm)
        fast_gmm_free(fastgmm);
    if (ascr)
        ascr_free(ascr);
    if (dict)
        dict_free(dict);

    kbcore_free(kbc);
}


/*
 * Build a filename int buf as follows (without file extension):
 *     if dir ends with ,CTLand ctlspec does not begin with /, filename is dir/ctlspec
 *     if dir ends with ,CTL and ctlspec DOES begin with /, filename is ctlspec
 *     if dir does not end with ,CTL, filename is dir/uttid,
 * where ctlspec is the complete utterance spec in the input control file, and
 * uttid is the last component of ctlspec.
 */
static void
build_output_uttfile(char *buf, char *dir, char *uttid, char *ctlspec)
{
    int32 k;

    k = strlen(dir);
    if ((k > 4) && (strcmp(dir + k - 4, ",CTL") == 0)) {        /* HACK!! Hardwired ,CTL */
        if (ctlspec[0] != '/') {
            strcpy(buf, dir);
            buf[k - 4] = '/';
            strcpy(buf + k - 3, ctlspec);
        }
        else
            strcpy(buf, ctlspec);
    }
    else {
        strcpy(buf, dir);
        buf[k] = '/';
        strcpy(buf + k + 1, uttid);
    }
    /* Build output directory structure if possible/requested (it is
     * by default). */
    if (cmd_ln_boolean("-build_outdirs")) {
        char *dirname = ckd_salloc(buf);
        path2dirname(buf, dirname);
        build_directory(dirname);
        ckd_free(dirname);
    }
}


/*
 * Write state segmentation in Sphinx-II format.  (Must be written in BIG-ENDIAN
 * format!)
 */
static void
write_s2stseg(char *dir, align_stseg_t * stseg, char *uttid, char *ctlspec, int32 cdsen)
{
    char filename[1024];
    FILE *fp;
    align_stseg_t *tmp;
    int32 k;
    s3cipid_t ci[3];
    word_posn_t wpos;
    int16 s2_info;
    char buf[8];
    static int32 byterev = -1;  /* Whether to byte reverse output data */

    build_output_uttfile(filename, dir, uttid, ctlspec);
    strcat(filename, ".v8_seg");        /* .v8_seg for compatibility */
    E_INFO("Writing Sphinx-II format state segmentation to: %s\n",
           filename);
    if (cdsen) {
        E_INFO("Writing context-dependent state indices in segmentation file\n");
        if (mdef_n_sen(kbc->mdef) > 0xffff) {
            E_ERROR("Number of senones exceeds 65535, cannot write them to segmentation file\n");
            return;
        }
    }
    if ((fp = fopen(filename, "wb")) == NULL) {
        E_ERROR_SYSTEM("Failed to open %s for writing", filename);
        return;
    }

    if (byterev < 0) {
        /* Byte ordering of host machine unknown; first figure it out */
        k = (int32) BYTE_ORDER_MAGIC;
        if (fwrite(&k, sizeof(int32), 1, fp) != 1)
            goto write_error;

        fclose(fp);
        if ((fp = fopen(filename, "rb")) == NULL) {
            E_ERROR_SYSTEM("Failed to open file %s for reading", filename);
            return;
        }
        if (fread(buf, 1, sizeof(int32), fp) != sizeof(int32)) {
            E_ERROR_SYSTEM("Failed to read from the file %s", filename);
            return;
        }
        fclose(fp);

        /* If buf[0] == lsB of BYTE_ORDER_MAGIC, we are little-endian.  Need to byterev */
        byterev = (buf[0] == (BYTE_ORDER_MAGIC & 0x000000ff)) ? 1 : 0;

        if ((fp = fopen(filename, "wb")) == NULL) {
            E_ERROR_SYSTEM("Failed to open file %s for writing", filename);
            return;
        }
    }

    /* Write #frames */
    for (k = 0, tmp = stseg; tmp; k++, tmp = tmp->next);
    if (byterev)
        SWAP_INT32(&k);
    if (fwrite(&k, sizeof(int32), 1, fp) != 1)
        goto write_error;

    /* Write state info for each frame */
    for (; stseg; stseg = stseg->next) {
        if (cdsen) {
            s2_info = stseg->sen;
        }
        else {
            mdef_phone_components(kbc->mdef, stseg->pid, ci, &(ci[1]),
                                  &(ci[2]), &wpos);
            s2_info = ci[0] * kbc->mdef->n_emit_state + stseg->state;
            if (stseg->start)
                s2_info |= 0x8000;
        }
        if (byterev)
            SWAP_INT16(&s2_info);

        if (fwrite(&s2_info, sizeof(int16), 1, fp) != 1)
            goto write_error;
    }

    fclose(fp);
    return;

  write_error:
    E_ERROR_SYSTEM("Failed to write data to the file %s", filename);
    fclose(fp);
}


/* Write state segmentation output file */
static void
write_stseg(char *dir, align_stseg_t * stseg, char *uttid, char *ctlspec)
{
    char filename[1024];
    FILE *fp;
    align_stseg_t *tmp;
    int32 i, k;
    s3cipid_t ci[3];
    uint8 pos;
    char *str;
    word_posn_t wpos;

    build_output_uttfile(filename, dir, uttid, ctlspec);
    strcat(filename, ".stseg");
    E_INFO("Writing state segmentation to: %s\n", filename);
    if ((fp = fopen(filename, "wb")) == NULL) {
        E_ERROR_SYSTEM("Failed to open file %s for writing\n", filename);
        return;
    }

    /* Write version no. */
    if (fwrite("0.1\n", sizeof(char), 4, fp) != 4)
        goto write_error;

    /* Write CI phone names */
    for (k = 0; k < kbc->mdef->n_ciphone; k++) {
        const char *str = mdef_ciphone_str(kbc->mdef, k);
        if (fwrite(str, sizeof(char), strlen(str), fp) != strlen(str))
            goto write_error;
        if (fwrite(" ", sizeof(char), 1, fp) != 1)
            goto write_error;
    }
    str = WPOS_NAME;
    if (fwrite(str, sizeof(char), strlen(str), fp) != strlen(str))
        goto write_error;

    /* Write format "description" */
    str = "\nCI.16 LC.16 RC.16 POS.3(HI)-ST.5(LO) SCR(32)\n";
    if (fwrite(str, sizeof(char), strlen(str), fp) != strlen(str))
        goto write_error;

    /* Write binary comment string */
    if (fwrite("*end_comment*\n", sizeof(char), 14, fp) != 14)
        goto write_error;

    /* Write byte-ordering magic number */
    k = BYTE_ORDER_MAGIC;
    if (fwrite(&k, sizeof(int32), 1, fp) != 1)
        goto write_error;

    /* Write #frames */
    for (k = 0, tmp = stseg; tmp; k++, tmp = tmp->next);
    if (fwrite(&k, sizeof(int32), 1, fp) != 1)
        goto write_error;

    /* Write state segmentation for each frame */
    for (i = 0; stseg; i++, stseg = stseg->next) {
        mdef_phone_components(kbc->mdef, stseg->pid, ci, &(ci[1]),
                              &(ci[2]), &wpos);
        assert((wpos >= 0) && (wpos < 8));
        assert((stseg->state >= 0) && (stseg->state < 32));

        if (fwrite(ci, sizeof(s3cipid_t), 3, fp) != 3)
            goto write_error;
        pos = (wpos << 5) | (stseg->state & 0x001f);
        if (fwrite(&pos, sizeof(uint8), 1, fp) != 1)
            goto write_error;

        k = stseg->score;
        if (fwrite(&k, sizeof(int32), 1, fp) != 1)
            goto write_error;
    }

    fclose(fp);
    return;

  write_error:
    E_ERROR_SYSTEM("Failed to write to the file %s\n", filename);
    fclose(fp);
}


/* Write phone segmentation output file */
static void
write_phseg(char *dir, align_phseg_t * phseg, char *uttid, char *ctlspec)
{
    char str[1024];
    FILE *fp;
    int32 uttscr;

    /* Attempt to write segmentation for this utt to a separate file */
    build_output_uttfile(str, dir, uttid, ctlspec);
    strcat(str, ".phseg");
    E_INFO("Writing phone segmentation to: %s\n", str);
    if ((fp = fopen(str, "w")) == NULL) {
        E_ERROR_SYSTEM("Failed to open file %s for writing", str);
        fp = stdout;            /* Segmentations can be directed to stdout this way */
        E_INFO("Phone segmentation (%s):\n", uttid);
        dir = NULL;             /* Flag to indicate fp shouldn't be closed at the end */
    }

    if (!dir) {
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }
    fprintf(fp, "\t%5s %5s %9s %s\n", "SFrm", "EFrm", "SegAScr", "Phone");
    fflush(fp);
    uttscr = 0;
    for (; phseg; phseg = phseg->next) {
        mdef_phone_str(kbc->mdef, phseg->pid, str);

        if (!dir) {
            fprintf(fp, "ph:%s>", uttid);
            fflush(fp);
        }
        fprintf(fp, "\t%5d %5d %9d %s\n",
                phseg->sf, phseg->ef, phseg->score, str);
        fflush(fp);
        uttscr += (phseg->score);
    }

    if (!dir) {
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }
    fprintf(fp, " Total score: %11d\n", uttscr);
    fflush(fp);

    if (dir)
        fclose(fp);
    else {
        fprintf(fp, "\n");
        fflush(fp);
    }
}

/* Write xlabel style phone segmentation output file */
static void
write_phlab(char *dir, align_phseg_t * phseg, char *uttid, char *ctlspec, int32 fps)
{
    char str[1024];
    FILE *fp;

    /* Attempt to write segmentation for this utt to a separate file */
    build_output_uttfile(str, dir, uttid, ctlspec);
    strcat(str, ".lab");
    E_INFO("Writing xlabel style phone labels to: %s\n", str);
    if ((fp = fopen(str, "w")) == NULL) {
        E_ERROR_SYSTEM("Failed to open file %s for writing", str);
        fp = stdout;            /* Segmentations can be directed to stdout this way */
        E_INFO("Phone segmentation (%s):\n", uttid);
        dir = NULL;             /* Flag to indicate fp shouldn't be closed at the end */
    }

    if (!dir) {
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }
    fprintf(fp, "#\n");
    for (; phseg; phseg = phseg->next) {
        const char *name;

        name =
            mdef_ciphone_str(kbc->mdef, kbc->mdef->phone[phseg->pid].ci);

        if (!dir) {
            fprintf(fp, "ph:%s>", uttid);
            fflush(fp);
        }
        fprintf(fp, "%0.6f 125 %s\n", (double) phseg->ef / fps, name);
        fflush(fp);
    }

    if (dir)
        fclose(fp);
    else {
        fprintf(fp, "\n");
        fflush(fp);
    }
}


/* Write word segmentation output file */
static void
write_wdseg(char *dir, align_wdseg_t * wdseg, char *uttid, char *ctlspec)
{
    char str[1024];
    FILE *fp;
    int32 uttscr;

    /* Attempt to write segmentation for this utt to a separate file */
    build_output_uttfile(str, dir, uttid, ctlspec);
    strcat(str, ".wdseg");
    E_INFO("Writing word segmentation to: %s\n", str);
    if ((fp = fopen(str, "w")) == NULL) {
        E_ERROR_SYSTEM("Failed to open file %s for writing", str);
        fp = stdout;            /* Segmentations can be directed to stdout this way */
        E_INFO("Word segmentation (%s):\n", uttid);
        dir = NULL;             /* Flag to indicate fp shouldn't be closed at the end */
    }

    if (!dir) {
        fprintf(fp, "WD:%s>", uttid);
        fflush(fp);
    }
    fprintf(fp, "\t%5s %5s %10s %s\n", "SFrm", "EFrm", "SegAScr", "Word");
    fflush(fp);
    uttscr = 0;
    for (; wdseg; wdseg = wdseg->next) {
        if (!dir) {
            fprintf(fp, "wd:%s>", uttid);
            fflush(fp);
        }
        fprintf(fp, "\t%5d %5d %10d %s\n",
                wdseg->sf, wdseg->ef, wdseg->score,
                dict_wordstr(dict, wdseg->wid));
        fflush(fp);


        uttscr += wdseg->score;
    }

    if (!dir) {
        fprintf(fp, "WD:%s>", uttid);
        fflush(fp);
    }

    fprintf(fp, " Total score: %11d\n", uttscr);
    fflush(fp);
    if (dir)
        fclose(fp);
    else {
        fprintf(fp, "\n");
        fflush(fp);
    }
}


/* Write exact transcription (pronunciation and silence/noise words included) */
static void
write_outsent(FILE * fp, align_wdseg_t * wdseg, char *uttid)
{
    for (; wdseg; wdseg = wdseg->next)
        fprintf(fp, "%s ", dict_wordstr(dict, wdseg->wid));
    fprintf(fp, " (%s)\n", uttid);
    fflush(fp);
}

/* Write output ctlfile entry */
static void
write_outctl(FILE * fp, char *uttctl)
{
    fprintf(fp, "%s\n", uttctl);
    fflush(fp);
}



/*
 * Find Viterbi alignment.
 */
static void
align_utt(char *sent,           /* In: Reference transcript */
          int32 nfr,            /* In: #frames of input */
          char *ctlspec,        /* In: Utt specifiction from control file */
          char *uttid)
{                               /* In: Utterance id, for logging and other use */
    int32 i;
    align_stseg_t *stseg;
    align_phseg_t *phseg;
    align_wdseg_t *wdseg;
    int32 w;

    w = feat_window_size(kbcore_fcb(kbc));  /* #MFC vectors needed on either side of current
                                   frame to compute one feature vector */
    if (nfr <= (w << 1)) {
        E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid,
                (w << 1) + 1, nfr);
        return;
    }

    ptmr_reset_all(timers);

    ptmr_reset(&tm_utt);
    ptmr_start(&tm_utt);
    ptmr_reset(&tm_ovrhd);
    ptmr_start(&tm_ovrhd);
    ptmr_start(timers + tmr_utt);


    if (align_build_sent_hmm(sent, cmd_ln_int32_r(kbc->config, "-insert_sil")) != 0) {
        align_destroy_sent_hmm();
        ptmr_stop(timers + tmr_utt);

        E_ERROR("No sentence HMM; no alignment for %s\n", uttid);

        return;
    }

    align_start_utt(uttid);

    for (i = 0; i < nfr; i++) {
        ptmr_start(timers + tmr_utt);

        /* Obtain active senone flags */
        ptmr_start(timers + tmr_gauden);
        ptmr_start(timers + tmr_senone);

        align_sen_active(ascr->sen_active, ascr->n_sen);

        /* Bah, there ought to be a function for this. */
        if (kbc->ms_mgau) {
            ms_cont_mgau_frame_eval(ascr,
				    kbc->ms_mgau,
				    kbc->mdef, feat[i], i);
        }
        else if (kbc->s2_mgau) {
            s2_semi_mgau_frame_eval(kbc->s2_mgau,
				    ascr, fastgmm, feat[i],
				    i);
        }
        else if (kbc->mgau) {
            approx_cont_mgau_ci_eval(kbcore_svq(kbc),
                                     kbcore_gs(kbc),
                                     kbcore_mgau(kbc),
                                     fastgmm,
                                     kbc->mdef,
                                     feat[i][0],
                                     ascr->cache_ci_senscr[0],
                                     &(ascr->cache_best_list[0]), i,
                                     kbcore_logmath(kbc));
            approx_cont_mgau_frame_eval(kbcore_mdef(kbc),
					kbcore_svq(kbc),
					kbcore_gs(kbc),
					kbcore_mgau(kbc),
					fastgmm, ascr,
					feat[i][0], i,
					ascr->
					cache_ci_senscr[0],
					&tm_ovrhd,
					kbcore_logmath(kbc));
        }

        ptmr_stop(timers + tmr_gauden);
        ptmr_stop(timers + tmr_senone);

        /* Step alignment one frame forward */
        ptmr_start(timers + tmr_align);
        align_frame(ascr->senscr);
        ptmr_stop(timers + tmr_align);
        ptmr_stop(timers + tmr_utt);
    }
    ptmr_stop(&tm_utt);
    ptmr_stop(&tm_ovrhd);

    printf("\n");

    /* Wind up alignment for this utterance */
    if (align_end_utt(&stseg, &phseg, &wdseg) < 0)
        E_ERROR("Final state not reached; no alignment for %s\n\n", uttid);
    else {
        if (s2stsegdir)
            write_s2stseg(s2stsegdir, stseg, uttid, ctlspec, cmd_ln_boolean_r(kbc->config, "-s2cdsen"));
        if (stsegdir)
            write_stseg(stsegdir, stseg, uttid, ctlspec);
        if (phsegdir)
            write_phseg(phsegdir, phseg, uttid, ctlspec);
        if (phlabdir)
            write_phlab(phlabdir, phseg, uttid, ctlspec, cmd_ln_int32_r(kbc->config, "-frate"));
        if (wdsegdir)
            write_wdseg(wdsegdir, wdseg, uttid, ctlspec);
        if (outsentfp)
            write_outsent(outsentfp, wdseg, uttid);
        if (outctlfp)
            write_outctl(outctlfp, ctlspec);
    }

    align_destroy_sent_hmm();

    ptmr_print_all(stdout, timers, nfr * 0.1);

    printf
        ("EXECTIME: %5d frames, %7.2f sec CPU, %6.2f xRT; %7.2f sec elapsed, %6.2f xRT\n",
         nfr, tm_utt.t_cpu, tm_utt.t_cpu * 100.0 / nfr, tm_utt.t_elapsed,
         tm_utt.t_elapsed * 100.0 / nfr);

    tot_nfr += nfr;
}


#define UPPER_CASE(c)   ((((c) >= 'a') && ((c) <= 'z')) ? (c-32) : c)
/* Case insensitive string compare */
static int32
id_cmp(char *str1, char *str2)
{
    char c1, c2;

    for (;;) {
        c1 = *(str1++);
        c1 = UPPER_CASE(c1);
        c2 = *(str2++);
        c2 = UPPER_CASE(c2);
        if (c1 != c2)
            return (c1 - c2);
        if (c1 == '\0')
            return 0;
    }
}

static void
utt_align(void *data, utt_res_t * ur, int32 sf, int32 ef, char *uttid)
{
    int32 nfr;
    int k, i;
    const char *cepdir;
    const char *cepext;
    char sent[16384];
    cmd_ln_t *config = (cmd_ln_t*) data;

    cepdir = cmd_ln_str_r(kbc->config, "-cepdir");
    cepext = cmd_ln_str_r(kbc->config, "-cepext");


    /* UGLY! */
    /* Read utterance transcript and match it with the control file. */
    if (fgets(sent, sizeof(sent), sentfp) == NULL) {
        E_FATAL("EOF(%s) of the transcription\n", sentfile);
    }
    /*  E_INFO("SENT %s\n",sent); */
    /* Strip utterance id from the end of the transcript */
    for (k = strlen(sent) - 1;
         (k > 0) && ((sent[k] == '\n') || (sent[k] == '\t') || (sent[k] == '\r')
                     || (sent[k] == ' ')); --k);
    if ((k > 0) && (sent[k] == ')')) {
        for (--k; (k >= 0) && (sent[k] != '('); --k);
        if ((k >= 0) && (sent[k] == '(')) {
            sent[k] = '\0';

            /* Check that uttid in transcript and control file match */
            for (i = ++k;
                 sent[i] && (sent[i] != ')') &&
                 (sent[i] != '\n') && (sent[i] != '\t') && (sent[i] != '\r')
                 && (sent[i] != ' '); i++);
            sent[i] = '\0';
            if (id_cmp(sent + k, uttid) != 0)
                E_ERROR
                    ("Uttid mismatch: ctlfile = \"%s\"; transcript = \"%s\"\n",
                     uttid, sent + k);
        }
    }

    /* Convert input file to cepstra if waveform input is selected */
    if (cmd_ln_boolean_r(config, "-adcin")) {
        int16 *adcdata;
        size_t nsamps = 0;
        mfcc_t **mfcc;

        if ((adcdata = bio_read_wavfile(cmd_ln_str_r(config, "-cepdir"),
    				        ur->uttfile,
    				        cmd_ln_str_r(config, "-cepext"),
    				        cmd_ln_int32_r(config, "-adchdr"),
    				        strcmp(cmd_ln_str_r(config, "-input_endian"), "big"),
    				        &nsamps)) == NULL) {
            E_FATAL("Cannot read file %s\n", ur->uttfile);
        }
        fe_start_utt(fe);
        if (fe_process_utt(fe, adcdata, nsamps, &mfcc, &nfr) < 0) {
            E_FATAL("MFCC calculation failed\n", ur->uttfile);
        }
        ckd_free(adcdata);
        if (nfr > S3_MAX_FRAMES) {
            E_FATAL("Maximum number of frames (%d) exceeded\n", S3_MAX_FRAMES);
        }
        if ((nfr = feat_s2mfc2feat_live(kbcore_fcb(kbc),
						mfcc,
						&nfr,
						TRUE, TRUE,
						feat)) < 0) {
            E_FATAL("Feature computation failed\n");
        }
        if (mfcc)
            ckd_free_2d((void **)mfcc);
    }
    else {
        nfr =
            feat_s2mfc2feat(kbcore_fcb(kbc), ur->uttfile, cepdir, cepext, sf, ef, feat,
                            S3_MAX_FRAMES);
    }

    if (ur->regmatname) {
        if (kbc->mgau)
            adapt_set_mllr(adapt_am, kbc->mgau, ur->regmatname,
                           ur->cb2mllrname, kbc->mdef, kbc->config);
        else if (kbc->ms_mgau)
            model_set_mllr(kbc->ms_mgau, ur->regmatname, ur->cb2mllrname,
                           kbcore_fcb(kbc), kbc->mdef, kbc->config);
        else
            E_WARN("Can't use MLLR matrices with .s2semi. yet\n");
    }

    if (nfr <= 0) {
        if (cepdir != NULL) {
            E_ERROR
                ("Utt %s: Input file read (%s) with dir (%s) and extension (%s) failed \n",
                 uttid, ur->uttfile, cepdir, cepext);
        }
        else {
            E_ERROR
                ("Utt %s: Input file read (%s) with extension (%s) failed \n",
                 uttid, ur->uttfile, cepext);
        }
    }
    else {
        E_INFO("%s: %d input frames\n", uttid, nfr);
        align_utt(sent, nfr, ur->uttfile, uttid);
    }

}
int
main(int32 argc, char *argv[])
{
    char sent[16384];
    cmd_ln_t *config;

    cmd_ln_appl_enter(argc, argv, "default.arg", defn);

    config = cmd_ln_get();

    ctloffset = cmd_ln_int32_r(config, "-ctloffset");
    sentfile = cmd_ln_str_r(config, "-insent");

    if ((sentfp = fopen(sentfile, "r")) == NULL)
        E_FATAL_SYSTEM("Failed to open file %s for reading", sentfile);

    /* Note various output directories */
    if (cmd_ln_str_r(config, "-s2stsegdir") != NULL)
        s2stsegdir = (char *) ckd_salloc(cmd_ln_str_r(config, "-s2stsegdir"));
    if (cmd_ln_str_r(config, "-stsegdir") != NULL)
        stsegdir = (char *) ckd_salloc(cmd_ln_str_r(config, "-stsegdir"));
    if (cmd_ln_str_r(config, "-phsegdir") != NULL)
        phsegdir = (char *) ckd_salloc(cmd_ln_str_r(config, "-phsegdir"));
    if (cmd_ln_str_r(config, "-phlabdir") != NULL)
        phlabdir = (char *) ckd_salloc(cmd_ln_str_r(config, "-phlabdir"));
    if (cmd_ln_str_r(config, "-wdsegdir") != NULL)
        wdsegdir = (char *) ckd_salloc(cmd_ln_str_r(config, "-wdsegdir"));

    /* HACK! Pre-read insent without checking whether ctl could also 
       be read.  In general, this is caused by the fact that we used
       multiple files to specify resource in sphinx III.  This is easy
       to solve but currently I just to remove process_ctl because it
       duplicates badly with ctl_process.  

       The call back function will take care of matching the uttfile
       names. We don't need to worry too much about inconsistency. 
     */

    while (ctloffset > 0) {
        if (fgets(sent, sizeof(sent), sentfp) == NULL) {
            E_ERROR("EOF(%s)\n", sentfile);
            break;
        }
        --ctloffset;
    }

    if ((outsentfile = cmd_ln_str_r(config, "-outsent")) != NULL) {
        if ((outsentfp = fopen(outsentfile, "w")) == NULL)
            E_FATAL_SYSTEM("Failed to open file %s for writing", outsentfile);
    }

    if ((outctlfile = cmd_ln_str_r(config, "-outctl")) != NULL) {
        if ((outctlfp = fopen(outctlfile, "w")) == NULL)
            E_FATAL_SYSTEM("Failed top open file %s for writing", outctlfile);
    }

    if ((cmd_ln_str_r(config, "-s2stsegdir") == NULL) &&
        (cmd_ln_str_r(config, "-stsegdir") == NULL) &&
        (cmd_ln_str_r(config, "-phlabdir") == NULL) &&
        (cmd_ln_str_r(config, "-phsegdir") == NULL) &&
        (cmd_ln_str_r(config, "-wdsegdir") == NULL) &&
        (cmd_ln_str_r(config, "-outsent") == NULL))
        E_FATAL("Missing output file/directory argument(s)\n");

    /* Read in input databases */
    models_init(config);

    if (!feat)
        feat = feat_array_alloc(kbcore_fcb(kbc), S3_MAX_FRAMES);

    timers[tmr_utt].name = "U";
    timers[tmr_gauden].name = "G";
    timers[tmr_senone].name = "S";
    timers[tmr_align].name = "A";

    /* Initialize align module */
    align_init(kbc->mdef, kbc->tmat, dict, config, kbc->logmath);
    printf("\n");

    if (cmd_ln_str_r(config, "-mllr") != NULL) {
        if (kbc->mgau)
            adapt_set_mllr(adapt_am, kbc->mgau, cmd_ln_str_r(config, "-mllr"), NULL,
                           kbc->mdef, config);
        else if (kbc->ms_mgau)
            model_set_mllr(kbc->ms_mgau, cmd_ln_str_r(config, "-mllr"), NULL, kbcore_fcb(kbc),
                           kbc->mdef, config);
        else
            E_WARN("Can't use MLLR matrices with .s2semi. yet\n");
    }

    tot_nfr = 0;

    /*  process_ctlfile (); */

    if (cmd_ln_str_r(config, "-ctl")) {
        /* When -ctlfile is speicified, corpus.c will look at -ctl_mllr to get
           the corresponding  MLLR for the utterance */
        ctl_process(cmd_ln_str_r(config, "-ctl"),
                    NULL,
                    cmd_ln_str_r(config, "-ctl_mllr"),
                    cmd_ln_int32_r(config, "-ctloffset"),
                    cmd_ln_int32_r(config, "-ctlcount"),
                    utt_align, config);
    }
    else {
        E_FATAL(" -ctl are not specified.\n");
    }

    if (tot_nfr > 0) {
        printf("\n");
        printf("TOTAL FRAMES:       %8d\n", tot_nfr);
        printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
               tm_utt.t_tot_cpu, tm_utt.t_tot_cpu / (tot_nfr * 0.01));
        printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
               tm_utt.t_tot_elapsed,
               tm_utt.t_tot_elapsed / (tot_nfr * 0.01));
    }

    if (outsentfp)
        fclose(outsentfp);
    if (outctlfp)
        fclose(outctlfp);
    if (sentfp)
        fclose(sentfp);

    ckd_free(s2stsegdir);
    ckd_free(stsegdir);
    ckd_free(phsegdir);
    ckd_free(wdsegdir);

    feat_array_free(feat);
    align_free();
    models_free();

    cmd_ln_free_r(config);
    return 0;
}
