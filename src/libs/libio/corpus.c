/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * 4. Redistributions of any form whatsoever must retain the following
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
/*********************************************************************
 *
 * File: corpus.c
 * 
 * Description: 
 *	This file contains the routines which manage the access of the
 *	data related to each utterance of a speech data corpus.  Such
 *	data currently includes MFCC and word transcript data.
 *	One could imagine many more data types however.
 *
 *	A speech corpus is defined by a control file.  It is just a
 *	list of relative paths, one-per-line, for all the utterances
 *	in a speech corpus.  Each data type (e.g. MFCC, SENT) must
 *	reside under a single root directory.  If a speech corpus
 *	is not unified under a single root directory, it is
 *	assumed that it is easy to construct such a directory
 *	heirarchy using, for example, symbolic links.
 *
 *	These routines allow partitions of a speech corpus to
 *	be defined for the purpose of distributing the work over
 *	multiple processors.  Selection of a set in a partition
 *	may be specified in two ways; either a by a skip count
 *	and a run length or by identifying a set out of N (roughly)
 *	equal sized sets (i.e. part 1 of 10).
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <s3/corpus.h>

#include <s3/ckd_alloc.h>
#include <s3/read_line.h>
#include <s3/prefetch.h>
#include <s3/mllr_io.h>
#include <sys_compat/file.h>
#include <sys_compat/misc.h>
#include <s3/acmod_set.h>
#include <s3/s2io.h>
#include <s3/s3.h>

/* System level includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

/*
 * Private functions to corpus
 */

static
int strcmp_ci(const char *a, const char *b);

static char *
fgets_wo_nl(char *str,
	    size_t max,
	    FILE *fp);
static int
read_sildel(uint32 **del_sf,
	    uint32 **del_ef,
	    uint32 *n_del);

static char *
mk_filename(uint32 type, char *rel_path);

static FILE *
open_file_for_reading(uint32 type);

static int
corpus_read_next_sent_file(char **trans);

static int
corpus_read_next_lsn_line(char **trans);

#define	DATA_TYPE_SENT	0
#define DATA_TYPE_MFCC	1
#define DATA_TYPE_SEG	2
#define DATA_TYPE_CCODE	3
#define DATA_TYPE_DCODE	4
#define DATA_TYPE_PCODE	5
#define DATA_TYPE_DDCODE 6
#define DATA_TYPE_MLLR  7

#define N_DATA_TYPE	8

#define MAX_LSN_LINE	8192

/* The root directory for the speech corpus.  Each line of the control
 * file is appended to this directory */
static const char *data_dir[N_DATA_TYPE];

/* The file name extensions for each data type */
static const char *extension[N_DATA_TYPE];

/* Whether the directory is flat or uses the subdirs in the control
 * file. */
static int is_flat[N_DATA_TYPE];

/* The name of an LSN file containing all transcripts for the corpus */
static const char *lsn_filename = NULL;

/* Standard I/O file pointer for the LSN file */
static FILE *lsn_fp = NULL;

/* The current LSN transcript */
static char lsn_line[MAX_LSN_LINE];


/* The current mllr tranform */
static char mllr_line[MAXPATHLEN];

/* Standard I/O file pointer for the silence deletion file */
static FILE *sil_fp = NULL;

/* next line of the sildence file file */
static char sil_line[MAXPATHLEN];

static uint32 *del_sf = NULL;
static uint32 *del_ef = NULL;
static uint32 n_del = 0;

/* Standard I/O file pointer for the control file */
static FILE *ctl_fp = NULL;

/* Control file lines may be of the following form:
 *
 *	<path w/o extension> [<start_frame> <end_frame> [<utt_id>]]
 *
 */

static char ctl_line_a[8192] = "";
static char ctl_line_b[8192] = "";

#define NO_FRAME	0xffffffff

/* The current line from a control file */
static char *cur_ctl_line = ctl_line_a;

/* The current path from a control file */
static char *cur_ctl_path = NULL;

/* The current start frame ( < 0 indicates NONE) from a control file */
static uint32 cur_ctl_sf = NO_FRAME;

/* The current end frame ( < 0 indicates NONE) from a control file */
static uint32 cur_ctl_ef = NO_FRAME;

/* The current utt id (NULL indicates NONE) from a control file */
static char *cur_ctl_utt_id = NULL;

static char *next_ctl_line = ctl_line_b;

static char *next_ctl_path = NULL;

/* Flag to indicate whether the application requires MFCC data */
static int32 requires_mfcc = FALSE;

/* Flag to indicate whether the application requires sentence
 * transcripts */
static int32 requires_sent = FALSE;

/* Flag to indicate whether the application requires state
 * segmentations */
static int32 requires_seg = FALSE;

static int32 requires_ccode = FALSE;
static int32 requires_dcode = FALSE;
static int32 requires_pcode = FALSE;
static int32 requires_ddcode = FALSE;

#define UNTIL_EOF	0xffffffff

static uint32 sv_n_skip = 0;
static uint32 sv_run_len = UNTIL_EOF;

static uint32 n_run = UNTIL_EOF;
static uint32 n_proc = 0;

static uint32 begin;

static
int strcmp_ci(const char *a, const char *b)
{
    char a_lc[1024];
    char b_lc[1024];
    int i;

    strcpy(a_lc, a);
    strcpy(b_lc, b);
    
    for (i = 0; i < strlen(a_lc); i++)
	a_lc[i] = tolower(a_lc[i]);

    for (i = 0; i < strlen(b_lc); i++)
	b_lc[i] = tolower(b_lc[i]);

    return strcmp(a_lc, b_lc);
}

