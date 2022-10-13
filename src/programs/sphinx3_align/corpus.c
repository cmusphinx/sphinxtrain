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
 * corpus.c -- Corpus-file related misc functions.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996-2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.12  2006/02/22  19:49:25  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII:
 * 1, Add structure utt_res_t, this is an utterance-based resouce
 * structure. Add basic operation such as free and report.
 * 2, Modify the structure of the loop in ctl_corpus to make it not so
 * clunky. Tested with make check .
 * 3, Completely removed ctl_process_dyn_lm, it is a product of code
 * duplication (alright, it is written by me......)
 * 4, Fixed doc-dox.
 * 
 * Revision 1.11.4.3  2005/08/02 21:09:07  arthchan2003
 * Removed error message
 *
 * Revision 1.11.4.2  2005/07/27 23:19:11  arthchan2003
 * 1, Added utt_res_t structure and its methods. 2, Changed the function pointer prototype. 3, Removed the lm and mllr set process out of ctl_process
 *
 * Revision 1.11.4.1  2005/07/26 03:14:17  arthchan2003
 * Removed ctl_process_dyn_lm. One of my sin.
 *
 * Revision 1.11  2005/06/21 20:44:34  arthchan2003
 * 1, Fixed doxygen documentation, 2, Add the $ keyword.
 *
 *
 * 09-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added ctl_process_utt ().
 * 
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added check for already existing file extension in ctl_infile().
 * 
 * 23-Mar-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added a general purpose data argument to ctl_process() and its function
 * 		argument func.
 * 
 * 22-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added an optional validation function argument and an optional
 *		duplicate-resolution function argument to both corpus_load_headid() and
 * 		corpus_load_tailid().
 * 
 * 25-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <stdlib.h>
#endif

#include <sphinxbase/pio.h>
#include <sphinxbase/filename.h>

#include "corpus.h"
typedef struct kb_s kb_t;

#if (defined(WIN32) && !defined(__CYGWIN__))
#define SLEEP_SEC(sec)  (0)                     /* Why doesn't Sleep((sec)*1000) work? */
#else
#define SLEEP_SEC(sec)  sleep(sec)              /* sec must be integer */
#endif

utt_res_t *
new_utt_res()
{
    utt_res_t *ur;
    ur = ckd_calloc(1, sizeof(utt_res_t));
    utt_res_set_uttfile(ur, NULL);
    utt_res_set_lmname(ur, NULL);
    utt_res_set_fsgname(ur, NULL);
    utt_res_set_regmatname(ur, NULL);
    utt_res_set_cb2mllrname(ur, NULL);

    return ur;
}

void
free_utt_res(utt_res_t * ur)
{
    ckd_free(ur);
}

void
report_utt_res(utt_res_t * ur)
{
    E_INFO_NOFN("Utt res, report:\n");
    if (ur->uttfile != NULL)
        E_INFO_NOFN("uttfile %s\n", ur->uttfile);
    if (ur->lmname != NULL)
        E_INFO_NOFN("lmname %s\n", ur->lmname);
    if (ur->fsgname != NULL)
        E_INFO_NOFN("fsgname %s\n", ur->fsgname);
    if (ur->regmatname != NULL)
        E_INFO_NOFN("regmatname %s\n", ur->regmatname);
    if (ur->cb2mllrname != NULL)
        E_INFO_NOFN("cb2mllrname %s\n", ur->cb2mllrname);


}

