/* -*- c-basic-offset: 4 -*- */
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

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/pio.h>
#include <sphinxbase/case.h>

#include <sys_compat/file.h>
#include <sys_compat/misc.h>

#include <s3/mllr_io.h>
#include <s3/acmod_set.h>
#include <s3/s3.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

/*
 * Private functions to corpus
 */

static char *
mk_filename(uint32 type, char *rel_path);

static FILE *
open_file_for_reading(uint32 type);

static int
corpus_read_next_sent_file(char **trans);

static int
corpus_read_next_transcription_line(char **trans);

/*
 * Data folders
 */

enum {
    DATA_TYPE_SENT,
    DATA_TYPE_MFCC,
    DATA_TYPE_SEG,
    DATA_TYPE_MLLR,
    DATA_TYPE_PHSEG,
    N_DATA_TYPE
};

/* The root directory for the speech corpus.  Each line of the control
 * file is appended to this directory */
static const char *data_dir[N_DATA_TYPE];

/* The file name extensions for each data type */
static const char *extension[N_DATA_TYPE];

/* Whether the directory is flat or uses the subdirs in the control
 * file. */
static int is_flat[N_DATA_TYPE];

/* The name of an LSN file containing all transcripts for the corpus */
static const char *transcription_filename = NULL;

/* Standard I/O file pointer for the control file */
static FILE *ctl_fp = NULL;

/* Standard I/O file pointer for the LSN file */
static FILE *transcription_fp = NULL;

/* The current transcript */
static char* transcription_line;

/* The current mllr tranform */
static char* mllr_line;


/* Control file lines may be of the following form:
 *
 *	<path w/o extension> [<start_frame> <end_frame> [<utt_id>]]
 *
 */

#define NO_FRAME	0xffffffff

/* The current path from a control file */
static char *cur_ctl_path = NULL;

/* The current start frame ( < 0 indicates NONE) from a control file */
static uint32 cur_ctl_sf = NO_FRAME;

/* The current end frame ( < 0 indicates NONE) from a control file */
static uint32 cur_ctl_ef = NO_FRAME;

/* The current utt id (NULL indicates NONE) from a control file */
static char *cur_ctl_utt_id = NULL;

/* The next path from a control file */
static char *next_ctl_path = NULL;

/* The next start frame ( < 0 indicates NONE) from a control file */
static uint32 next_ctl_sf = NO_FRAME;

/* The next end frame ( < 0 indicates NONE) from a control file */
static uint32 next_ctl_ef = NO_FRAME;

/* The next utt id (NULL indicates NONE) from a control file */
static char *next_ctl_utt_id = NULL;

/* Flag to indicate whether the application requires MFCC data */
static int32 requires_mfcc = FALSE;

/* Flag to indicate whether the application requires sentence
 * transcripts */
static int32 requires_sent = FALSE;

/* Flag to indicate whether the application requires state
 * segmentations */
static int32 requires_seg = FALSE;

/* Flag to indicate whether the application requires phone
 * segmentations */
static int32 requires_phseg = FALSE;

#define UNTIL_EOF	0xffffffff

static uint32 sv_n_skip = 0;
static uint32 sv_run_len = UNTIL_EOF;

static uint32 n_run = UNTIL_EOF;
static uint32 n_proc = 0;

