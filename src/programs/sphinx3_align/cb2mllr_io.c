/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * File: s3cb2mllr_io.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/bio.h>
#include <sphinxbase/err.h>

#include "cb2mllr_io.h"

#define IS_MAPPED	1
#define NOT_MAPPED	0
#define MAP_FILE_VERSION	"1.2"

/**
   Reading a regression map
 */
static int
s3map_read(const char *fn, /**< The file name */
           void **out_map,
           uint32 * out_n_dom, uint32 * out_n_rng, size_t map_elem_size)
{
    uint32 rd_chksum = 0;
    uint32 sv_chksum;
    uint32 ignore;
    char *ver;
    char *do_chk;
    FILE *fp;
    int32 swap;
    char **argname, **argval;
    int i;

    if ((fp = fopen(fn, "rb")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,rb) failed\n", fn);

    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr(fp, &argname, &argval, &swap) < 0)
        E_FATAL("bio_readhdr(%s) failed\n", fn);

    /* Parse argument-value list */
    ver = do_chk = NULL;
    for (i = 0; argname[i]; i++) {
        if (strcmp(argname[i], "version") == 0) {
            if (strcmp(argval[i], MAP_FILE_VERSION) != 0) {
                E_FATAL("Version mismatch(%s): %s, expecting %s\n",
                        fn, argval[i], MAP_FILE_VERSION);
            }
            ver = argval[i];
        }
        else if (strcmp(argname[i], "chksum0") == 0) {
            do_chk = argval[i];
        }
    }
    if (ver == NULL)
        E_FATAL("No version attribute for %s\n", fn);
    bio_hdrarg_free(argname, argval);
    argname = argval = NULL;

    if (bio_fread(out_n_rng, sizeof(uint32), 1, fp, swap, &rd_chksum) != 1) {
        fclose(fp);

        return S3_ERROR;
    }

    if (bio_fread_1d(out_map,
                     map_elem_size, out_n_dom, fp, swap, &rd_chksum) < 0) {
        fclose(fp);

        return S3_ERROR;
    }

    if (do_chk) {
        if (bio_fread(&sv_chksum, sizeof(uint32), 1, fp, swap, &ignore) !=
            1) {
            fclose(fp);

            return S3_ERROR;
        }

        if (sv_chksum != rd_chksum) {
            E_FATAL("Checksum error; read corrupted data.\n");
        }
    }

    E_INFO("Read %s [%u mappings to %u]\n", fn, *out_n_dom, *out_n_rng);

    return S3_SUCCESS;
}

int
cb2mllr_read(const char *fn,
             int32 ** out_cb2mllr, uint32 * out_n_cb, uint32 * out_n_mllr)
{
    int ret, i, n_d, n_r;
    int *did_map;
    int32 *cb2mllr;
    int in_hole = FALSE;
    int beg_hole = 0;

    ret = s3map_read(fn,
                     (void **) out_cb2mllr,
                     out_n_cb, out_n_mllr, sizeof(int32));


    if (ret == S3_SUCCESS) {
        n_d = *out_n_cb;
        n_r = *out_n_mllr;
        cb2mllr = *out_cb2mllr;

        did_map = ckd_calloc(n_r, sizeof(int));

        for (i = 0; i < n_d; i++) {
            if (cb2mllr[i] >= (int32) * out_n_mllr) {
                E_FATAL
                    ("%s cb2mllr[%d] -> %d which is >= n_mllr_class (%d)\n",
                     fn, i, cb2mllr[i], *out_n_mllr);
            }
            else if (cb2mllr[i] >= 0)
                did_map[cb2mllr[i]] = IS_MAPPED;
        }

        for (i = 0; i < n_d; i++) {
            if (cb2mllr[i] < 0)
                continue;       /* skipped */
            if (!in_hole && (did_map[cb2mllr[i]] == NOT_MAPPED)) {
                beg_hole = i;
                in_hole = TRUE;
            }
            if (in_hole && (did_map[cb2mllr[i]] == IS_MAPPED)) {
                E_FATAL("cb unmapped in region [%u %u]\n", beg_hole,
                        i - 1);
                in_hole = FALSE;
            }
        }
        if (in_hole) {
            E_FATAL("cb unmapped in region [%u %u]\n", beg_hole, i - 1);
        }

        ckd_free(did_map);
    }

    return ret;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2006/02/22  15:02:47  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Follow Dave's change to make uint32 to int32. 2, Fixed some dox-doc issue.
 * 
 *
 * Revision 1.3  2005/10/05 00:29:44  dhdfu
 * Silence some largely unimportant signedness warnings.
 *
 * Revision 1.2  2005/06/22 03:29:35  arthchan2003
 * Makefile.am s  for all subdirectory of libs3decoder/
 *
 * Revision 1.1  2005/06/15 19:41:02  archan
 * Added cb2mllr_io into libam
 *
 * Revision 1.1  2005/04/15 14:16:51  dhdfu
 * Additional MLLR runtime support, multi-class (1 stream only) MLLR runtime support, regression and performance tests
 *
 * Revision 1.5  2005/01/18 19:10:06  dhdfu
 * Fix signedness issues in multi-class MLLR code, incorporate extra error checking from original Sam-Joo Doh code
 *
 * Revision 1.4  2004/07/21 18:05:40  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