static char *
fgets_wo_nl(char *str,
	    size_t max,
	    FILE *fp)
{
    char *out;
    uint32 len;

    out = fgets(str, max, fp);

    if (out == NULL)
	return NULL;

    len = strlen(out);

    if (out[len-1] == '\n')
	out[len-1] = '\0';
    else {
	E_FATAL("input string too long.  Truncated.\n");

	return NULL;
    }

    return out;
}

int32
corpus_provides_mfcc()
{
    return requires_mfcc;
}

int32
corpus_provides_sent()
{
    return requires_sent;
}

int32
corpus_provides_seg()
{
    return requires_seg;
}

int32
corpus_provides_ccode()
{
    return requires_ccode;
}

int32
corpus_provides_dcode()
{
    return requires_dcode;
}

int32
corpus_provides_pcode()
{
    return requires_pcode;
}

int32
corpus_provides_ddcode()
{
    return requires_ddcode;
}

static void
parse_ctl_line(char *line,
	       char **path,
	       uint32 *sf,
	       uint32 *ef,
	       char **id)
{
    char *sp;
    char sv_line[4192];

    strcpy(sv_line, line);

    sp = strchr(sv_line, ' ');
    if (sp == NULL) {
	/* 'old' style control file */
	if (path)
	    *path = strdup(sv_line);
	if (sf)
	    *sf = NO_FRAME;
	if (ef)
	    *ef = NO_FRAME;
	if (id)
	    *id = NULL;
    }
    else {
	*sp = '\0';
	if (path)
	    *path = strdup(sv_line);
	
	/* at least one space, so try to parse rest of line */
	if (sf != NULL)
	    *sf = atoi(sp+1);	/* set the start frame */

	sp = strchr(sp+1, ' ');
	if (sp == NULL) {
	    E_FATAL("Control file line must be '<path> [<sf> <ef> [<id>]]', instead saw '%s'\n",
		    line);
	}

	if (ef != NULL)
	    *ef = atoi(sp+1);	/* set the end frame */

	sp = strchr(sp+1, ' ');
	if (id != NULL) {
	    if (sp == NULL) {
		/* assume that the optional ID has been omitted */
		*id = NULL;
	    }
	    else {
		*id = strdup(sp+1);	/* set the utterance ID */
	    }
	}
    }
}

/*********************************************************************
 *
 * Function: corpus_set_ctl_filename
 * 
 * Description: 
 *    This routine sets the control file used to define the corpus.
 *    It has a side-effect of opening the control file.
 * 
 * Function Inputs: 
 *    const char *ctl_filename -
 * 	This is the file name of the control file.
 *
 * Global Inputs: 
 *    None
 *
 * Return Values: 
 *    S3_SUCCESS -
 *	Indicates the control file could be opened for reading.
 *
 *    S3_ERROR -
 *	Indicates some error occured while opening the control file.
 *
 * Global Outputs: 
 *    None
 *
 * Pre-Conditions: 
 *    ctl_filename argument must be a pointer to a C string.
 * 
 * Post-Conditions: 
 * 
 *********************************************************************/

int
corpus_set_ctl_filename(const char *ctl_filename)
{
    ctl_fp = fopen(ctl_filename, "r");
    if (ctl_fp == NULL) {
	E_WARN_SYSTEM("Unable to open %s for reading\n",
		      ctl_filename);

	return S3_ERROR;
    }

    if (fgets_wo_nl(next_ctl_line, MAXPATHLEN, ctl_fp) == NULL) {
	E_ERROR("Must be at least one line in the control file\n");

	return S3_ERROR;
    }

    parse_ctl_line(next_ctl_line,
		   &next_ctl_path,
		   NULL,
		   NULL,
		   NULL);

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_get_begin
 * 
 * Description: 
 *    This function returns the offset of the first utterance to be
 *    processed.  This allows the
 * 
 * Function Inputs: 
 *    None
 * 
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    The current skip count to the starting utterance.
 * 
 * Global Outputs: 
 *    None
 * 
 * Errors: 
 *    None
 * 
 *********************************************************************/

uint32
corpus_get_begin()
{
    return begin;
}

/*********************************************************************
 *
 * Function: corpus_set_interval
 * 
 * Description: 
 *    Defines the interval in the control file over which to run.
 *    The interval is specified by a count of the utterances to skip
 *    followed by a number of utterances to run over.
 * 
 * Function Inputs: 
 *    uint32 n_skip -
 *	The number of utterances in the control file to skip over
 *
 *    uint32 run_len -
 *	The number of utterances to run over after skipping over
 *	N_SKIP utterances.
 *
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Currently, the only possible return value
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_interval(uint32 n_skip,
		    uint32 run_len)
{
    sv_n_skip = n_skip;
    sv_run_len = run_len;

    if (n_skip) {
	E_INFO("skipping %d utts.\n", n_skip);
	
	for (begin = 0; (n_skip > 0) && corpus_next_utt(); --n_skip, begin++);
	
	E_INFO("Last utt skipped: %s\n", corpus_utt());
    }
    
    if (run_len != UNTIL_EOF)
	n_run = run_len;

    n_proc = 0;
    
    return S3_SUCCESS;
}

int
corpus_reset()
{
    n_run = UNTIL_EOF;

    assert(ctl_fp);
    rewind(ctl_fp);

    if (lsn_fp)
	rewind(lsn_fp);
    if (sil_fp)
	rewind(sil_fp);

    cur_ctl_line[0] = '\0';
    next_ctl_line[0] = '\0';

    /* Position the control file to the
     * saved values
     */
    corpus_set_interval(sv_n_skip, sv_run_len);

    return S3_SUCCESS;
}

/*
 * This must be done after the processing of the utterance
 */