static uint32 begin;

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
corpus_provides_phseg()
{
    return requires_phseg;
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
    lineiter_t *li;
    ctl_fp = fopen(ctl_filename, "rb");

    if (ctl_fp == NULL) {
	E_ERROR_SYSTEM("Unable to open %s for reading",  ctl_filename);
	return S3_ERROR;
    }
    
    li = lineiter_start_clean(ctl_fp);

    if (li == NULL) {
	E_ERROR("Must be at least one line in the control file\n");
	return S3_ERROR;
    }

    parse_ctl_line(li->buf,
		   &next_ctl_path,
		   &next_ctl_sf,
		   &next_ctl_ef,
		   &next_ctl_utt_id);
    lineiter_free (li);
    
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
    lineiter_t* li;
    n_run = UNTIL_EOF;

    assert(ctl_fp);
    fseek(ctl_fp, 0L, SEEK_SET);

    if (transcription_fp)
        fseek(transcription_fp, 0L, SEEK_SET);

    li = lineiter_start_clean(ctl_fp);

    if (li == NULL) {
	E_ERROR("Must be at least one line in the control file\n");
	return S3_ERROR;
    }

    parse_ctl_line(li->buf,
		   &next_ctl_path,
		   &next_ctl_sf,
		   &next_ctl_ef,
		   &next_ctl_utt_id);
    lineiter_free (li);


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
corpus_set_partition(uint32 part,
		     uint32 parts)
{
    uint32 run_len;
    uint32 n_skip;
    int lineno = 0;
    lineiter_t* li;

    if (ctl_fp == NULL) {
	E_ERROR("Control file has not been set\n");

	return S3_ERROR;
    }

    for (li = lineiter_start(ctl_fp); li; li = lineiter_next(li)) {
	lineno++;
    }

    fseek(ctl_fp, 0L, SEEK_SET);
    
    li = lineiter_start(ctl_fp);
    lineiter_free(li);
    
    run_len = lineno / parts;

    n_skip = (part - 1) * run_len;

    if (part == parts)
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
 * Function: corpus_set_phseg_dir
 * 
 * Description: 
 *    Set the root directory for the phone segmentation data.
 * 
 * Function Inputs: 
 *    const char *dir -
 *	This is the root directory for the phone segmentation data.
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
corpus_set_phseg_dir(const char *dir)
{
    char *tt;

    requires_phseg = TRUE;

    tt = strrchr(dir, ',');
    if (tt != NULL) {
	if (strcmp(tt+1, "FLAT") == 0) {
	    is_flat[DATA_TYPE_PHSEG] = TRUE;
	    *tt = '\0';
	}
	else if (strcmp(tt+1, "CTL") == 0) {
	    is_flat[DATA_TYPE_PHSEG] = FALSE;
	    *tt = '\0';
	}
	else {
	    E_INFO("Assuming ',' in phseg dir is part of a pathname\n");
	    
	    is_flat[DATA_TYPE_PHSEG] = FALSE;
	}
    }
    else {
	is_flat[DATA_TYPE_PHSEG] = FALSE;
    }

    data_dir[DATA_TYPE_PHSEG] = dir;

    return S3_SUCCESS;
}

/*********************************************************************
 *
 * Function: corpus_set_phseg_ext
 * 
 * Description: 
 *    Set the file name extension for the phone segmentation files
 * 
 * Function Inputs: 
 *    const char *ext -
 *	This is the file name extension for the phone segmentation file names.
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
corpus_set_phseg_ext(const char *ext)
{
    extension[DATA_TYPE_PHSEG] = ext;

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

    assert(transcription_filename == NULL);

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

    transcription_filename = fn;

    requires_sent = TRUE;

    transcription_fp = fopen(transcription_filename, "r");
    if (transcription_fp == NULL) {
	E_FATAL_SYSTEM("Can not open transcription file '%s'", transcription_filename);
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
	(transcription_fp == NULL) &&
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

    if (requires_phseg &&
	extension[DATA_TYPE_PHSEG] == NULL) {
	E_ERROR("No phseg extension given\n");

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
    lineiter_t *li;
    
    if (cur_ctl_path) {
	free(cur_ctl_path);
    }
    cur_ctl_path = next_ctl_path;

    if (cur_ctl_utt_id) {
	free(cur_ctl_utt_id);
	cur_ctl_utt_id = NULL;
    }
    cur_ctl_utt_id = next_ctl_utt_id;
    
    cur_ctl_sf = next_ctl_sf;
    cur_ctl_ef = next_ctl_ef;

    if (n_run != UNTIL_EOF) {
	if (n_run == 0) return FALSE;

	--n_run;
    }

    ++n_proc;

    if (cur_ctl_path == NULL || strlen(cur_ctl_path) == 0)
	return FALSE;

    /* if a big LSN file exists, position it to the correct line
     * corpus_set_ctl_filename() reads the first line of
     * the control file, so that transcription_fp is one line
     * behind ctl_fp. */
    if (transcription_fp) {
	lineiter_t *trans_li;
	trans_li = lineiter_start_clean(transcription_fp);
	if (trans_li == NULL) {
	    E_FATAL("File length mismatch at line %d in %s\n", n_proc, transcription_filename);
	}
	if (transcription_line)
	    free(transcription_line);
	transcription_line = strdup(trans_li->buf);
	lineiter_free(trans_li);
    }  

    li = lineiter_start_clean(ctl_fp);

    if (li != NULL) {
        parse_ctl_line(li->buf,
                       &next_ctl_path,
                       &next_ctl_sf,
                       &next_ctl_ef,
                       &next_ctl_utt_id);
        lineiter_free (li);
    } else {
        next_ctl_path = NULL;
        next_ctl_sf = NO_FRAME;
        next_ctl_ef = NO_FRAME;
        next_ctl_utt_id = NULL;
    }

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
	E_ERROR_SYSTEM("Unable to open %s for reading\n", fn);
    }
    
    return out;
}

