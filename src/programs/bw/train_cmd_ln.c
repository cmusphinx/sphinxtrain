/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * File: train_cmd_ln.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "train_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <sys_compat/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

int
validate_writeable_dir(char *switch_name, void *arg)
{
#ifndef WIN32
    char *path = arg;
    struct stat s;

    if (path == NULL) {
	E_ERROR("%s is a necessary switch\n", switch_name);

	return FALSE;
    }

    if (stat(path, &s) < 0) {
	E_ERROR("%s %s does not exist or is inaccessible\n", switch_name, path);

	return FALSE;
    }

    if (!S_ISDIR(s.st_mode)) {
	E_ERROR("%s %s is not a directory\n", switch_name, path);

	return FALSE;
    }	

    if ((s.st_mode && S_IWOTH) ||
	((s.st_uid == getuid()) && (s.st_mode && S_IWUSR)) ||
	((s.st_gid == getgid()) && (s.st_mode && S_IWGRP))) {
	return TRUE;
    }
    else {
	E_ERROR("%s %s is not writeable\n", switch_name, path);

	return FALSE;
    }
#else
    /* WIN32 */

    /* Do no validation for now.  Need to figure out WIN32 compatible way */

    return TRUE;
#endif
}
int
validate_opt_writeable_dir(char *switch_name, void *arg)
{
#ifndef WIN32
    char *path = arg;
    struct stat s;

    if (path == NULL) {
	return TRUE;
    }

    if (stat(path, &s) < 0) {
	E_ERROR("%s %s does not exist or is inaccessible\n", switch_name, path);

	return FALSE;
    }

    if (!S_ISDIR(s.st_mode)) {
	E_ERROR("%s %s is not a directory\n", switch_name, path);

	return FALSE;
    }	

    if ((s.st_mode && S_IWOTH) ||
	((s.st_uid == getuid()) && (s.st_mode && S_IWUSR)) ||
	((s.st_gid == getgid()) && (s.st_mode && S_IWGRP))) {
	return TRUE;
    }
    else {
	E_ERROR("%s %s is not writeable\n", switch_name, path);

	return FALSE;
    }
#else
    /* WIN32 */

    /* Do no validation for now.  Need to figure out WIN32 compatible way */

    return TRUE;
#endif
}

int
validate_readable_dir(char *switch_name, void *arg)
{
#ifndef WIN32
    char *path = arg;
    struct stat s;

    if (path == NULL) {
	E_ERROR("%s is a necessary switch\n", switch_name);

	return FALSE;
    }

    if (stat(path, &s) < 0) {
	E_ERROR("%s %s does not exist or is inaccessible\n", switch_name, path);

	return FALSE;
    }

    if (!S_ISDIR(s.st_mode)) {
	E_ERROR("%s %s is not a directory\n", switch_name, path);

	return FALSE;
    }	

    if ((s.st_mode && S_IROTH) ||
	((s.st_uid == getuid()) && (s.st_mode && S_IRUSR)) ||
	((s.st_gid == getgid()) && (s.st_mode && S_IRGRP))) {
	return TRUE;
    }
    else {
	E_ERROR("%s %s is not readable\n", switch_name, path);

	return FALSE;
    }
#else
    /* WIN32 */

    /* Do no validation for now.  Need to figure out a WIN32 compatible
       way */

    return TRUE;
#endif
}

int
validate_agc(char *switch_name, void *arg)
{
    if ((strcmp(arg, "max") == 0) || (strcmp(arg, "emax") == 0) || (strcmp(arg, "none") == 0)) {
	return TRUE;
    }
    else {
	E_ERROR("Unknown agc type %s %s\n", switch_name, arg);

	return FALSE;
    }

    assert(FALSE);
}

int
validate_cmn(char *switch_name, void *arg)
{

    if ((strcmp(arg, "current") == 0) ||
	(strcmp(arg, "none") == 0) ||
	(strcmp(arg, "prior") == 0)) {
	return TRUE;
    }
    else {
	E_ERROR("Unknown CMN type %s %s\n", switch_name, arg);
    }
    return TRUE;
}

int
validate_silcomp(char *switch_name, void *arg)
{

    if ((strcmp(arg, "none") == 0) ||
	(strcmp(arg, "sildelfn") == 0) ||
	(strcmp(arg, "prior") == 0) ||
	(strcmp(arg, "current") == 0)) {
	return TRUE;
    }
    else {
	E_ERROR("Unknown silence compression type %s %s\n", switch_name, arg);
	
	return FALSE;
    }
    
    assert(FALSE);
}

/* defines, parses and (partially) validates the arguments
   given on the command line */