int
corpus_ckpt(const char *fn)
{
    FILE *fp;
    char tmp[256];

    fp = fopen(fn, "w");
    if (fp == NULL) {
	E_ERROR_SYSTEM("Unable to open chkpt file %s\n", fn);

	return S3_ERROR;
    }
    
    sprintf(tmp,"%u %u\n", begin + n_proc, n_run); 

    printf("|%s|\n", tmp);

    if (fprintf(fp, "%u %u\n", begin + n_proc, n_run) != strlen(tmp)) {
	E_ERROR_SYSTEM("Unable to write %s successfully\n", fn);
    }

    fclose(fp);
    
    return S3_SUCCESS;
}

int
corpus_ckpt_set_interval(const char *fn)
{
    FILE *fp;
    uint32 o, rl;

    fp = fopen(fn, "r");
    if (fp == NULL) {
	E_ERROR_SYSTEM("Can't open ckpt file %s", fn);

	return S3_ERROR;
    }

    if (fscanf(fp, "%u %u", &o, &rl) != 2) {
	E_ERROR("Problems reading ckpt file %s\n", fn);
	fclose(fp);

	return S3_ERROR;
    }

    fclose(fp);

    return corpus_set_interval(o, rl);
}

/*********************************************************************
 *
 * Function: corpus_set_partition
 * 
 * Description: 
 *    This function allows one to specify a set R of a partition of
 *    the corpus into S (roughly) equal sized partitions.
 * 
 * Function Inputs: 
 *    uint32 r -
 *	This argument selects the Rth OF_S sets (R runs from 1..OF_S)
 *
 *    uint32 of_s -
 *	The number of total (roughly equal sized) sets in the partition.
 * 
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Operation completed successfully
 *    S3_ERROR   - Operation did not complete successfully
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_partition(uint32 r,
		     uint32 of_s)
{
    uint32 lineno;
    char ignore[MAXPATHLEN+1];
    uint32 run_len;
    uint32 n_skip;

    if (ctl_fp == NULL) {
	E_ERROR("Control file has not been set\n");

	return S3_ERROR;
    }

    for (lineno = 0; fgets(ignore, MAXPATHLEN+1, ctl_fp); lineno++);

    rewind(ctl_fp);

    fgets_wo_nl(next_ctl_line, MAXPATHLEN, ctl_fp);

    run_len = lineno / of_s;

    n_skip = (r-1) * run_len;

    if (r == of_s)
	run_len = UNTIL_EOF;

    return corpus_set_interval(n_skip, run_len);
}

/*********************************************************************
 *
 * Function: corpus_set_mfcc_dir
 * 
 * Description: 
 *    Set the root directory for the MFCC data.
 * 
 * Function Inputs: 
 *    const char *dir -
 *	This is the root directory for the MFCC data.
 *
 * Global Inputs: 
 *    None
 *
 * Return Values: 
 *    S3_SUCCESS - Currently the only return value.
 *
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_mfcc_dir(const char *dir)
{
    requires_mfcc = TRUE;

    data_dir[DATA_TYPE_MFCC] = dir;
    is_flat[DATA_TYPE_MFCC] = FALSE;

    return S3_SUCCESS;
}


/*********************************************************************
 *
 * Function: corpus_set_mllr_dir
 * 
 * Description: 
 *    Set the root directory for the MLLR data.
 * 
 * Function Inputs: 
 *    const char *dir -
 *	This is the root directory for the MLLR data.
 *
 * Global Inputs: 
 *    None
 *
 * Return Values: 
 *    S3_SUCCESS - Currently the only return value.
 *
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_mllr_dir(const char *dir)
{

    data_dir[DATA_TYPE_MLLR] = dir;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_mfcc_ext
 * 
 * Description: 
 *    Set the file name extension for MFCC data files
 * 
 * Function Inputs: 
 *    const char *ext -
 *	This is the file name extension for MFCC file names.
 *
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Currently, the only return value
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_mfcc_ext(const char *ext)
{
    extension[DATA_TYPE_MFCC] = ext;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_seg_dir
 * 
 * Description: 
 *    Set the root directory for the state segmentation data.
 * 
 * Function Inputs: 
 *    const char *dir -
 *	This is the root directory for the state segmentation data.
 *
 * Global Inputs: 
 *    None
 *
 * Return Values: 
 *    S3_SUCCESS - Currently the only return value.
 *
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_seg_dir(const char *dir)
{
    char *tt;

    requires_seg = TRUE;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_SEG] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_SEG] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in seg dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_SEG] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_SEG] = FALSE;
    }

    data_dir[DATA_TYPE_SEG] = dir;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_seg_ext
 * 
 * Description: 
 *    Set the file name extension for the state segmentation files
 * 
 * Function Inputs: 
 *    const char *ext -
 *	This is the file name extension for the state segmentation file names.
 *
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Currently, the only return value
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_seg_ext(const char *ext)
{
    extension[DATA_TYPE_SEG] = ext;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_sent_dir
 * 
 * Description: 
 *    Set the root directory for word transcripts.  This is only
 *    used if corpus_set_lsn_filename() has not been called.
 * 
 * Function Inputs: 
 *    const char *dir -
 *	This is the root directory for the word transcripts.
 *
 * Global Inputs: 
 *    None
 *
 * Return Values: 
 *    S3_SUCCESS - Currently the only return value.
 *
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_sent_dir(const char *dir)
{
    char *tt;

    assert(lsn_filename == NULL);

    requires_sent = TRUE;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_SENT] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_SENT] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in sent dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_SENT] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_SENT] = FALSE;
    }

    data_dir[DATA_TYPE_SENT] = dir;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_sent_ext
 * 
 * Description: 
 *    Set the file name extension for word transcripts.
 * 
 * Function Inputs: 
 *    const char *ext -
 *	This is the file name extension for word transcripts.
 *
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Currently, the only return value
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_sent_ext(const char *ext)
{
    extension[DATA_TYPE_SENT] = ext;

    return S3_SUCCESS;
}
int
corpus_set_ccode_dir(const char *dir)
{
    char *tt;

    requires_ccode = TRUE;

    data_dir[DATA_TYPE_CCODE] = dir;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_CCODE] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_CCODE] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in ccode dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_CCODE] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_CCODE] = FALSE;
    }

    return S3_SUCCESS;
}
int
corpus_set_ccode_ext(const char *ext)
{
    extension[DATA_TYPE_CCODE] = ext;

    return S3_SUCCESS;
}
int
corpus_set_dcode_dir(const char *dir)
{
    char *tt;

    requires_dcode = TRUE;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_DCODE] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_DCODE] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in dcode dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_DCODE] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_DCODE] = FALSE;
    }

    data_dir[DATA_TYPE_DCODE] = dir;

    return S3_SUCCESS;
}
int
corpus_set_dcode_ext(const char *ext)
{
    extension[DATA_TYPE_DCODE] = ext;

    return S3_SUCCESS;
}
int
corpus_set_pcode_dir(const char *dir)
{
    char *tt;

    requires_pcode = TRUE;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_PCODE] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_PCODE] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in pcode dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_PCODE] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_PCODE] = FALSE;
    }

    data_dir[DATA_TYPE_PCODE] = dir;

    return S3_SUCCESS;
}
int
corpus_set_pcode_ext(const char *ext)
{
    extension[DATA_TYPE_PCODE] = ext;

    return S3_SUCCESS;
}
int
corpus_set_ddcode_dir(const char *dir)
{
    char *tt;

    requires_ddcode = TRUE;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_PCODE] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_PCODE] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in pcode dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_PCODE] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_PCODE] = FALSE;
    }

    data_dir[DATA_TYPE_DDCODE] = dir;

    return S3_SUCCESS;
}
int
corpus_set_ddcode_ext(const char *ext)
{
    extension[DATA_TYPE_DDCODE] = ext;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_lsn_filename
 * 
 * Description:
 *    This routine sets and opens a LSN file which must contain all
 *    the word transcripts for corpus.  The order of this file must
 *    be the same as the control file.  The LSN file is expected to
 *    use the NIST defined LSN format.
 * 
 * Function Inputs: 
 *    const char *fn -
 *	This is the LSN file name.
 * 
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Currently, this is the only return value.  All
 *		errors currently are fatal.
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_lsn_filename(const char *fn)
{
    assert(data_dir[DATA_TYPE_SENT] == NULL);

    lsn_filename = fn;

    requires_sent = TRUE;

    lsn_fp = fopen(lsn_filename, "r");
    if (lsn_fp == NULL) {
	E_FATAL_SYSTEM("Cannot open LSN filename %s", lsn_filename);
    }

    return S3_SUCCESS;
}


/*********************************************************************
 *
 * Function: corpus_set_mllr_filename
 * 
 * Description:
 *    This routine sets and opens a MLLR file which must contain all
 *    the MLLR transforms for the corpus.  The order of this file must
 *    be the same as the control file.
 * 
 * Function Inputs: 
 *    const char *fn -
 *	This is the MLLR file name.
 * 
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - Currently, this is the only return value.  All
 *		errors currently are fatal.
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_set_mllr_filename(const char *fn)
{
    const char *mllr_filename = NULL;
    FILE *mllr_fp = NULL;

    assert(data_dir[DATA_TYPE_SENT] == NULL);

    mllr_filename = fn;

    mllr_fp = fopen(mllr_filename, "r");
    if (mllr_fp == NULL) {
	E_FATAL_SYSTEM("Cannot open MLLR filename %s", mllr_filename);
    }

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_init
 * 
 * Description: 
 *    This routine takes the configuration parameters given by
 *    the corpus_set_*() functions defined above and does any
 *    remaining setup.
 * 
 * Function Inputs: 
 *    None
 * 
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    S3_SUCCESS - No error was found during initialization
 *    S3_ERROR   - Some error occurred so that further use of the
 *		   corpus module would result in errors.
 * 
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
corpus_init()
{
    /* Currently, just do some sanity checking */

    if (ctl_fp == NULL) {
	E_ERROR("Control file not given before corpus_init() called\n");

	return S3_ERROR;
    }

    if (requires_sent &&
	(lsn_fp == NULL) &&
	(extension[DATA_TYPE_SENT] == NULL)) {

	E_ERROR("No lexical entry transcripts given\n");
	
	return S3_ERROR;
    }

    if (requires_mfcc &&
	extension[DATA_TYPE_MFCC] == NULL) {
	E_ERROR("No MFCC extension given\n");

	return S3_ERROR;
    }

    if (requires_seg &&
	extension[DATA_TYPE_SEG] == NULL) {
	E_ERROR("No seg extension given\n");

	return S3_ERROR;
    }

    if (requires_ccode &&
	extension[DATA_TYPE_CCODE] == NULL) {
	E_ERROR("No ccode extension given\n");

	return S3_ERROR;
    }

    if (requires_dcode &&
	extension[DATA_TYPE_DCODE] == NULL) {
	E_ERROR("No dcode extension given\n");

	return S3_ERROR;
    }

    if (requires_pcode &&
	extension[DATA_TYPE_PCODE] == NULL) {
	E_ERROR("No pcode extension given\n");

	return S3_ERROR;
    }

    if (requires_ddcode &&
	extension[DATA_TYPE_DDCODE] == NULL) {
	E_ERROR("No ddcode extension given\n");

	return S3_ERROR;
    }

    if (n_run == UNTIL_EOF) {
	E_INFO("Will process all remaining utts starting at %d\n", begin);
    }
    else {
	E_INFO("Will process %d utts starting at %d\n", n_run, begin);
    }
    
    return S3_SUCCESS;
}

