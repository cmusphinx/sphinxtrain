/* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights 
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
 *********************************************************************
 *
 * File: segdmp.c
 * 
 * Description: 
 * 	This routine dumps acoustic data segments according to
 *	state segmentation files.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>
#include <sphinxbase/bio.h>

#include <s3/segdmp.h>
#include <s3/s3io.h>
#include <s3/vector.h>
#include <s3/swap.h>
#include <sys_compat/file.h>
#include <sys_compat/misc.h>		/* for sleep() on _WIN32 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static uint32 n_id;
static uint32 *id_part = NULL;	/* which dmp ID is in */

/* write: current offset for next dmp */
/*  read: offset of first item (segment or frame) */
static uint32 *id_off  = NULL;
static uint32 *id_nxt_off = NULL;

/* total # of items in the dmp (segment or frame) */
static uint32 *id_len  = NULL;

static uint32 n_part = 0;

static uint32 *n_seg = NULL;
static uint32 *nxt_seg = NULL;
static uint32 **n_frame = NULL;

static char dmp_fn[MAXPATHLEN+1];
static char idx_fn[MAXPATHLEN+1];
static FILE **dmp_fp = NULL;
static uint32 *dmp_swp = NULL;
static FILE **idx_fp = NULL;

static const uint32 *vecsize;
static uint32 n_stream;
static uint32 blksize;

static uint32 frame_sz;

static uint32 ignore = 0;
segdmp_type_t cur_type;

/* Max # of bytes in a subpart of a segment dump */
#define DMP_MAX	(uint32)0x7f000000

#define FRM_BUF_SZ (size_t)(200 MEG)

static uint32 frm_buf_sz = FRM_BUF_SZ;

void
segdmp_set_bufsz(uint32 sz_in_mb)
{
    E_INFO("Using %u Mb cache\n", sz_in_mb);

    frm_buf_sz = sz_in_mb MEG;
}

typedef struct seg_struct {
    uint32 len;
    uint32 idx;

    struct seg_struct *next;
} seg_t;

static seg_t **h_seg = NULL;
static seg_t **t_seg = NULL;

static char  *frm_buf = NULL;
static uint32 alloc_frm_buf = 0;
static uint32 nxt_frm_buf = 0;

static int
write_seg_inorder(FILE *fp,
		  seg_t *s)
{
    uint32 l;
    seg_t *cur, *tmp;

    for (cur = s; cur;) {
	l = frame_sz * cur->len;
	if (bio_fwrite((void *)&frm_buf[cur->idx], 1, l, fp, 0, &ignore) != l) {
	    E_ERROR_SYSTEM("Unable to write to dump file\n");
	    
	    return S3_ERROR;
	}

	tmp = cur;
	cur = cur->next;

	ckd_free(tmp);
    }

    return S3_SUCCESS;
}

static int
dump_frm_buf(void);

static int
dump_frm_buf()
{
    uint32 i;
    FILE *fp;
    int ret = S3_SUCCESS;

    fprintf(stderr, " dmp[");
    fflush(stderr);
    for (i = 0; i < n_id; i++) {
	if (!(i % 1000)) {
	    fprintf(stderr, " %u", i);
	    fflush(stderr);
	}
	if (h_seg[i]) {
	    fp = dmp_fp[id_part[i]];

	    if (h_seg[i]) {
		if (fseek(fp, id_off[i], SEEK_SET) < 0) {
		    E_ERROR_SYSTEM("Can't seek in dmp file\n");

		    return S3_ERROR;
		}

		ret = write_seg_inorder(fp, h_seg[i]);
		
		h_seg[i] = t_seg[i] = NULL;

		id_off[i] = ftell(fp);
	    }
	}
    }
    fprintf(stderr, " ]");
    fflush(stderr);

    nxt_frm_buf = 0;

    return ret;
}

