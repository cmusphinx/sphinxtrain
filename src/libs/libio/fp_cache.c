/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: fp_cache.c
 * 
 * Description: 
 *    Maintains a cache of (FILE *) according to a caller-supplied
 *    id.
 *
 *    Possible uses might include creating dump files for the
 *    observations associated with some state i where each state is kept
 *    in a separate file.
 *
 *    Uses LRU cache replacement.  Caller must supply open() and close()
 *    functions for cache replacement.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/fp_cache.h>

#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

#define UNSET	0xffffffff

static int use_lkptbl = FALSE;
static uint32   fpc_max_sz = 2048;
static uint32   fpc_sz = UNSET;
static FILE   **fpc = NULL;
static uint32  *fpc_cnt = NULL;
static uint32  *fpc_id = NULL;

static uint32 fpc_hit = 0;
static uint32 fpc_miss = 0;

static void (*fpc_close)(FILE *fp) = NULL;
static FILE * (*fpc_open)(uint32 id) = NULL;

int
fpc_setmaxsz(uint32 sz)
{

    fpc_max_sz = sz;

    return S3_SUCCESS;
}

int
fpc_setclose(void (*fn)(FILE *fp))
{
    fpc_close = fn;

    return S3_SUCCESS;
}

int
fpc_setopen(FILE * (*fn)(uint32 id))
{
    fpc_open = fn;

    return S3_SUCCESS;
}

int
fpc_n_id(uint32 n)
{
    uint32 i;

    /* Deallocate previous cache+stats (if any) */
    if (fpc) {
	ckd_free((void *)fpc);
	fpc = NULL;
    }
    if (fpc_cnt) {
	ckd_free((void *)fpc_cnt);
	fpc_cnt = NULL;
    }
    if (fpc_id) {
	ckd_free((void *)fpc_id);
	fpc_id = NULL;
    }

    if (n <= fpc_max_sz) {
	use_lkptbl = TRUE;
	fpc_sz = n;
    }
    else {
	fpc_sz = fpc_max_sz;
	use_lkptbl = FALSE;

	fpc_cnt = (uint32 *)ckd_calloc(fpc_sz, sizeof(uint32));
	fpc_id  = (uint32 *)ckd_calloc(fpc_sz, sizeof(uint32));
	for (i = 0; i < fpc_sz; i++)
	    fpc_id[i] = UNSET;
    }

    fpc = (FILE **)ckd_calloc(fpc_sz, sizeof(FILE *));

    return S3_SUCCESS;
}

FILE *
fpc_get(uint32 id)
{
    FILE *ret;
    uint32 max_cnt, lru_i=0;
    uint32 i;

    assert(fpc_open);
    assert(fpc_close);
    
    if (use_lkptbl) {
	if (fpc[id] == NULL) {
	    fpc[id] = fpc_open(id);
	}
	ret = fpc[id];
    }
    else {
	/* Can't use cache as a lookup table. */
	
	/* Check for cache hit; if no hit, do LRU replacement */
	for (i = 0, ret = NULL; i < fpc_sz; i++) {
	    if (fpc_id[i] == id) {
		/* The dmp fp is in cache; use it */

		ret = fpc[i];
		fpc_cnt[i] = 0;
		
		++fpc_hit;	/* cache hit count */
	    }
	}
	if (ret == NULL) {
	    /* The dmp fp is not in cache*/
		    
	    ++fpc_miss;
		    
	    /* Find the LRU fp for replacement */
	    for (i = 0, max_cnt = 0; i < fpc_sz; i++) {
		if (fpc_cnt[i] >= max_cnt) {
		    max_cnt = fpc_cnt[i];
		    lru_i = i;
		}
	    }
		    
	    /* Replace LRU fp with fp to id dmp file */
	    if (fpc[lru_i])
		fpc_close(fpc[lru_i]);

	    ret = fpc_open(id);

	    fpc[lru_i] = ret;
	    fpc_id[lru_i] = id;
	    fpc_cnt[lru_i] = 0;
	}

	/* update the LRU counters */
	for (i = 0; i < fpc_sz; i++)
	    ++fpc_cnt[i];
	
	++fpc_hit;
    }

    return ret;
}

int
fpc_flush()
{
    uint32 i;

    for (i = 0; i < fpc_sz; i++) {
	fpc_close(fpc[i]);
	if (fpc_id)
	    fpc_id[i] = UNSET;
	if (fpc_cnt)
	    fpc_cnt[i] = 0;
    }

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