int
corpus_next_utt()
{
    char *tt;

    tt = cur_ctl_line;
    cur_ctl_line = next_ctl_line;
    next_ctl_line = tt;

    if (cur_ctl_path) {
	free(cur_ctl_path);
	cur_ctl_path = NULL;
    }
    if (cur_ctl_utt_id) {
	free(cur_ctl_utt_id);
	cur_ctl_utt_id = NULL;
    }
    parse_ctl_line(cur_ctl_line,
		   &cur_ctl_path,
		   &cur_ctl_sf,
		   &cur_ctl_ef,
		   &cur_ctl_utt_id);

    if (next_ctl_path) {
	free(next_ctl_path);
	next_ctl_path = NULL;
    }
    parse_ctl_line(next_ctl_line,
		   &next_ctl_path,
		   NULL,
		   NULL,
		   NULL);
    
    if (n_run != UNTIL_EOF) {
	if (n_run == 0) return FALSE;

	--n_run;
    }

    ++n_proc;

    if (strlen(cur_ctl_line) == 0)
	/* this means that the prior call reached the ctl file EOF */
	return FALSE;


    /* if a big LSN file exists, position it to the correct line */

    /* NOTE: corpus_set_ctl_filename() reads the first line of
     *       the control file, so that lsn_fp is one line
     *       behind ctl_fp. */

    if (lsn_fp) {
	if (fgets_wo_nl(lsn_line, MAX_LSN_LINE, lsn_fp) == NULL)
	    lsn_line[0] = '\0';
    }

    if (sil_fp) {
	if (del_sf)
	    ckd_free(del_sf);
	if (del_ef)
	    ckd_free(del_ef);

	if (fgets_wo_nl(sil_line, MAXPATHLEN, sil_fp) == NULL)
	    sil_line[0] = '\0';

	read_sildel(&del_sf, &del_ef, &n_del);
    }

    if (fgets_wo_nl(next_ctl_line, MAXPATHLEN, ctl_fp) == NULL)
	next_ctl_line[0] = '\0';

    return TRUE;
}