corpus_t *
corpus_load_headid(const char *file,
                   int32(*validate) (char *str),
                   int32(*dup_resolve) (char *s1, char *s2))
{
    FILE *fp;
    char line[16384], wd[4096], *id;
    int32 j, k, m, n;
    corpus_t *corp;

    E_INFO("Loading corpus (%s)\n", file);

    if ((fp = fopen(file, "r")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,r) failed\n", file);

    corp = (corpus_t *) ckd_calloc(1, sizeof(corpus_t));

    n = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* Skip empty lines */
        if (sscanf(line, "%s", wd) == 1)
            n++;
    }
    rewind(fp);

    corp->ht = hash_table_new(n, HASH_CASE_YES);
    corp->n = 0;
    corp->str = (char **) ckd_calloc(n, sizeof(char *));

    n = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* Skip blank lines */
        if (sscanf(line, "%s%n", wd, &k) != 1)
            continue;

        /* Eliminate the line-terminating newline */
        j = strlen(line);
        if ((j > 0) && (line[j - 1] == '\n'))
            line[j - 1] = '\0';

        /* Validate if a validation function is given */
        if (validate && (!(*validate) (line + k))) {
            E_INFO("Corpus validation %s failed; skipping\n", wd);
            continue;
        }

        id = ckd_salloc(wd);
        if ((m = (long) hash_table_enter(corp->ht, id, (void *)(long)n)) != n) {
            /* Duplicate entry */
            if (!dup_resolve)
                E_FATAL
                    ("corpus_load_headid(%s) failed; duplicate ID: %s\n",
                     file, id);
            else {
                /* Invoke the application provided duplicate resolver function */
                if ((j = (*dup_resolve) (corp->str[m], line + k)) < 0)
                    E_FATAL
                        ("corpus_load_headid(%s) failed; duplicate ID: %s\n",
                         file, id);
                ckd_free(id);
                if (j > 0) {
                    /* Overwrite the original with the new entry */
                    ckd_free(corp->str[m]);
                    corp->str[m] = ckd_salloc(line + k);
                }
                else {
                    /* Retain the original entry, discard the new one */
                }
            }
        }
        else {
            /* Fill in new entry */
            corp->str[n] = ckd_salloc(line + k);
            n++;
        }
    }
    corp->n = n;

    fclose(fp);

    E_INFO("%s: %d entries\n", file, n);

    return corp;
}


static int32
sep_tailid(char *line, char *uttid)
{
    int32 i, k, l;

    l = strlen(line);
    uttid[0] = '\0';

    /* Find last close-paren */
    for (i = l - 1; (i >= 0) && ((line[i] == '\n') || (line[i] == ' ')
                                 || (line[i] == '\t')); --i);
    if ((i < 0) || (line[i] != ')'))    /* Missing uttid */
        return -1;
    k = i;

    /* Find closest open-paren; no spaces allowed in uttid */
    for (--i; (i >= 0) && (line[i] != ' ') && (line[i] != '\t')
         && (line[i] != '('); --i);
    if ((i < 0) || (k - i < 2) || (line[i] != '('))     /* Empty or missing uttid */
        return -1;

    /* Remove parentheses and copy uttid */
    line[k] = '\0';
    strcpy(uttid, line + i + 1);

    /* Strip uttid from line */
    line[i] = '\0';

    return 0;
}


corpus_t *
corpus_load_tailid(const char *file,
                   int32(*validate) (char *str),
                   int32(*dup_resolve) (char *s1, char *s2))
{
    FILE *fp;
    char line[16384], uttid[4096], *id;
    int32 j, m, n;
    corpus_t *corp;

    E_INFO("Loading corpus (%s)\n", file);

    if ((fp = fopen(file, "r")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,r) failed\n", file);

    corp = (corpus_t *) ckd_calloc(1, sizeof(corpus_t));

    n = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* Skip empty lines */
        if (sscanf(line, "%s", uttid) == 1)
            n++;
    }
    rewind(fp);

    corp->ht = hash_table_new(n, HASH_CASE_YES);
    corp->n = 0;
    corp->str = (char **) ckd_calloc(n, sizeof(char *));

    n = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* Skip blank lines */
        if (sscanf(line, "%s", uttid) < 1)
            continue;

        /* Look for a (uttid) at the end */
        if (sep_tailid(line, uttid) < 0)
            E_FATAL("corpus_load_tailid(%s) failed; bad line: %s\n", file,
                    line);

        /* Validate if a validation function is given */
        if (validate && (!(*validate) (line))) {
            E_INFO("Corpus validation %s failed; skipping\n", uttid);
            continue;
        }

        id = ckd_salloc(uttid);
        if ((m = (long) hash_table_enter(corp->ht, id, (void *)(long)n)) != n) {
            /* Duplicate entry */
            if (!dup_resolve)
                E_FATAL
                    ("corpus_load_tailid(%s) failed; duplicate ID: %s\n",
                     file, id);
            else {
                /* Invoke the application provided duplicate resolver function */
                if ((j = (*dup_resolve) (corp->str[m], line)) < 0)
                    E_FATAL
                        ("corpus_load(tailid(%s) failed; duplicate ID: %s\n",
                         file, id);
                ckd_free(id);
                if (j > 0) {
                    /* Overwrite the original with the new entry */
                    ckd_free(corp->str[m]);
                    corp->str[m] = ckd_salloc(line);
                }
                else {
                    /* Retain the original entry, discard the new one */
                }
            }
        }
        else {
            /* Fill in new entry */
            corp->str[n] = ckd_salloc(line);
            n++;
        }
    }
    corp->n = n;

    fclose(fp);

    E_INFO("%s: %d entries\n", file, n);

    return corp;
}