static int
read_idx(uint32 part)
{
    uint32 onefrm;
    uint32 i, j, s, n, e;

    if (bio_fread((int *)&cur_type, sizeof(int), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	E_FATAL_SYSTEM("unable to read seg idx file");
    }

    if (bio_fread(&n_id, sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to read index file");

	return S3_ERROR;
    }

    if (bio_fread(&onefrm, sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to read index file");

	return S3_ERROR;
    }

    if (!onefrm && (n_frame == NULL)) {
	n_frame = ckd_calloc(n_id, sizeof(uint32 *));
    }

    if (n_seg == NULL) {
	n_seg = ckd_calloc(n_id, sizeof(uint32));
    }

    if (id_part == NULL) {
	id_part = ckd_calloc(n_id, sizeof(uint32));
	id_off = ckd_calloc(n_id, sizeof(uint32));
	id_len = ckd_calloc(n_id, sizeof(uint32));
    }

    if (bio_fread(&frame_sz, sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to read index file");

	return S3_ERROR;
    }

    if (bio_fread(&s, sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to read index file");

	return S3_ERROR;
    }

    if (bio_fread(&n, sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to read index file");

	return S3_ERROR;
    }

    e = s + n;

    for (i = s; i < e; i++) {
	id_part[i] = part;

	if (bio_fread(&n_seg[i], sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	    E_FATAL_SYSTEM("Unable to read index file");

	    return S3_ERROR;
	}

	if (bio_fread(&id_off[i], sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
	    E_FATAL_SYSTEM("Unable to read index file");
	    
	    return S3_ERROR;
	}

	if (!onefrm) {
	    if (n_seg[i] > 0) {
		n_frame[i] = ckd_calloc(n_seg[i], sizeof(uint32));
	    }
	    else {
		n_frame[i] = NULL;
	    }
	    
	    for (j = 0; j < n_seg[i]; j++) {
		if (bio_fread(&n_frame[i][j], sizeof(uint32), 1, idx_fp[part], dmp_swp[part], &ignore) != 1) {
		    E_FATAL_SYSTEM("Unable to read index file");
		    
		    return S3_ERROR;
		}
	    }
	}
	else {
	    if (n_frame != NULL) {
		ckd_free(n_frame);
		n_frame = NULL;
	    }
	}
    }

    return S3_SUCCESS;
}

static int
write_idx(uint32 part)
{
    uint32 onefrm;
    uint32 i, j, ttt, n;

    if (bio_fwrite(&n_id, sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to write index file");

	return S3_ERROR;
    }

    if (n_frame == NULL)
	onefrm = TRUE;
    else
	onefrm = FALSE;

    /* Write one frame per segment flag */
    if (bio_fwrite(&onefrm, sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to write index file");

	return S3_ERROR;
    }

    if (bio_fwrite(&frame_sz, sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to write index file");

	return S3_ERROR;
    }

    /* Figure out offset (in # of id's) to the part of iterest */
    for (i = 0; (i < n_id) && (id_part[i] != part); i++);

    ttt = i;

    /* Figure out # of id's in this part */
    for (n = 0, j = i; (j < n_id) && (id_part[j] == part); j++, n++);

    i = ttt;

    if (bio_fwrite(&i, sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to write index file");

	return S3_ERROR;
    }
    if (bio_fwrite(&n, sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	E_FATAL_SYSTEM("Unable to write index file");

	return S3_ERROR;
    }

    for (; (i < n_id) && (id_part[i] == part); i++) {
	if (bio_fwrite(&n_seg[i], sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	    E_FATAL_SYSTEM("Unable to write index file");
	    
	    return S3_ERROR;
	}
	if (bio_fwrite(&id_off[i], sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
	    E_FATAL_SYSTEM("Unable to write index file");
	    
	    return S3_ERROR;
	}
	if (!onefrm) {
	    for (j = 0; j < n_seg[i]; j++) {
		if (bio_fwrite(&n_frame[i][j], sizeof(uint32), 1, idx_fp[part], 0, &ignore) != 1) {
		    E_FATAL_SYSTEM("Unable to write index file");
		    
		    return S3_ERROR;
		}
	    }
	}
    }

    return S3_SUCCESS;
}

int
segdmp_open_read(const char **dirs,		/* directories available for dump files */
		 const char *dfn,		/* dump file name */
		 const char *ifn,		/* index file name */
		 uint32 *out_n_id,		/* # of segment id's (i.e. phones, states, etc.)*/
		 segdmp_type_t *out_data_type, /* type of data to store in segment dump */
		 uint32 i_n_stream,
		 uint32* i_vecsize,
		 uint32 i_blksize)	
{
    char fn[MAXPATHLEN+1];
    uint32 n_dir;
    uint32 i, swp;
    uint32 *n_frame;
    
    n_stream = i_n_stream;
    vecsize = i_vecsize;
    blksize = i_blksize;
    
    n_seg = NULL;
    n_frame = NULL;

    for (i = 0; dirs[i]; i++);
    n_dir = i;

    idx_fp = ckd_calloc(n_dir, sizeof(FILE *));
    dmp_fp = ckd_calloc(n_dir, sizeof(FILE *));
    dmp_swp = ckd_calloc(n_dir, sizeof(uint32));

    for (i = 0; i < n_dir; i++) {
	sprintf(fn, "%s/%s", dirs[i], ifn);
	idx_fp[i] = s3open(fn, "rb", &dmp_swp[i]);
	if (idx_fp[i] == NULL) {
	    E_FATAL_SYSTEM("Unable to open index");
	}

	read_idx(i);

	sprintf(fn, "%s/%s", dirs[i], dfn);
	dmp_fp[i] = s3open(fn, "rb", &swp);
	if (dmp_fp[i] == NULL) {
	    E_FATAL_SYSTEM("Unable to open dmp");
	}
	if (swp != dmp_swp[i]) {
	    E_FATAL("Dmp and index assumed to have same byte-order, but they don't\n");
	}
    }

    n_part = n_dir;

    *out_data_type = cur_type;
    *out_n_id = n_id;

    id_nxt_off = ckd_calloc(n_id, sizeof(uint32));
    for (i = 0; i < n_id; i++) {
	id_nxt_off[i] = id_off[i];
    }
    nxt_seg    = ckd_calloc(n_id, sizeof(uint32));

    return S3_SUCCESS;
}
/* Need to do feat_dimension1() for VQ case rather than 4! */
/* Need to test for single ID len > DMP_MAX for all id, not just 1st */


int
segdmp_open_write(const char **dirs,		/* directories available for dump files */
		  const char *dfn,		/* dump file name */
		  const char *ifn,		/* index file name */
		  uint32 n,			/* # of segment id's (i.e. phones, states, etc.)*/
		  uint32 *in_n_seg,		/* # of segments per id */
		  uint32 **in_n_frame,		/* length of each segment in # of frames */
		  segdmp_type_t data_type,	/* type of data to store in segment dump */
		  uint32 i_n_stream,
		  uint32* i_vecsize,
		  uint32 i_blksize)
{
    uint32 n_dmp_min;
    uint32 n_dmp_frame_max;
    uint32 n_dir;
    uint32 n_tot_frame;
    uint32 i, j;
    uint32 part = 0;
    uint32 hdr_sz;
    
    n_seg = in_n_seg;
    n_frame = in_n_frame;	/* NULL => 1 frame per segment */

    n_stream = i_n_stream;
    vecsize = i_vecsize;
    blksize = i_blksize;

    printf("dfn is %s\n", dfn);
    printf("ifn is %s\n", ifn);
    fflush(stdout);

    for (n_dir = 0; dirs[n_dir]; n_dir++);

    cur_type = data_type;

    if (cur_type == SEGDMP_TYPE_FEAT) {
	E_INFO("Producing FEAT dump\n");
    }
    else if (cur_type == SEGDMP_TYPE_MFCC) {
	E_INFO("Producing MFCC dump\n");
    }
    else if (cur_type == SEGDMP_TYPE_VQ) {
	E_INFO("Producing VQ dump\n");
    }

    for (n_tot_frame = 0, i = 0; i < n; i++) {
	if (n_frame == NULL) {
	    /* one frame segments, so n_seg == n_frame */
	    n_tot_frame += n_seg[i];
	}
	else {
	    for (j = 0; j < n_seg[i]; j++) {
		n_tot_frame += n_frame[i][j];
	    }
	}
    }

    n_id = n;

    id_part = ckd_calloc(n_id, sizeof(uint32));
    id_off = ckd_calloc(n_id, sizeof(uint32));
    id_len = ckd_calloc(n_id, sizeof(uint32));
    
    if (cur_type == SEGDMP_TYPE_FEAT) {
	frame_sz = blksize * sizeof(float32);
    }
    else if (cur_type == SEGDMP_TYPE_VQ) {
	frame_sz = 4;	/* HAQUE */
    }
    else if (cur_type == SEGDMP_TYPE_MFCC) {
	/* Ideally, this module shouldn't be calling cmd_ln_* functions */
	frame_sz = cmd_ln_int32("-ceplen") * sizeof(float32);
    }

    n_dmp_frame_max = (DMP_MAX / frame_sz);
    
    if (DMP_MAX % frame_sz)
	++n_dmp_frame_max;
    
    n_dmp_min = n_tot_frame / n_dmp_frame_max;

    if (n_tot_frame % n_dmp_frame_max)
	++n_dmp_min;
    
    E_INFO("%u total frames; %u frames/dmp max\n", n_tot_frame, n_dmp_frame_max);

    part = 0;
    id_part[0] = part;
    id_off[0] = 0;
    if (n_frame == NULL) {
	id_len[0] = n_seg[0];
    }
    else {
	for (j = 0; j < n_seg[0]; j++) {
	    id_len[0] += n_frame[0][j];
	}
    }

    if (id_len[0] * frame_sz > DMP_MAX) {
	E_FATAL("ID 0 has more data (%u bytes) than DMP_MAX (%u bytes); Increase DMP_MAX\n", id_len[0]*frame_sz, DMP_MAX);
    }

    /* compute dump part, offset and len for all remaining id's */
    for (i = 1; i < n_id; i++) {
	id_off[i] = id_off[i-1] + id_len[i-1] * frame_sz;

	if (id_off[i] > DMP_MAX) {
	    ++part;
	    id_off[i] = 0;
	}

	id_part[i] = part;

	if (n_frame == NULL) {
	    /* i.e. 1 frame per segment */
	    id_len[i] = n_seg[i];
	}
	else {
	    for (j = 0; j < n_seg[i]; j++) {
		id_len[i] += n_frame[i][j];
	    }
	}
    }

    ++part;

    n_part = part;

    if (n_part <= n_dir) {
	E_INFO("%u of %u directories filled\n",
	       n_part, n_dir);
    }
    else {
	E_FATAL("%u directories required, but only %u given\n",
		n_part, n_dir);
    }

    /* Create/open the dump files */
    dmp_fp = ckd_calloc(n_part, sizeof(FILE *));
    for (i = 0; i < n_part; i++) {
	sprintf(dmp_fn, "%s/%s", dirs[i], dfn);

	dmp_fp[i] = s3open(dmp_fn, "wb", NULL);

	/* add the file header size to all the offsets computed above. */
	hdr_sz = ftell(dmp_fp[i]);
	for (j = 0; j < n_id; j++) {
	    if (id_part[j] == i)
		id_off[j] += hdr_sz;
	}
    }

    idx_fp = ckd_calloc(n_part, sizeof(FILE *));
    if (ifn) {
	for (i = 0; i < n_part; i++) {
	    /* Create the index file */
	    sprintf(idx_fn, "%s/%s", dirs[i], ifn);
	    
	    idx_fp[i] = s3open(idx_fn, "wb", NULL);
	    
	    if (bio_fwrite((int *)&data_type, sizeof(int), 1, idx_fp[i], 0, &ignore) != 1) {
		E_FATAL_SYSTEM("unable to write seg dmp file");
	    }
	    if (write_idx(i) == S3_SUCCESS) {
		E_INFO("Created index %s/%s\n", dirs[i], ifn);
	    }
	    else {
		E_INFO("Error creating index %s/%s\n", dirs[i], ifn);
	    }
		
	    s3close(idx_fp[i]);
	}
    }
    else {
	E_INFO("Assuming prior indices are available and appropriate\n");
    }

    ckd_free((void *)idx_fp);
    idx_fp = NULL;

    alloc_frm_buf = frm_buf_sz / frame_sz;
    printf("frm_buf_size is %d frame_sz is %d\n", frm_buf_sz , frame_sz);
    fflush(stdout);

    E_INFO("%u frames buffered before write\n", alloc_frm_buf);
    alloc_frm_buf *= frame_sz;			/* make a multiple of frame_sz */

    if ((frm_buf = (void *)ckd_calloc(alloc_frm_buf, sizeof(char)) )==NULL) {
	printf("died like a pig\n");
	exit(1);
    }
    h_seg = (seg_t **)ckd_calloc(n_id, sizeof(seg_t *));
    t_seg = (seg_t **)ckd_calloc(n_id, sizeof(seg_t *));

    cur_type = data_type;
    
    return S3_SUCCESS;
}

int
segdmp_close()
{
    if (frm_buf)
	dump_frm_buf();

    ckd_free(id_part);
    id_part = NULL;

    ckd_free(id_off);
    id_off = NULL;

    ckd_free(id_len);
    id_len = NULL;

    ckd_free(dmp_fp);
    dmp_fp = NULL;

    ckd_free(frm_buf);
    frm_buf = NULL;

    ckd_free(h_seg);
    h_seg = NULL;

    ckd_free(t_seg);
    t_seg = NULL;

    return S3_SUCCESS;
}

int
segdmp_add_feat(uint32 id,
		vector_t **feat,
		uint32 n_seg_frame)
{
    seg_t *s;

    if (n_seg_frame*frame_sz + nxt_frm_buf > alloc_frm_buf) {
	dump_frm_buf();
    }

    s = ckd_calloc(1, sizeof(seg_t));

    s->len = n_seg_frame;
    s->idx = nxt_frm_buf;

    if (t_seg[id]) {
	t_seg[id]->next = s;
    }

    t_seg[id] = s;

    if (h_seg[id] == NULL) {
	h_seg[id] = t_seg[id];
    }

    memcpy(&frm_buf[nxt_frm_buf],
	   (const void *)&feat[0][0][0],
	   n_seg_frame * frame_sz);
    
    nxt_frm_buf += n_seg_frame * frame_sz;
    
    return S3_SUCCESS;
}


int
segdmp_next_feat(uint32 id,
		 vector_t ***out_feat,
		 uint32 *out_n_frame)
{
    FILE *fp;
    uint32 s;
    uint32 off;
    uint32 n_seg_frame;
    float32 *buf;
    vector_t **feat;
    uint32 i, j, l;
    
    fp = dmp_fp[id_part[id]];

    s = nxt_seg[id];
    if (s == n_seg[id]) {
	nxt_seg[id] = 0;

	return 0;
    }
    else {
	++nxt_seg[id];
    }

    off = id_nxt_off[id];
    if (ftell(fp) != off) {
	if (fseek(fp, off, SEEK_SET) < 0) {
	    E_FATAL_SYSTEM("Unable to seek to position in dmp file");
	}
    }

    if (n_frame == NULL) {
	n_seg_frame = 1;
    }
    else {
	n_seg_frame = n_frame[id][s];
    }

    buf = ckd_malloc(n_seg_frame * frame_sz);

    if (bio_fread(buf, sizeof(char), frame_sz * n_seg_frame,
	       fp, dmp_swp[id_part[id]], &ignore) != frame_sz * n_seg_frame) {
	E_FATAL_SYSTEM("Unable to read segment from dmp file");
    }

    id_nxt_off[id] = ftell(fp);

    feat = (vector_t **)ckd_calloc_2d(n_seg_frame, n_stream, sizeof(vector_t));
    for (i = 0, l = 0; i < n_seg_frame; i++) {
	for (j = 0; j < n_stream; j++) {
	    feat[i][j] = &buf[l];
	    
	    l += vecsize[j];
	}
    }

    *out_feat = feat;
    *out_n_frame = n_seg_frame;

    return 1;
}


uint32
segdmp_n_seg(uint32 id)
{
    return n_seg[id];
}