char *corpus_utt()
{
    int need_slash = 1;
    int i;

    if (cur_ctl_utt_id)
	return cur_ctl_utt_id;
    else {
	if (cur_ctl_path && strlen(cur_ctl_path) > 0) {
	    i = strlen(cur_ctl_path);
	    do {
		if (cur_ctl_path[--i] == '/')
		    --need_slash;
	    } while ((i > 0) && need_slash);
	    
	    if (i == 0)
		return cur_ctl_path;
	    else
		return &cur_ctl_path[i+1];
	}
	else {
	    return "N/A";
	}
    }
}

char *corpus_utt_brief_name()
{
    int need_slash = 2;
    int i;

    if (cur_ctl_path && strlen(cur_ctl_path) > 0) {
	i = strlen(cur_ctl_path);
	do {
	    if (cur_ctl_path[--i] == '/')
		--need_slash;
	} while ((i > 0) && need_slash);
	
	if (i == 0)
	    return cur_ctl_path;
	else
	    return &cur_ctl_path[i+1];
    }
    else
	return "N/A";
}

char *corpus_utt_full_name()
{
    if (cur_ctl_path && strlen(cur_ctl_path) > 0) {
	return cur_ctl_path;
    }
    else {
	return "N/A";
    }
}

static char *
mk_filename(uint32 type, char *rel_path)
{
    static char fn[MAXPATHLEN];
    const char *r;
    const char *e;
    char *tt;

    r = data_dir[type];
    e = extension[type];
    
    if (r && e) {
	if (is_flat[type] == FALSE) {
	    sprintf(fn, "%s/%s.%s",
		    r, rel_path, e);
	}
	else {
	    tt = strrchr(rel_path, '/');
	    if (tt) {
		++tt;
	    }
	    else {
		tt = rel_path;
	    }
	    sprintf(fn, "%s/%s.%s",  r, tt, e);
	}
    }
    else if (r) {
	if (is_flat[type] == FALSE) {
	    sprintf(fn, "%s/%s",
		    r, rel_path);
	}
	else {
	    tt = strrchr(rel_path, '/');
	    if (tt) {
		++tt;
	    }
	    else {
		tt = rel_path;
	    }
	    sprintf(fn, "%s/%s",  r, tt);
	}
    }
    else if (e) {
	sprintf(fn, "%s.%s",
		rel_path, e);
    }
    else {
	strcpy(fn, rel_path);
    }

    return fn;
}

static FILE *
open_file_for_reading(uint32 type)
{
    char *fn;
    FILE *out;

    fn = mk_filename(type, cur_ctl_path);

    out = fopen(fn, "r");
    if (out == NULL) {
	E_WARN_SYSTEM("Unable to open %s for reading\n", fn);
    }
    
    return out;
}

static int
corpus_read_next_sent_file(char **trans)
{
    FILE *fp;
    char big_str[8192];

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_SENT, next_ctl_path));

    /* open the current file */
    fp = open_file_for_reading(DATA_TYPE_SENT);

    if (fgets_wo_nl(big_str, 8192, fp) == NULL) {
	E_ERROR("Unable to read data in sent file %s\n",
		mk_filename(DATA_TYPE_SENT, cur_ctl_path));
	
	return S3_ERROR;
    }

    fclose(fp);

    *trans = strdup(big_str);

    return S3_SUCCESS;
}

#include <s3/s2_param.h>	/* provides dfn of S2_CEP_VECLEN */