char *
corpus_lookup(corpus_t * corp, const char *id)
{
    void *val;
    int32 n;

    if (hash_table_lookup(corp->ht, id, &val) < 0)
        return NULL;
    n = (int32)(long)val;

    assert((n >= 0) && (n < corp->n));
    return (corp->str[n]);
}


#if _CORPUS_TEST_
main(int32 argc, char *argv[])
{
    corpus_t *ch, *ct;
    char id[4096], *str;

    if (argc != 3)
        E_FATAL("Usage: %s headid-corpusfile tailid-corpusfile\n",
                argv[0]);

    ch = corpus_load_headid(argv[1], NULL, NULL);
    ct = corpus_load_tailid(argv[2], NULL, NULL);
    for (;;) {
        printf("> ");
        scanf("%s", id);

        str = corpus_lookup(ch, id);
        if (str == NULL)
            printf("%s Not found in 1\n");
        else
            printf("%s(1): %s\n", id, str);

        str = corpus_lookup(ct, id);
        if (str == NULL)
            printf("%s Not found in 2\n");
        else
            printf("%s(2): %s\n", id, str);
    }
}
#endif


int32
ctl_read_entry(FILE * fp, char *uttfile, int32 * sf, int32 * ef,
               char *uttid)
{
    char line[16384];
    const char *base;
    int32 k;

    do {
        if (fgets(line, sizeof(line), fp) == NULL)
            return -1;
        if (line[0] == '#')
            k = 0;
        else
            k = sscanf(line, "%s %d %d %s", uttfile, sf, ef, uttid);
    } while (k <= 0);

    if ((k == 2) || ((k >= 3) && ((*sf >= *ef) || (*sf < 0))))
        E_FATAL("Error in ctlfile: %s\n", line);

    if (k < 4) {
        /* Create utt-id from mfc-filename (and sf/ef if specified) */
        base = path2basename(uttfile);
        /* strip_fileext (base, uttid); */
        strcpy(uttid, base);

        if (k == 3) {
            k = strlen(uttid);
            sprintf(uttid + k, "_%d_%d", *sf, *ef);
        }
        else {
            *sf = 0;
            *ef = -1;           /* Signifies "until EOF" */
        }
    }

    return 0;
}

#if 0
ptmr_t
ctl_process(char *ctlfile, char *ctlmllrfile, int32 nskip, int32 count,
            void (*func) (void *kb, char *uttfile, int32 sf, int32 ef,
                          char *uttid), void *kb)
{
    FILE *fp, *mllrfp;
    char uttfile[16384], uttid[4096];
    char regmatfile[4096], cb2mllrfile[4096];
    int32 sf, ef;
    ptmr_t tm;

    mllrfp = NULL;
    E_INFO("Batch mode recognition without dynamic LM\n");

    if (ctlfile) {
        if ((fp = fopen(ctlfile, "r")) == NULL)
            E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctlfile);
    }
    else
        fp = stdin;

    if (ctlmllrfile) {
        if ((mllrfp = fopen(ctlmllrfile, "r")) == NULL)
            E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctlmllrfile);
    }

    ptmr_init(&tm);

    if (nskip > 0) {
        E_INFO("Skipping %d entries at the beginning of %s\n", nskip,
               ctlfile);

        for (; nskip > 0; --nskip) {
            if (ctl_read_entry(fp, uttfile, &sf, &ef, uttid) < 0) {
                fclose(fp);
                return tm;
            }
        }

        if (ctlmllrfile) {
            for (; nskip > 0; --nskip) {
                if (ctl_read_entry(fp, regmatfile, &sf, &ef, cb2mllrfile) <
                    0) {
                    E_ERROR
                        ("MLLR cannot be read when skipping the %d-th sentence\n",
                         nskip);
                    fclose(fp);
                    return tm;
                }
            }
        }
    }

    for (; count > 0; --count) {
        if (ctl_read_entry(fp, uttfile, &sf, &ef, uttid) < 0)
            break;

        if (ctlmllrfile) {
            int32 tmp1, tmp2;

            if (ctl_read_entry
                (mllrfp, regmatfile, &tmp1, &tmp2, cb2mllrfile) < 0) {
                E_ERROR
                    ("MLLR cannot be read when counting the %d-th sentence\n",
                     count);
                break;
            }
            if (tmp2 == -1)
                strcpy(cb2mllrfile, ".1cls.");
        }

        /* Process this utterance */
        ptmr_start(&tm);
        if (func) {
            if (ctlmllrfile)
                kb_setmllr(regmatfile, cb2mllrfile, kb);
            (*func) (kb, uttfile, sf, ef, uttid);
        }
        ptmr_stop(&tm);

        E_INFO
            ("%s: %6.1f sec CPU, %6.1f sec Clk;  TOT: %8.1f sec CPU, %8.1f sec Clk\n\n",
             uttid, tm.t_cpu, tm.t_elapsed, tm.t_tot_cpu,
             tm.t_tot_elapsed);


        ptmr_reset(&tm);
    }

    if (fp)
        fclose(fp);


    return tm;
}
#endif