int
train_cmd_ln_parse(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The model definition file for the model inventory to train" },

	{ "-tmatfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The transition matrix parameter file name"},

	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The mixture weight parameter file name"},

	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The mean parameter file name"},

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The var parameter file name"},

	{ "-mwfloor",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.00001",
	  "Mixing weight smoothing floor" },

	{ "-tpfloor",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.0001",
	  "Transition probability smoothing floor" },

	{ "-varfloor",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.00001",
	  "The minimum variance"},
	
	{ "-topn",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "4",
	  "Compute output probabilities based this number of top scoring densities."},

	{ "-dictfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The content word dictionary" },

	{ "-fdictfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The filler word dictionary (e.g. SIL, SILb, ++COUGH++)" },

	{ "-ctlfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The training corpus control file" },

	{ "-nskip",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The number of utterances to skip at the beginning of a control file" },

	{ "-runlen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "-1",	/* until eof */
	  "The number of utterances to process in the (skipped) control file" },

	{ "-part",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Identifies the corpus part number (range 1..NPART)" },

	{ "-npart",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Partition the corpus into this many equal sized subsets" },

	{ "-cepext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "mfc",
	  "The cepstrum file extension" },

	{ "-cepdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The cepstrum data root directory" },

	{ "-segext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "v8_seg",
	  "State segmentation file extension" },

	{ "-segdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "State segmentation file root directory" },
	
	{ "-sentdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The sentence transcript file directory"},

	{ "-sentext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "sent",
	  "The sentence transcript file extension"},

	{ "-lsnfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The corpus word transcript file"},

	{ "-accumdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A path where accumulated counts are to be written." },

	{ "-ceplen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "13",
	  "The length of the input feature (e.g. MFCC) vectors"},

	{ "-agc",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "max",
	  "The type of automatic gain control to do {max|emax}"},

	{ "-cmn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "current",
	  "The do cepstral mean normalization based on {current|prior} utterance(s)"},

	{ "-varnorm",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Variance Normalize?"},

	{ "-silcomp",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "none",
	  "Do silence compression based on {current|prior} utterance"},

	/* By ARCHAN at 200, fix the long-time legacy problem of not able to delete silence*/
	{ "-sildel",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Allow optional silence deletion in the Baum-Welch algorithm or the Viterbi algorithm."},
	
	{ "-siltag",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "SIL",
	  "Specify the tag of silence, by default it is <sil>."},

	{ "-abeam",
	  CMD_LN_FLOAT64,
	  CMD_LN_NO_VALIDATION,
	  "1e-100",
	  "Evaluate alpha values subject to this beam"},

	{ "-bbeam",
	  CMD_LN_FLOAT64,
	  CMD_LN_NO_VALIDATION,
	  "1e-100",
	  "Evaluate beta values (update reestimation sums) subject to this beam"},

	{ "-varreest",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Reestimate variances"},

	{ "-meanreest",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Reestimate means"},

	{ "-mixwreest",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Reestimate mixing weights"},

	{ "-tmatreest",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Reestimate transition probability matrices"},

	{ "-spkrxfrm",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A speaker transform to use for SAT modelling"},

	{ "-mllrmult",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Reestimate multiplicative term of MLLR adatpation of means"},

	{ "-mllradd",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Reestimate shift term of MLLR adaptation of means"},

	{ "-ts2cbfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Tied-state-to-codebook mapping file name" },

	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "This argument selects the derived feature computation to use."},

	{ "-timing",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Controls whether profiling information is displayed"},
	
	{ "-viterbi",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Controls whether Viterbi training is done"},
	
	{ "-2passvar",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Reestimate variances based on prior means"},

	{ "-sildelfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "File which specifies frames of background 'silence' to delete" },

	{ "-cb2mllrfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Codebook-to-MLLR-class mapping file name" },

	{ "-spthresh",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.0",
	  "State posterior probability floor for reestimation.  States below this are not counted"},
	
	{ "-maxuttlen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "0",
	  "Maximum # of frames for an utt ( 0 => no fixed limit )"},
	
	{ "-ckptintv",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Checkpoint the reestimation sums every -chkptintv utts" },
	
	{ NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
    };

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    if (cmd_ln_validate() == FALSE) {
	E_FATAL("Unable to validate command line arguments\n");
    }

    cmd_ln_print_configuration();

    return 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.7  2004/07/17  08:00:23  arthchan2003
 * deeply regretted about one function prototype, now revert to the state where multiple pronounciations code doesn't exist
 * 
 * Revision 1.5  2004/06/17 19:17:14  arthchan2003
 * Code Update for silence deletion and standardize the name for command -line arguments
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
 * Revision 1.14  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.13  1996/08/06  14:03:47  eht
 * -sildelfn argument to specify silence deletion list
 *
 * Revision 1.12  1996/07/29  16:18:48  eht
 * Make -accumdir optional so that it may be omitted for
 * debugging purposes
 * MLLR command line options
 * -veclen to -ceplen
 * -minvar to -varfloor (now named consistently w/ the other floors)
 * added -2passvar switch to allow reestimation based on prior means
 *
 * Revision 1.11  1996/03/26  14:03:24  eht
 * - Added '-timing' argument
 * - changed doc strings for some arguments
 *
 * Revision 1.10  1996/02/02  17:41:47  eht
 * Add alpha and beta beams
 *
 * Revision 1.9  1996/01/26  18:23:49  eht
 * Reformatted argument specifications
 *
 * Revision 1.8  1995/11/30  20:42:07  eht
 * Add argument for transition matrix reestimation
 * Add argument for state parameter definition file
 *
 *
 */