int
corpus_get_mfcc(vector_t **mfc,
		uint32 *n_frame,
		uint32 *veclen)
{
    vector_t *out;
    float32 *coeff;
    uint32 n_f;
    uint32 n_c;
    uint32 i, j;
    uint32 ret = S3_ERROR;

    if (!requires_mfcc) {
	/* asked for mfc data, but not set up to send it */
	return S3_ERROR;
    }

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_MFCC, next_ctl_path));

    do {
	if ((cur_ctl_sf == NO_FRAME) && (cur_ctl_ef == NO_FRAME)) {
	    ret = areadfloat(mk_filename(DATA_TYPE_MFCC, cur_ctl_path),
			     &coeff, (int *)&n_c);
	}
	else if ((cur_ctl_sf != NO_FRAME) && (cur_ctl_ef != NO_FRAME)) {
	    ret = areadfloat_part(mk_filename(DATA_TYPE_MFCC, cur_ctl_path),
				  cur_ctl_sf * S2_CEP_VECLEN,
				  (cur_ctl_ef + 1) * S2_CEP_VECLEN - 1,
				  &coeff, (int *)&n_c);
	}
	else {
	    E_FATAL("Both start and end frame must be set in the ctl file\n");
	}

	if (ret == S3_ERROR) {
	    E_ERROR("MFCC read failed.  Retrying after sleep...\n");
	    sleep(900);
	}
    } while (ret == S3_ERROR);

    if ((ret == 0) && (cur_ctl_sf != NO_FRAME) && (cur_ctl_ef != NO_FRAME)) {
	E_ERROR("Region [%d %d] for %s extends beyond end of file\n",
		cur_ctl_sf, cur_ctl_ef, corpus_utt());
    }

    if ((n_c % S2_CEP_VECLEN) != 0) {
	E_FATAL("Expected mfcc vector len of %d\n", S2_CEP_VECLEN);
    }
    
    n_f = n_c / S2_CEP_VECLEN;

    if (n_f == 0) {
	*mfc = NULL;
	*n_frame = 0;
    }

    out = (vector_t *)ckd_calloc(n_f, sizeof(vector_t));

    for (i = 0, j = 0; i < n_f; i++, j += S2_CEP_VECLEN) {
	out[i] = &coeff[j];
    }

    *mfc = out;
    *n_frame = n_f;
    *veclen = S2_CEP_VECLEN;

    return S3_SUCCESS;
}


/* ADDED BY BHIKSHA TO READ FEATURES OF GENERIC LENGTH. TO ALLOW
   VECLEN TO BE VALUES OTHER THAN 13. 7 JAN 98 */
int
corpus_get_generic_featurevec(vector_t **mfc,
			      uint32 *n_frame,
			      uint32 veclen)
{
    vector_t *out;
    float32 *coeff;
    uint32 n_f;
    uint32 n_c;
    uint32 i, j;
    uint32 ret=S3_ERROR;

    if (!requires_mfcc) {
	/* asked for mfc data, but not set up to send it */
	return S3_ERROR;
    }

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_MFCC, next_ctl_path));

    do {
	if ((cur_ctl_sf == NO_FRAME) && (cur_ctl_ef == NO_FRAME)) {
	    ret = areadfloat(mk_filename(DATA_TYPE_MFCC, cur_ctl_path),
			     &coeff, (int *)&n_c);
	}
	else if ((cur_ctl_sf != NO_FRAME) && (cur_ctl_ef != NO_FRAME)) {
	    ret = areadfloat_part(mk_filename(DATA_TYPE_MFCC, cur_ctl_path),
				  cur_ctl_sf * veclen,
				  (cur_ctl_ef + 1) * veclen - 1,
				  &coeff, (int *)&n_c);
	}
	else {
	    E_FATAL("Both start and end frame must be set in the ctl file\n");
	}

	if (ret == S3_ERROR) {
	    E_ERROR("MFCC read failed.  Retrying after sleep...\n");
	    sleep(900);
	}
    } while (ret == S3_ERROR);

    if ((ret == 0) && (cur_ctl_sf != NO_FRAME) && (cur_ctl_ef != NO_FRAME)) {
	E_ERROR("Region [%d %d] for %s extends beyond end of file\n",
		cur_ctl_sf, cur_ctl_ef, corpus_utt());
    }

    if ((n_c % veclen) != 0) {
	E_FATAL("Expected mfcc vector len of %d\n", veclen);
    }
    
    n_f = n_c / veclen;

    if (n_f == 0) {
	*mfc = NULL;
	*n_frame = 0;
    }

    out = (vector_t *)ckd_calloc(n_f, sizeof(vector_t));

    for (i = 0, j = 0; i < n_f; i++, j += veclen) {
	out[i] = &coeff[j];
    }

    *mfc = out;
    *n_frame = n_f;

    return S3_SUCCESS;
}

/* END OF CHANGES BY BHIKSHA */