ptmr_t
ctl_process(const char *ctlfile, const char *ctllmfile, const char *ctlmllrfile, int32 nskip,
            int32 count, void (*func) (void *kb, utt_res_t * ur, int32 sf,
                                       int32 ef, char *uttid), void *kb)
{
    FILE *fp;
    FILE *ctllmfp;
    FILE *ctlmllrfp;
    char uttfile[16384], uttid[4096];
    char lmname[4096];
    char regmatname[4096], cb2mllrname[4096];
    char tmp[4096];
    int32 sf, ef;
    utt_res_t *ur;
    ptmr_t tm;
    kb_t *k;

    k = (kb_t *) kb;

    ctllmfp = NULL;
    ctlmllrfp = NULL;
    ur = new_utt_res();

    if (ctlfile) {
        if ((fp = fopen(ctlfile, "r")) == NULL)
            E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctlfile);
    }
    else
        fp = stdin;

    if (ctllmfile) {
        E_INFO("LM is used in this session\n");
        if ((ctllmfp = fopen(ctllmfile, "r")) == NULL)
            E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctllmfile);
    }

    if (ctlmllrfile) {
        E_INFO("MLLR is used in this session\n");
        if ((ctlmllrfp = fopen(ctlmllrfile, "r")) == NULL)
            E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctlmllrfile);
    }

    ptmr_init(&tm);

    if (nskip > 0) {
        E_INFO("Skipping %d entries at the beginning of %s\n", nskip,
               ctlfile);

        for (; nskip > 0; --nskip) {
            if (ctl_read_entry(fp, uttfile, &sf, &ef, uttid) < 0) {
                fclose(fp);
                return tm;
            }

            /*This checks the size of the control file of the lm in batch mode */
            if (ctllmfile) {
                if (ctl_read_entry(ctllmfp, lmname, &sf, &ef, tmp) < 0) {
                    fclose(ctllmfp);
                    E_ERROR
                        ("An LM control file is specified but LM cannot be read when skipping the %d-th sentence\n",
                         nskip);
                    return tm;
                }
            }

            /*This checks the size of the control file of the mllr in batch mode */
            if (ctlmllrfile) {
                if (ctl_read_entry(ctlmllrfp, regmatname, &sf, &ef, tmp) <
                    0) {
                    fclose(ctlmllrfp);
                    E_ERROR
                        ("A MLLR control file is specified but MLLR cannot be read when skipping the %d-th sentence\n",
                         nskip);
                    return tm;
                }
            }

        }
    }

    for (; count > 0; --count) {
        int32 tmp1, tmp2;


        if (ctl_read_entry(fp, uttfile, &sf, &ef, uttid) < 0)
            break;

        /*This checks the size of the control file in batch mode */
        if (ctllmfile) {
            if (ctl_read_entry(ctllmfp, lmname, &tmp1, &tmp2, tmp) < 0) {
                fclose(ctllmfp);
                E_ERROR
                    ("LM control file is specified but LM cannot be read when counting the %d-th sentence\n",
                     count);
                break;
            }
        }

        if (ctlmllrfile) {
            if (ctl_read_entry
                (ctlmllrfp, regmatname, &tmp1, &tmp2, cb2mllrname) < 0) {
                E_ERROR
                    ("MLLR control file is specified but MLLR cannot be read when counting the %d-th sentence\n",
                     count);
                break;
            }
            if (tmp2 == -1)
                strcpy(cb2mllrname, ".1cls.");
        }


        /* Process this utterance */
        ptmr_start(&tm);
        if (func) {
            utt_res_set_uttfile(ur, uttfile);
            if (ctllmfile)
                utt_res_set_lmname(ur, lmname);
            if (ctlmllrfile) {
                utt_res_set_regmatname(ur, regmatname);
                utt_res_set_cb2mllrname(ur, cb2mllrname);
            }
            (*func) (kb, ur, sf, ef, uttid);
        }
        ptmr_stop(&tm);

        E_INFO
            ("%s: %6.1f sec CPU, %6.1f sec Clk;  TOT: %8.1f sec CPU, %8.1f sec Clk\n\n",
             uttid, tm.t_cpu, tm.t_elapsed, tm.t_tot_cpu,
             tm.t_tot_elapsed);


        ptmr_reset(&tm);
    }

    if (fp)
        fclose(fp);
    if (ctllmfp)
        fclose(ctllmfp);
    if (ctlmllrfp)
        fclose(ctlmllrfp);
    if (ur)
        ckd_free(ur);

    return tm;
}



