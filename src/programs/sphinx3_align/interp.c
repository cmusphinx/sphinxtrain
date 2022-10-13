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
/*
 * interp.c -- CD-senone and CI-senone score interpolation
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.4  2005/06/21  18:39:03  arthchan2003
 * Log. 1, Fixed doxygen documentation, 2, Added $Log$ keyword
 * 
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 27-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporate it from s3 code base. 
 *
 * 20-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed interp_read to use the new libio/bio_fread functions.
 * 
 * 05-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <string.h>
#include <assert.h>

#include <sphinxbase/bio.h>

#include "interp.h"
#include "logs3.h"

#define INTERP_VERSION "1.0"


static int32
interp_read(interp_t * ip, const char *file_name)
{
    FILE *fp;
    int32 byteswap, chksum_present;
    int32 i;
    char eofchk;
    float f;
    char **argname, **argval;
    uint32 chksum;

    E_INFO("Reading interpolation weights: %s\n", file_name);

    if ((fp = fopen(file_name, "rb")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);

    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr(fp, &argname, &argval, &byteswap) < 0)
        E_FATAL("bio_readhdr(%s) failed\n", file_name);

    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
        if (strcmp(argname[i], "version") == 0) {
            if (strcmp(argval[i], INTERP_VERSION) != 0)
                E_WARN("Version mismatch(%s): %s, expecting %s\n",
                       file_name, argval[i], INTERP_VERSION);
        }
        else if (strcmp(argname[i], "chksum0") == 0) {
            chksum_present = 1; /* Ignore the associated value */
        }
    }
    bio_hdrarg_free(argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #senones */
    if (bio_fread(&(ip->n_sen), sizeof(int32), 1, fp, byteswap, &chksum) !=
        1)
        E_FATAL("fread(%s) (arraysize) failed\n", file_name);
    if (ip->n_sen <= 0)
        E_FATAL("%s: arraysize= %d in header\n", file_name, ip->n_sen);

    ip->wt =
        (struct interp_wt_s *) ckd_calloc(ip->n_sen,
                                          sizeof(struct interp_wt_s));

    for (i = 0; i < ip->n_sen; i++) {
        if (bio_fread(&f, sizeof(float32), 1, fp, byteswap, &chksum) != 1)
            E_FATAL("fread(%s) (arraydata) failed\n", file_name);
        if ((f < 0.0) || (f > 1.0))
            E_FATAL("%s: interpolation weight(%d)= %e\n", file_name, i, f);

        ip->wt[i].cd = (f == 0.0) ? S3_LOGPROB_ZERO : logs3(ip->logmath, f);
        ip->wt[i].ci = (f == 1.0) ? S3_LOGPROB_ZERO : logs3(ip->logmath, 1.0 - f);
    }

    if (chksum_present)
        bio_verify_chksum(fp, byteswap, chksum);

    if (fread(&eofchk, 1, 1, fp) == 1)
        E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read %d interpolation weights\n", ip->n_sen);

    return 1;
}


interp_t *
interp_init(const char *file, logmath_t *logmath)
{
    interp_t *ip;

    assert(file != NULL);

    ip = (interp_t *) ckd_calloc(1, sizeof(interp_t));
    ip->logmath = logmath;

    if (interp_read(ip, file) != 1)
        E_FATAL("interp_init(%s) failed\n", file);

    return ip;
}


int32
interp_cd_ci(interp_t * ip, int32 * senscr, int32 cd, int32 ci)
{
    assert((ci >= 0) && (ci < ip->n_sen));
    assert((cd >= 0) && (cd < ip->n_sen));

    senscr[cd] = logmath_add(ip->logmath,
                             senscr[cd] + ip->wt[cd].cd,
                             senscr[ci] + ip->wt[cd].ci);

    return 0;
}


int32
interp_all(interp_t * ip, int32 * senscr, s3senid_t * cimap,
           int32 n_ci_sen)
{
    int32 ci, cd;

    assert(n_ci_sen <= ip->n_sen);

    for (cd = n_ci_sen; cd < ip->n_sen; cd++) {
        ci = cimap[cd];
        senscr[cd] = logmath_add(ip->logmath,
                                 senscr[cd] + ip->wt[cd].cd,
                                 senscr[ci] + ip->wt[cd].ci);
    }

    return 0;
}
