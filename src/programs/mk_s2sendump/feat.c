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
/*
 * feat.c -- Feature vector description and cepstra->feature computation.
 *
 * HISTORY
 * 
 * 12-Jun-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Major changes to accommodate arbitrary feature input types.  Added
 * 		feat_read(), moved various cep2feat functions from other files into
 *		this one.  Also, made this module object-oriented with the feat_t type.
 * 		Changed definition of s2mfc_read to let the caller manage MFC buffers.
 * 
 * 03-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added unistd.h include.
 * 
 * 02-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check for sf argument to s2mfc_read being within file size.
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added sf, ef parameters to s2mfc_read().
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added feat_cepsize().
 * 		Added different feature-handling (s2_4x, s3_1x39 at this point).
 * 		Moved feature-dependent functions to feature-dependent files.
 * 
 * 09-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Moved constant declarations from feat.h into here.
 * 
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


/*
 * This module encapsulates different feature streams used by the Sphinx group.  New
 * stream types can be added by augmenting feat_init() and providing an accompanying
 * compute_feat function.  It also provides a "generic" feature vector definition for
 * handling "arbitrary" speech input feature types (see the last section in feat_init()).
 * In this case the speech input data should already be feature vectors; no computation,
 * such as MFC->feature conversion, is available or needed.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#if (_SUN4)
#include <unistd.h>
#endif
#include <string.h>
#include <assert.h>

#if (! WIN32)
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/param.h>
#else
#include <fcntl.h>
#endif

#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s2_read_map.h>
#include <s3/s2_write_seno.h>

#include <s3/cmd_ln.h>

#include "s3/hash.h"
typedef hash_t hash_table_t;
#include "s3types.h"
#include "another_s3types.h"
#include "another_mdef.h"
#include "another_senone.h"
#include "s3types.h"
#include "bio.h"
#include "feat.h"

#define FEAT_VERSION	"1.0"


int32 feat_readfile (feat_t *fcb, char *file, int32 sf, int32 ef,
		     float32 ***feat, int32 maxfr)
{
    FILE *fp;
    int32 i, l, k, nfr;
    int32 byteswap, chksum_present;
    uint32 chksum;
    char **argname, **argval;
    
    if ((sf == 0) && (ef < 0))
	E_INFO("Reading feature file: '%s'\n", file);
    else
	E_INFO("Reading feature file: '%s'[%d..%d]\n", file, sf, ef);
    if ((ef <= sf) && ((sf != 0) || (ef >= 0))) {
	E_ERROR("%s: End frame (%d) <= Start frame (%d)\n", file, ef, sf);
	return -1;
    }
    assert (fcb);
    
    if ((fp = fopen(file, "rb")) == NULL) {
	E_ERROR("fopen(%s,rb) failed\n", file);
	return -1;
    }
    
    /* Read header */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0) {
	E_ERROR("bio_readhdr(%s) failed\n", file);
	fclose (fp);
	return -1;
    }
    
    /* Parse header info (although nothing much is done with it) */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], FEAT_VERSION) != 0)
		E_WARN("%s: Version mismatch: %s, expecting %s\n",
		       file, argval[i], FEAT_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }

    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;
    
    chksum = 0;
    
    /* #Frames */
    if (bio_fread (&nfr, sizeof(int32), 1, fp, byteswap, &chksum) != 1) {
	E_ERROR("%s: fread(#frames) failed\n", file);
	fclose (fp);
	return -1;
    }
    
    /* #Feature streams */
    if ((bio_fread (&l, sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(l != feat_n_stream(fcb))) {
	E_ERROR("%s: Missing or bad #feature streams\n", file);
	fclose (fp);
	return -1;
    }
    
    /* Feature stream lengths */
    k = 0;
    for (i = 0; i < feat_n_stream(fcb); i++) {
	if ((bio_fread (&l, sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	    (l != feat_stream_len (fcb, i))) {
	    E_ERROR("%s: Missing or bad feature stream size\n", file);
	    fclose (fp);
	    return -1;
	}
	k += l;
    }
    
    /* Set endframe if not specified */
    if (ef < 0)
	ef = nfr-1;
    else if (ef >= nfr) {
	E_ERROR("%s: End frame spec (%d) beyond EOF (%d frames); limited to EOF\n",
		file, ef, nfr);
	ef = nfr-1;
    }

    if ((sf < 0) || (sf >= nfr) || (ef < 0) || (ef < sf)) {
	E_ERROR("%s: Start/end frame spec (%d,%d) out of file size range (%d)\n",
		file, sf, ef, nfr);
	fclose (fp);
	return -1;
    }
    
    /* Check sf/ef specified */
    if (sf > 0) {
	if (sf >= nfr) {
	    E_ERROR("%s: Start frame (%d) beyond file size (%d)\n", file, sf, nfr);
	    fclose (fp);
	    return -1;
	}
	nfr -= sf;
    }
    
    /* Limit nfr as indicated by [sf..ef] */
    if ((ef-sf+1) < nfr)
	nfr = (ef-sf+1);
    if (nfr > maxfr) {
	E_ERROR("%s: Feature buffer size(%d) too small; %d required\n", file, maxfr, nfr);
	fclose (fp);
	return -1;
    }
    
    /* Position at desired start frame and read feature data */
    if (sf > 0)
	fseek (fp, sf * k * sizeof(float32), SEEK_CUR);
    if (bio_fread (feat[0][0], sizeof(float32), nfr*k, fp, byteswap, &chksum) != nfr*k) {
	E_ERROR("%s: fread(%dx%d) (feature data) failed\n", file, nfr, k);
	fclose (fp);
	return -1;
    }
    
    fclose (fp);	/* NOTE: checksum NOT verified; we might read only part of file */
    
    return nfr;
}


int32 feat_writefile (feat_t *fcb, char *file, float32 ***feat, int32 nfr)
{
    FILE *fp;
    int32 i, k;
    
    E_INFO ("Writing feature file: '%s'\n", file);
    assert (fcb);
    
    if ((fp = fopen (file, "wb")) == NULL) {
	E_ERROR("fopen(%s,wb) failed\n", file);
	return -1;
    }
    
    /* Write header */
    bio_writehdr_version (fp, FEAT_VERSION);
    
    fwrite (&nfr, sizeof(int32), 1, fp);
    fwrite (&(fcb->n_stream), sizeof(int32), 1, fp);
    k = 0;
    for (i = 0; i < feat_n_stream(fcb); i++) {
	fwrite (&(fcb->stream_len[i]), sizeof(int32), 1, fp);
	k += feat_stream_len(fcb, i);
    }
    
    /* Feature data is assumed to be in a single block, starting at feat[0][0][0] */
    if (fwrite (feat[0][0], sizeof(float32), nfr*k, fp) != (size_t)nfr*k) {
	E_ERROR("%s: fwrite(%dx%d feature data) failed\n", file, nfr, k);
	fclose (fp);
	return -1;
    }
    
    fclose (fp);
    
    return 0;
}


#define FREAD_RETRY_COUNT	60

int32 fread_retry(void *pointer, int32 size, int32 num_items, FILE *stream)
{
    char *data;
    uint32 n_items_read;
    uint32 n_items_rem;
    uint32 n_retry_rem;
    int32 loc;

    n_retry_rem = FREAD_RETRY_COUNT;

    data = pointer;
    loc = 0;
    n_items_rem = num_items;
	
    do {
	n_items_read = fread(&data[loc], size, n_items_rem, stream);

	n_items_rem -= n_items_read;

	if (n_items_rem > 0) {
	    /* an incomplete read occurred */

	    if (n_retry_rem == 0)
		return -1;

	    if (n_retry_rem == FREAD_RETRY_COUNT) {
		E_ERROR_SYSTEM("fread() failed; retrying...\n");
	    }

	    --n_retry_rem;
	    
	    loc += n_items_read * size;
#if (! WIN32)
	    sleep(1);
#endif
	}
    } while (n_items_rem > 0);

    return num_items;
}


#define STAT_RETRY_COUNT	10

int32 stat_retry (char *file, struct stat *statbuf)
{
    int32 i;
    
    for (i = 0; i < STAT_RETRY_COUNT; i++) {
	if (stat (file, statbuf) == 0)
	    return 0;
	
	if (i == 0) {
	    E_ERROR_SYSTEM("stat(%s) failed; retrying...\n", file);
	}
#if (! WIN32)
	sleep (10);
#endif
    }
    
    return -1;
}


/*
 * Read specified segment [sf..ef] of Sphinx-II format mfc file read and return
 * #frames read.  Return -1 if error.
 */
int32 s2mfc_read (char *file, int32 sf, int32 ef, int32 featpad,
		  float32 **mfc, int32 maxfr)
{
    FILE *fp;
    int32 n_float32;
    struct stat statbuf;
    int32 i, n, byterev, cepsize, spad, epad, n_read;
    
    if ((sf == 0) && (ef < 0))
	E_INFO("Reading MFC file: '%s'\n", file);
    else
	E_INFO("Reading MFC file: '%s'[%d..%d]\n", file, sf, ef);
    if ((ef <= sf) && ((sf != 0) || (ef >= 0))) {
	E_ERROR("%s: End frame (%d) <= Start frame (%d)\n", file, ef, sf);
	return -1;
    }
    
    cepsize = 13;	/* Hack!! hardwired constant */

    /* Find filesize; HACK!! To get around intermittent NFS failures, use stat_retry */
    if ((stat_retry (file, &statbuf) < 0) || ((fp = fopen(file, "rb")) == NULL)) {
	E_ERROR("stat_retry/fopen(%s) failed\n", file);
	return -1;
    }
    
    /* Read #floats in header */
    if (fread_retry (&n_float32, sizeof(int32), 1, fp) != 1) {
	E_ERROR("%s: fread(#floats) failed\n", file);
	fclose (fp);
	return -1;
    }
    
    /* Check if n_float32 matches file size */
    byterev = FALSE;
    if ((n_float32*sizeof(float32) + 4) != (size_t)statbuf.st_size) {
	n = n_float32;
	SWAP_INT32(&n);

	if ((n*sizeof(float32) + 4) != (size_t)statbuf.st_size) {
	    E_ERROR("%s: Header size field: %d(%08x); filesize: %d(%08x)\n",
		    file, n_float32, n_float32, statbuf.st_size, statbuf.st_size);
	    fclose (fp);
	    return -1;
	}
	
	n_float32 = n;
	byterev = TRUE;
    }
    if (n_float32 <= 0) {
	E_ERROR("%s: Header size field (#floats) = %d\n", file, n_float32);
	fclose (fp);
	return -1;
    }
    
    /* Convert n to #frames of input */
    n = n_float32/cepsize;
    if (n * cepsize != n_float32) {
	E_ERROR("%s: Header size field: %d; not multiple of %d\n",
		file, n_float32, cepsize);
	fclose (fp);
	return -1;
    }
    
    /* Set endframe if not specified */
    if (ef < 0)
	ef = n-1;
    else if (ef >= n) {
	E_ERROR("%s: End frame spec (%d) beyond EOF (%d frames); limited to EOF\n",
		file, ef, n);
	ef = n-1;
    }

    if ((sf < 0) || (sf >= n) || (ef < 0) || (ef < sf)) {
	E_ERROR("%s: Start/end frame spec (%d,%d) out of file size range (%d)\n",
		file, sf, ef, n);
	fclose (fp);
	return -1;
    }

    /* Account for the necessary padding */
    sf -= featpad;
    ef += featpad;
    n_read = ef - sf + 1;
    if (n_read > maxfr) {
	E_ERROR("%s: MFC buffer size (%d) too small; %d required\n", file, maxfr, n_read);
	fclose (fp);
	return -1;
    }

    if (sf < 0) {
	spad = -sf;
	sf = 0;
    } else
	spad = 0;
    if (ef >= n) {
	epad = ef - (n-1);
	ef = n-1;
    } else
	epad = 0;

    /* Position at desired start frame and read MFC data */
    if (sf > 0)
	fseek (fp, sf * cepsize * sizeof(float32), SEEK_CUR);
    n_float32 = (ef-sf+1) * cepsize;
    if ((i = fread (mfc[spad], sizeof(float32), n_float32, fp)) != n_float32) {
	E_ERROR("%s: fread(%d,%d) returned %d\n", file, sizeof(float32), n_float32, i);
	fclose (fp);
	return -1;
    }
    if (byterev) {
	for (i = 0; i < n_float32; i++)
	    SWAP_FLOAT32(&(mfc[spad][i]));
    }
    fclose (fp);
    
    /* Add padding that could not be obtained from the MFC file itself */
    if (spad > 0) {
	for (i = 0; i < spad; i++)
	    memcpy ((void *)(mfc[i]), (void *)(mfc[spad]), cepsize*sizeof(float32));
    }
    if (epad > 0) {
	for (i = 0; i < epad; i++)
	    memcpy ((void *)(mfc[n_read-1-i]), (void *)(mfc[n_read-1-epad]),
		    cepsize*sizeof(float32));
    }
    
    return n_read;
}


static int32 feat_stream_len_sum (feat_t *fcb)
{
    int32 i, k;
    
    k = 0;
    for (i = 0; i < feat_n_stream(fcb); i++)
	k += feat_stream_len (fcb, i);
    return k;
}


float32 **feat_vector_alloc (feat_t *fcb)
{
    int32 i, k;
    float32 *data, **feat;
    
    assert (fcb);
    
    if ((k = feat_stream_len_sum(fcb)) <= 0) {
	E_ERROR("Sum(feature stream lengths) = %d\n", k);
	return NULL;
    }
    
    /* Allocate feature data array so that data is in one block from feat[0][0] */
    feat = (float32 **) ckd_calloc (feat_n_stream(fcb), sizeof(float32 *));
    data = (float32 *) ckd_calloc (k, sizeof(float32));
    
    for (i = 0; i < feat_n_stream(fcb); i++) {
	feat[i] = data;
	data += feat_stream_len (fcb, i);
    }
    
    return feat;
}


float32 ***feat_array_alloc (feat_t *fcb, int32 nfr)
{
    int32 i, j, k;
    float32 *data, ***feat;
    
    assert (fcb);
    assert (nfr > 0);
    
    if ((k = feat_stream_len_sum(fcb)) <= 0) {
	E_ERROR("Sum(feature stream lengths) = %d\n", k);
	return NULL;
    }
    
    /* Allocate feature data array so that data is in one block from feat[0][0][0] */
    feat = (float32 ***) ckd_calloc_2d (nfr, feat_n_stream(fcb), sizeof(float32 *));
    data = (float32 *) ckd_calloc (nfr*k, sizeof(float32));
    
    for (i = 0; i < nfr; i++) {
	for (j = 0; j < feat_n_stream(fcb); j++) {
	    feat[i][j] = data;
	    data += feat_stream_len (fcb, j);
	}
    }
    
    return feat;
}


static void feat_s2_4x_cep2feat (feat_t *fcb, float32 **mfc, float32 **feat)
{
    float32 *f;
    float32 *w, *_w;
    float32 *w1, *w_1, *_w1, *_w_1;
    float32 d1, d2;
    int32 i, j;
    
    assert (fcb);
    assert (feat_cepsize (fcb) == 13);
    assert (feat_cepsize_used (fcb) == 13);
    assert (feat_n_stream (fcb) == 4);
    assert (feat_stream_len (fcb, 0) == 12);
    assert (feat_stream_len (fcb, 1) == 24);
    assert (feat_stream_len (fcb, 2) == 3);
    assert (feat_stream_len (fcb, 3) == 12);
    assert (feat_window_size (fcb) == 4);
    
    /* CEP; skip C0 */
    memcpy (feat[0], mfc[0]+1, (feat_cepsize(fcb)-1) * sizeof(float32));
    
    /*
     * DCEP(SHORT): mfc[2] - mfc[-2]
     * DCEP(LONG):  mfc[4] - mfc[-4]
     */
    w  = mfc[2] + 1;	/* +1 to skip C0 */
    _w = mfc[-2] + 1;

    f = feat[1];
    for (i = 0; i < feat_cepsize(fcb)-1; i++)	/* Short-term */
	f[i] = w[i] - _w[i];

    w  = mfc[4] + 1;	/* +1 to skip C0 */
    _w = mfc[-4] + 1;

    for (j = 0; j < feat_cepsize(fcb)-1; i++, j++)	/* Long-term */
	f[i] = w[j] - _w[j];

    /* D2CEP: (mfc[3] - mfc[-1]) - (mfc[1] - mfc[-3]) */
    w1   = mfc[3] + 1;	/* Final +1 to skip C0 */
    _w1  = mfc[-1] + 1;
    w_1  = mfc[1] + 1;
    _w_1 = mfc[-3] + 1;

    f = feat[3];
    for (i = 0; i < feat_cepsize(fcb)-1; i++) {
	d1 =  w1[i] -  _w1[i];
	d2 = w_1[i] - _w_1[i];

	f[i] = d1 - d2;
    }
    
    /* POW: C0, DC0, D2C0; differences computed as above for rest of cep */
    f = feat[2];
    f[0] = mfc[0][0];
    f[1] = mfc[2][0] - mfc[-2][0];

    d1 = mfc[3][0] - mfc[-1][0];
    d2 = mfc[1][0] - mfc[-3][0];
    f[2] = d1 - d2;
}


static void feat_s3_1x39_cep2feat (feat_t *fcb, float32 **mfc, float32 **feat)
{
    float32 *f;
    float32 *w, *_w;
    float32 *w1, *w_1, *_w1, *_w_1;
    float32 d1, d2;
    int32 i;
    
    assert (fcb);
    assert (feat_cepsize (fcb) == 13);
    assert (feat_cepsize_used (fcb) == 13);
    assert (feat_n_stream (fcb) == 1);
    assert (feat_stream_len (fcb, 0) == 39);
    assert (feat_window_size (fcb) == 3);
    
    /* CEP; skip C0 */
    memcpy (feat[0], mfc[0]+1, (feat_cepsize(fcb)-1) * sizeof(float32));
    
    /*
     * DCEP: mfc[2] - mfc[-2];
     */
    f = feat[0] + feat_cepsize(fcb)-1;
    w  = mfc[2] + 1;	/* +1 to skip C0 */
    _w = mfc[-2] + 1;

    for (i = 0; i < feat_cepsize(fcb)-1; i++)
	f[i] = w[i] - _w[i];
    
    /* POW: C0, DC0, D2C0 */
    f += feat_cepsize(fcb)-1;

    f[0] = mfc[0][0];
    f[1] = mfc[2][0] - mfc[-2][0];

    d1 = mfc[3][0] - mfc[-1][0];
    d2 = mfc[1][0] - mfc[-3][0];
    f[2] = d1 - d2;

    /* D2CEP: (mfc[3] - mfc[-1]) - (mfc[1] - mfc[-3]) */
    f += 3;
    
    w1   = mfc[3] + 1;	/* Final +1 to skip C0 */
    _w1  = mfc[-1] + 1;
    w_1  = mfc[1] + 1;
    _w_1 = mfc[-3] + 1;

    for (i = 0; i < feat_cepsize(fcb)-1; i++) {
	d1 =  w1[i] -  _w1[i];
	d2 = w_1[i] - _w_1[i];

	f[i] = d1 - d2;
    }
}


static void feat_s3_cep (feat_t *fcb, float32 **mfc, float32 **feat)
{
    assert (fcb);
    assert (feat_cepsize (fcb) == 13);
    assert ((feat_cepsize_used (fcb) <= 13) && (feat_cepsize_used(fcb) > 0));
    assert (feat_n_stream (fcb) == 1);
    assert (feat_stream_len (fcb, 0) == feat_cepsize_used(fcb));
    assert (feat_window_size (fcb) == 0);
    
    /* CEP */
    memcpy (feat[0], mfc[0], feat_cepsize_used(fcb) * sizeof(float32));
}


static void feat_s3_cep_dcep (feat_t *fcb, float32 **mfc, float32 **feat)
{
    float32 *f;
    float32 *w, *_w;
    int32 i;
    
    assert (fcb);
    assert (feat_cepsize (fcb) == 13);
    assert ((feat_cepsize_used (fcb) <= 13) && (feat_cepsize_used(fcb) > 0));
    assert (feat_n_stream (fcb) == 1);
    assert (feat_stream_len (fcb, 0) == (feat_cepsize_used(fcb) * 2));
    assert (feat_window_size (fcb) == 2);
    
    /* CEP */
    memcpy (feat[0], mfc[0], feat_cepsize_used(fcb) * sizeof(float32));
    
    /*
     * DCEP: mfc[2] - mfc[-2];
     */
    f = feat[0] + feat_cepsize_used(fcb);
    w  = mfc[2];
    _w = mfc[-2];

    for (i = 0; i < feat_cepsize_used(fcb); i++)
	f[i] = w[i] - _w[i];
}


feat_t *feat_init (char *type)
{
    feat_t *fcb;
    int32 i, l, k;
    char wd[16384], *strp;
    
    E_INFO("Initializing feature stream to type: %s\n", type);
    
    fcb = (feat_t *) ckd_calloc (1, sizeof(feat_t));
    
    fcb->name = (char *) ckd_salloc (type);
    if (strcmp (type, "s2_4x") == 0) {
	/* Sphinx-II format 4-stream feature (Hack!! hardwired constants below) */
	fcb->cepsize = 13;
	fcb->cepsize_used = 13;
	fcb->n_stream = 4;
	fcb->stream_len = (int32 *) ckd_calloc (4, sizeof(int32));
	fcb->stream_len[0] = 12;
	fcb->stream_len[1] = 24;
	fcb->stream_len[2] = 3;
	fcb->stream_len[3] = 12;
	fcb->window_size = 4;
	fcb->compute_feat = feat_s2_4x_cep2feat;
    } else if (strcmp (type, "s3_1x39") == 0) {
	/* 1-stream cep/dcep/pow/ddcep (Hack!! hardwired constants below) */
	fcb->cepsize = 13;
	fcb->cepsize_used = 13;
	fcb->n_stream = 1;
	fcb->stream_len = (int32 *) ckd_calloc (1, sizeof(int32));
	fcb->stream_len[0] = 39;
	fcb->window_size = 3;
	fcb->compute_feat = feat_s3_1x39_cep2feat;
    } else if (strncmp (type, "cep_dcep", 8) == 0) {
	/* 1-stream cep/dcep (Hack!! hardwired constants below) */
	fcb->cepsize = 13;
	/* Check if using only a portion of cep dimensions */
	if (type[8] == ',') {
	    if ((sscanf (type+9, "%d%n", &(fcb->cepsize_used), &l) != 1) ||
		(type[l+9] != '\0') ||
		(feat_cepsize_used(fcb) <= 0) ||
		(feat_cepsize_used(fcb) > feat_cepsize(fcb)))
		E_FATAL("Bad feature type argument: '%s'\n", type);
	} else
	    fcb->cepsize_used = 13;
	fcb->n_stream = 1;
	fcb->stream_len = (int32 *) ckd_calloc (1, sizeof(int32));
	fcb->stream_len[0] = feat_cepsize_used(fcb) * 2;
	fcb->window_size = 2;
	fcb->compute_feat = feat_s3_cep_dcep;
    } else if (strncmp (type, "cep", 3) == 0) {
	/* 1-stream cep (Hack!! hardwired constants below) */
	fcb->cepsize = 13;
	/* Check if using only a portion of cep dimensions */
	if (type[3] == ',') {
	    if ((sscanf (type+4, "%d%n", &(fcb->cepsize_used), &l) != 1) ||
		(type[l+4] != '\0') ||
		(feat_cepsize_used(fcb) <= 0) ||
		(feat_cepsize_used(fcb) > feat_cepsize(fcb)))
		E_FATAL("Bad feature type argument: '%s'\n", type);
	} else
	    fcb->cepsize_used = 13;
	fcb->n_stream = 1;
	fcb->stream_len = (int32 *) ckd_calloc (1, sizeof(int32));
	fcb->stream_len[0] = feat_cepsize_used(fcb);
	fcb->window_size = 0;
	fcb->compute_feat = feat_s3_cep;
    } else {
	/*
	 * Generic definition: Format should be %d,%d,%d,...,%d (i.e., comma separated
	 * list of feature stream widths; #items = #streams).
	 */
	l = strlen(type);
	k = 0;
	for (i = 1; i < l-1; i++)
	    if (type[i] == ',') {
		type[i] = ' ';
		k++;
	    }
	k++;	/* Presumably there are (#commas+1) streams */
	fcb->n_stream = k;
	fcb->stream_len = (int32 *) ckd_calloc (k, sizeof(int32));

	/* Scan individual feature stream lengths */
	strp = type;
	i = 0;
	while (sscanf (strp, "%s%n", wd, &l) == 1) {
	    strp += l;
	    if ((i >= fcb->n_stream) || (sscanf (wd, "%d", &(fcb->stream_len[i])) != 1) ||
		(fcb->stream_len[i] <= 0))
		E_FATAL("Bad feature type argument\n");
	    i++;
	}
	if (i != fcb->n_stream)
	    E_FATAL("Bad feature type argument\n");
	
	/* Input is already the feature stream */
	fcb->cepsize = -1;
	fcb->cepsize_used = -1;
	fcb->window_size = 0;
	fcb->compute_feat = NULL;
    }
    
    return fcb;
}