ptmr_t
ctl_process_utt(const char *uttfile, int32 count,
                void (*func) (void *kb, utt_res_t * ur, int32 sf, int32 ef,
                              char *uttid), void *kb)
{
    char utterance_file[16384];
    char uttid[4096];
    const char *base;
    int32 i, c;
    int32 ts, newts;
    ptmr_t tm;
    utt_res_t *ur;


    ptmr_init(&tm);
    ur = new_utt_res();
    base = path2basename(uttfile);
    /* strip_fileext() copies base to uttid. So, copying uttid to base
     *  is redundant if strip_fileext() is not called.
     */
    /*
       strip_fileext (base, uttid);
       strcpy (base, uttid);
     */

    ts = -1;
    for (c = 0; c < count; c++) {
        /* Wait for uttfile to change from previous iteration */
        for (i = 0;; i++) {
            newts = stat_mtime(uttfile);
            if ((newts >= 0) && (newts != ts))
                break;

            if (i == 0)
                E_INFO("Waiting for %s, count %d, c %d\n", uttfile, count,
                       c);
            SLEEP_SEC(1);
        }
        ts = newts;

        /* Form uttid */
        sprintf(uttid, "%s_%08d", base, c);
        strncpy(utterance_file, uttfile, sizeof(utterance_file) - 1);
        utterance_file[sizeof(utterance_file) - 1] = 0;

        /* Process this utterance */
        ptmr_start(&tm);
        if (func) {
            utt_res_set_uttfile(ur, utterance_file);

            (*func) (kb, ur, 0, -1, uttid);
        }
        ptmr_stop(&tm);

        E_INFO
            ("%s: %6.1f sec CPU, %6.1f sec Clk;  TOT: %8.1f sec CPU, %8.1f sec Clk\n\n",
             uttid, tm.t_cpu, tm.t_elapsed, tm.t_tot_cpu,
             tm.t_tot_elapsed);

        ptmr_reset(&tm);
    }

    if (ur)
        free_utt_res(ur);
    return tm;
}


void
ctl_infile(char *file, const char *dir, const char *ext, const char *utt)
{
    int32 l1, l2;

    assert(utt);

    if (ext && (ext[0] != '\0')) {
        l1 = strlen(ext);
        l2 = strlen(utt);
        if ((l2 > l1) && (utt[l2 - l1 - 1] == '.')
            && (strcmp(utt + (l2 - l1), ext) == 0))
            ext = NULL;         /* utt already has the desired extension */
    }

    if ((utt[0] != '/') && dir) {
        /* Dir specified for relative uttfile pathname */
        if (ext && (ext[0] != '\0'))
            sprintf(file, "%s/%s.%s", dir, utt, ext);
        else
            sprintf(file, "%s/%s", dir, utt);
    }
    else {
        if (ext && (ext[0] != '\0'))
            sprintf(file, "%s.%s", utt, ext);
        else
            strcpy(file, utt);
    }
}


void
ctl_outfile(char *file, const char *dir, const char *ext,
	    const char *utt, const char *uttid, int build_dirs)
{
    int32 k;

    k = strlen(dir);

    if ((k > 4) && (strcmp(dir + k - 4, ",CTL") == 0)) {        /* HACK!! Hardwired ,CTL */
        if (utt[0] != '/') {
            strcpy(file, dir);
            file[k - 4] = '/';
            strcpy(file + k - 3, utt);
        }
        else
            strcpy(file, utt);
    }
    else {
        strcpy(file, dir);
        file[k] = '/';
        strcpy(file + k + 1, uttid);
    }

    if (ext && (ext[0] != '\0')) {
        strcat(file, ".");
        strcat(file, ext);
    }

    /* Build output directory structure if possible/requested */
    if (build_dirs) {
        char *dirname = ckd_salloc(file);
        path2dirname(file, dirname);
        build_directory(dirname);
        ckd_free(dirname);
    }
}