static int
corpus_read_next_sent_file(char **trans)
{
    FILE *fp;
    lineiter_t *li;

    /* open the current file */
    fp = open_file_for_reading(DATA_TYPE_SENT);

    li = lineiter_start_clean(fp);
    if (li == NULL) {
	E_ERROR("Unable to read data in sent file %s\n",
		mk_filename(DATA_TYPE_SENT, cur_ctl_path));		
	return S3_ERROR;
    }

    *trans = strdup(li->buf);
    lineiter_free(li);
    fclose(fp);

    return S3_SUCCESS;
}

int
corpus_get_generic_featurevec(vector_t **mfc,
			      int32 *n_frame,
			      uint32 veclen)
{
    vector_t *out;
    float32 *coeff, **cptr;
    uint32 n_f;
    uint32 n_c;
    uint32 i, j;
    uint32 ret=S3_ERROR;
    uint32 no_retries=0;

    if (!requires_mfcc) {
	/* asked for mfc data, but not set up to send it */
	return S3_ERROR;
    }

    if (mfc)
	cptr = &coeff;
    else {
	/* If mfc == NULL, just get the number of frames. */
	coeff = NULL;
	cptr = NULL;
    }

    do {
	if ((cur_ctl_sf == NO_FRAME) && (cur_ctl_ef == NO_FRAME)) {
	    ret = areadfloat(mk_filename(DATA_TYPE_MFCC, cur_ctl_path),
			     cptr, (int *)&n_c);
	}
	else if ((cur_ctl_sf != NO_FRAME) && (cur_ctl_ef != NO_FRAME)) {
	    ret = areadfloat_part(mk_filename(DATA_TYPE_MFCC, cur_ctl_path),
				  cur_ctl_sf * veclen,
				  (cur_ctl_ef + 1) * veclen - 1,
				  cptr, (int *)&n_c);
	}
	else {
	    E_FATAL("Both start and end frame must be set in the ctl file\n");
	}

	if (ret == S3_ERROR) {
	    E_ERROR_SYSTEM("Failed to read MFC file '%s'. Retrying after sleep...\n",
		    mk_filename(DATA_TYPE_MFCC, cur_ctl_path));
	    no_retries++;
	    sleep(3);
	    if(no_retries>100){ 
	      E_FATAL("Failed to get the files after 100 retries (about 300 seconds)\n");
	    }
	}
    } while (ret == S3_ERROR);

    if ((ret == 0) && (cur_ctl_sf != NO_FRAME) && (cur_ctl_ef != NO_FRAME)) {
	E_ERROR("Region [%d %d] for %s extends beyond end of file\n",
		cur_ctl_sf, cur_ctl_ef, corpus_utt());
    }

    if ((n_c % veclen) != 0) {
	E_FATAL("Expected mfcc vector len of %d, got %d (%d)\n", veclen, n_c % veclen, n_c);
    }
    
    n_f = n_c / veclen;

    if (n_f == 0) {
	if (mfc)
	    *mfc = NULL;
	if (n_frame)
	    *n_frame = 0;
    }

    if (mfc && coeff) {
	out = (vector_t *)ckd_calloc(n_f, sizeof(vector_t));

	for (i = 0, j = 0; i < n_f; i++, j += veclen) {
	    out[i] = &coeff[j];
	}

	*mfc = out;
    }
    if (n_frame)
	*n_frame = n_f;

    return S3_SUCCESS;
}