int
corpus_get_ccode(unsigned char **ccode,
		 uint32 *n_frame)
{
    if (!requires_ccode) {
	/* asked for ccode data, but not set up to send it */
	return S3_ERROR;
    }
    if ((cur_ctl_sf != NO_FRAME) ||
	(cur_ctl_ef != NO_FRAME)) {
	E_WARN("Start and end frames not yet implemented for ccode data\n");
    }

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_CCODE, next_ctl_path));
    
    if (areadchar(mk_filename(DATA_TYPE_CCODE, cur_ctl_path),
		  (char **)ccode, (int32 *)n_frame) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

int
corpus_get_dcode(unsigned char **dcode,
		 uint32 *n_frame)
{
    if (!requires_dcode) {
	/* asked for dcode data, but not set up to send it */
	return S3_ERROR;
    }
    if ((cur_ctl_sf != NO_FRAME) ||
	(cur_ctl_ef != NO_FRAME)) {
	E_WARN("Start and end frames not yet implemented for dcode data\n");
    }

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_DCODE, next_ctl_path));

    if (areadchar(mk_filename(DATA_TYPE_DCODE, cur_ctl_path),
		  (char **)dcode, (int32 *)n_frame) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

int
corpus_get_pcode(unsigned char **pcode,
		 uint32 *n_frame)
{
    if (!requires_pcode) {
	/* asked for pcode data, but not set up to send it */
	return S3_ERROR;
    }
    if ((cur_ctl_sf != NO_FRAME) ||
	(cur_ctl_ef != NO_FRAME)) {
	E_WARN("Start and end frames not yet implemented for pcode data\n");
    }

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_PCODE, next_ctl_path));

    if (areadchar(mk_filename(DATA_TYPE_PCODE, cur_ctl_path),
		  (char **)pcode, (int32 *)n_frame) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

int
corpus_get_ddcode(unsigned char **ddcode,
		 uint32 *n_frame)
{
    if (!requires_ddcode) {
	/* asked for ddcode data, but not set up to send it */
	return S3_ERROR;
    }
    if ((cur_ctl_sf != NO_FRAME) ||
	(cur_ctl_ef != NO_FRAME)) {
	E_WARN("Start and end frames not yet implemented for dcode data\n");
    }

    /* start prefetching the next file, if one. */
    if (strlen(next_ctl_path) > 0)
	(void) prefetch_hint(mk_filename(DATA_TYPE_DDCODE, next_ctl_path));

    if (areadchar(mk_filename(DATA_TYPE_DDCODE, cur_ctl_path),
		  (char **)ddcode, (int32 *)n_frame) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

int
corpus_get_seg(uint16 **seg,
	       uint32 *n_seg)
{
    char *rel_path;

    if (!requires_seg) {
	/* asked for seg data, but not set up to send it */
	return S3_ERROR;
    }
#if 0
    if ((cur_ctl_sf != NO_FRAME) ||
	(cur_ctl_ef != NO_FRAME)) {
	E_WARN("Start and end frames not yet implemented for seg data\n");
    }
#endif

    /* If control file specifies an utt ID, use it.  O/W use the path */
    if (cur_ctl_utt_id != NULL)
	rel_path = cur_ctl_utt_id;
    else
	rel_path = cur_ctl_path;

    if (areadshort(mk_filename(DATA_TYPE_SEG, rel_path),
		   (int16 **)seg, (int32 *)n_seg) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

int
corpus_get_sildel(uint32 **sf,
		  uint32 **ef,
		  uint32 *n_seg)
{
    *sf = del_sf;
    *ef = del_ef;
    *n_seg = n_del;

    return S3_SUCCESS;
}

static int
corpus_read_next_lsn_line(char **trans)
{
    char utt_id[512];
    char *s;

    /* look for a close paren in the line */
    s = strrchr(lsn_line, ')');

    if (s != NULL) {
	/* found a close paren */

	if (s[1] == '\0') {
	    /* it is at the end of the line */

	    /* BEWARE: no space/tab charactes may follow the closing paren of an utt id */

	    *s = '\0';		/* terminate the string at the paren */

	    /* search for a matching open paren */
	    for (s--; (s >= lsn_line) && (*s != '('); s--);

	    if (*s == '(') {
		/* found a matching open paren */

		assert(strlen(s+1) < 512);

		strcpy(utt_id, s+1);

		/* check LSN utt id (if any) against ctl file utt id */
		if (strcmp_ci(utt_id, corpus_utt()) != 0) {
		    E_WARN("LSN utt id, %s, does not match ctl utt id, %s.\n",
			   utt_id, corpus_utt());
		}

		/* look for the first non-whitespace character before
		   the open paren */
		for (--s; (s >= lsn_line) && isspace((int)*s); s--);
		if (s < lsn_line) {
		  E_FATAL("Utterance transcription is empty: %s\n", lsn_line);
		}
		++s;
		*s = '\0';	/* terminate the string at the first whitespace character
				   following the first non-whitespace character found above */
	    }
	    else {
		E_ERROR("Expected open paren after ending close paren in line:\n%s", lsn_line);
		return S3_ERROR;
	    }
	}
	else {
	    /* close paren not at end of line so assume it is not
	       the close paren associated with the utt id */
	}
    }
    else {
	/* No close paren, so no utt id */
	
	/* This is fine, but the user gets no explicit sanity check
	   for the ordering of the LSN file */
    }

    *trans = strdup(lsn_line);
    
    return S3_SUCCESS;
}

int
corpus_get_sent(char **trans)
{
  if (lsn_fp == NULL)
    return corpus_read_next_sent_file(trans);
  else
    return corpus_read_next_lsn_line(trans);
}

int
corpus_has_xfrm()
{
/*    return mllr_fp != NULL; */
    return 0;
}

int
corpus_get_xfrm(float32 *****out_a,
		float32 ****out_b,
		const uint32 **out_veclen,
		uint32 *out_n_mllrcls,
		uint32 *out_n_stream)
{
    char mllrfn[MAXPATHLEN];

    if (data_dir[DATA_TYPE_MLLR])
	sprintf(mllrfn, "%s/%s", data_dir[DATA_TYPE_MLLR], mllr_line);
    else {
	strcpy(mllrfn, mllr_line);
    }
    
    if (read_reg_mat(mllrfn,
	 	     out_veclen,
		     out_n_mllrcls,
		     out_n_stream,
		     out_a,
		     out_b) != S3_SUCCESS)
	E_FATAL("Unable to read %s\n",mllrfn);

    return S3_SUCCESS;
}



static uint32 sil_lineno;

int
corpus_set_sildel_filename(const char *file_name)
{
    E_INFO("Silence deletions in %s\n", file_name);

    sil_fp = fopen(file_name, "r");
    if (sil_fp == NULL) {
	E_FATAL_SYSTEM("Unable to open silence deletion file %s", file_name);
    }

    sil_lineno = 0;

    return 0;
}

#define OFF 10

static int
read_sildel(uint32 **out_sf,
	    uint32 **out_ef,
	    uint32 *out_n_seg)
{
    char *sf_str, *ef_str;
    char sv[1024];
    char *tok;
    uint32 n_f, n_seg;
    uint32 sdf;
    uint32 edf;
    uint32 *sf;
    uint32 *ef;

    strcpy(sv, sil_line);

    tok = strtok(sil_line, " \t");
    if (strcmp_ci(tok, corpus_utt()) != 0) {
	E_FATAL("ctl file utt == %s, but sildel file utt == %s\n",
		corpus_utt(), tok);
    }
    for (n_f = 0; strtok(NULL, " \t"); n_f++);
    if (n_f % 2) {
	E_FATAL("expected even # of fields in sildel file lineno %u\n",
		sil_lineno);
    }

    n_seg = n_f / 2;

    if (n_seg == 0) {
	*out_sf = NULL;
	*out_ef = NULL;
	*out_n_seg = 0;

	return S3_SUCCESS;
    }

    sf = ckd_calloc(n_seg, sizeof(uint32));
    ef = ckd_calloc(n_seg, sizeof(uint32));

    strcpy(sil_line, sv);
    
    tok = strtok(sil_line, " \t");

    n_seg = 0;
    while ((sf_str = strtok(NULL, " \t")) &&
	   (ef_str = strtok(NULL, " \t"))) {

	sdf = atoi(sf_str) + OFF;
	edf = atoi(ef_str) - OFF;
	
	if ((int)edf - (int)sdf + 1 > 0) {
	    sf[n_seg] = sdf;
	    ef[n_seg] = edf;

	    ++n_seg;
	}
    }

    if (n_seg == 0) {
	ckd_free(sf);
	ckd_free(ef);
	
	*out_sf = NULL;
	*out_ef = NULL;
	*out_n_seg = 0;
    }
    else {
	*out_sf = sf;
	*out_ef = ef;
	*out_n_seg = n_seg;
    }

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.7  2004/03/01  15:38:00  egouvea
 * Fixed number of data types, bug report 660181
 * 
 * Revision 1.6  2003/11/18 21:07:25  egouvea
 * Got rid of warning casting the argument to "isspace".
 *
 * Revision 1.5  2002/11/13 21:42:52  egouvea
 * Code breaks if transcription is empty.
 *
 * Revision 1.4  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.3  2001/03/01 00:47:44  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.3  1997/07/18  14:04:51  eht
 * Added corpus_reset() call
 *
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 * Revision 1.19  1996/06/17  14:43:22  eht
 * Made status output a bit less cryptic
 *
 * Revision 1.18  1996/03/26  15:16:26  eht
 * Fixed defn of local functions
 *
 * Revision 1.17  1996/03/25  15:28:02  eht
 * Added ability to get VQ code files.
 *
 * Revision 1.16  1996/03/04  15:52:57  eht
 * Added capability to use VQ codeword files
 *
 * Revision 1.15  1996/02/06  18:45:26  eht
 * Added corpus_utt_full_name() call to functions that want to get the path
 * information out.
 *
 * Revision 1.14  1996/01/23  18:23:33  eht
 * Got rid of:
 * 	Unused variables
 * 	incompatible type error messages for malloced pointers
 *
 * Used:
 * 	E_ERROR() calls in stead of ad hoc solutions
 *
 * Revision 1.13  1995/12/01  19:53:24  eht
 * - Changed a float to a float32 in the corpus_get_mfcc() prototype
 * - Changed int6 to uint16 for seg output since bitwise AND operations are
 *   done on seg numbers.
 *
 * Revision 1.12  1995/12/01  18:02:17  eht
 * - Add ability to get state segmentation data per utterance
 * - Add ability to ignore any data type (e.g. decoder may only be
 * 	interested in MFCC data alone).
 * - Add corpus_utt() function to return just the utterance id.
 *
 * Revision 1.11  1995/10/17  14:02:55  eht
 * Changed so that it would port to Windows NT
 *
 * Revision 1.10  1995/10/10  12:27:37  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.9  1995/10/09  15:07:37  eht
 * Moved most of prefetching calls into corpus.c
 *
 * Revision 1.8  1995/09/07  19:03:29  eht
 * Added ability to specify training partitions as part N of R
 * total equal sized partitions of the control file.
 *
 * Revision 1.7  1995/08/29  12:20:29  eht
 * Changed all configuration calls to be prefixed with
 * "corpus_set"
 *
 * Revision 1.6  1995/08/29  12:11:17  eht
 * Updated corpus interface so that not so much is
 * bundled up in the corpus_init() call.  I'd rather
 * that call not change, but be able to do various
 * kinds of initialization
 *
 * Revision 1.5  1995/08/29  11:47:37  eht
 * Fixed erroneous "LSN file is shorter than ctl file" message
 *
 * Revision 1.4  1995/08/24  20:04:58  eht
 * Added PWP's prefetching changed
 *
 * Revision 1.3  1995/08/24  19:30:07  eht
 * Added global LSN file capability
 *
 * Revision 1.2  1995/06/02  20:28:32  eht
 * Use PWP's error reporting package
 *
 * Revision 1.1  1995/06/02  14:52:54  eht
 * Initial revision
 *
 *
 */