int
corpus_get_seg(uint16 **seg,
	       int32 *n_seg)
{
    char *rel_path;

    if (!requires_seg) {
	/* asked for seg data, but not set up to send it */
	return S3_ERROR;
    }

    /* If control file specifies an utt ID, use it.  O/W use the path */
    if (cur_ctl_utt_id != NULL)
	rel_path = cur_ctl_utt_id;
    else
	rel_path = cur_ctl_path;

    if (areadshort(mk_filename(DATA_TYPE_SEG, rel_path), (int16**)seg, n_seg) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

int
corpus_get_phseg(acmod_set_t *acmod_set,
		 s3phseg_t **out_phseg)
{
    char *rel_path;

    if (!requires_phseg) {
	/* asked for seg data, but not set up to send it */
	return S3_ERROR;
    }

    /* If control file specifies an utt ID, use it.  O/W use the path */
    if (cur_ctl_utt_id != NULL)
	rel_path = cur_ctl_utt_id;
    else
	rel_path = cur_ctl_path;

    if (s3phseg_read(mk_filename(DATA_TYPE_PHSEG, rel_path),
		     acmod_set,
		     out_phseg) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}

static int
corpus_read_next_transcription_line(char **trans)
{
    char utt_id[512];
    char *s;

    /* look for a close paren in the line */
    s = strrchr(transcription_line, ')');

    if (s != NULL) {
	int nspace;
	/* found a close paren */

	nspace = strspn(s + 1, " \t\r\n");
	if (s[nspace + 1] == '\0') {
	    /* it is at the end of the line */
	    *s = '\0';		/* terminate the string at the paren */

	    /* search for a matching open paren */
	    for (s--; (s >= transcription_line) && (*s != '('); s--);

	    if (*s == '(') {
		/* found a matching open paren */

		assert(strlen(s+1) < 512);

		strcpy(utt_id, s+1);

		if (strcmp_nocase(utt_id, corpus_utt()) != 0) {
		    char *uttfullname = corpus_utt_full_name();
		    int suffpos = strlen(uttfullname) - strlen(utt_id);

		    if (suffpos >= 0 && strlen(utt_id) > 0 && strcmp_nocase(&uttfullname[suffpos], utt_id) != 0) {
		        E_WARN("Utterance id in transcription file, '%s', does not match filename in control path '%s'.\n",
			       utt_id, uttfullname);
		    }
		}

		/* look for the first non-whitespace character before
		   the open paren */
		for (--s; (s >= transcription_line) && isspace((unsigned char)*s); s--);
		if (s < transcription_line) {
		    E_FATAL("Utterance transcription is empty: %s\n", transcription_line);
		}
		++s;
		*s = '\0';	/* terminate the string at the first whitespace character
				   following the first non-whitespace character found above */
	    }
	    else {
		E_ERROR("Expected open paren after ending close paren in line: '%s'\n", transcription_line);
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

    *trans = strdup(transcription_line);
    
    return S3_SUCCESS;
}

int
corpus_get_sent(char **trans)
{
  if (transcription_fp == NULL)
    return corpus_read_next_sent_file(trans);
  else
    return corpus_read_next_transcription_line(trans);
}

int
corpus_has_xfrm()
{
    return 0;
}

int
corpus_get_xfrm(float32 *****out_a,
		float32 ****out_b,
		uint32 **out_veclen,
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


int
corpus_load_lattice(s3lattice_t **out_lattice, const char *lat_dir, const char *lat_ext)
{
    char *rel_path;
    char fn[1024];

    /* If control file specifies an utt ID, use it.  O/W use the path */
    if (cur_ctl_utt_id != NULL)
	rel_path = cur_ctl_utt_id;
    else {
	rel_path = cur_ctl_path;
    }

    sprintf(fn, "%s/%s.%s",  lat_dir, rel_path, lat_ext);
    if (s3lattice_read(fn, out_lattice) < 0)
	return S3_ERROR;
    
    return S3_SUCCESS;
}
