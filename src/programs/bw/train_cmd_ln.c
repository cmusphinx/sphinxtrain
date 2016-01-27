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
 * File: train_cmd_ln.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "train_cmd_ln.h"

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/feat.h>
#include <sphinxbase/err.h>

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
#ifndef _WIN32
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
    /* _WIN32 */

    /* Do no validation for now.  Need to figure out _WIN32 compatible way */

    return TRUE;
#endif
}
int
validate_opt_writeable_dir(char *switch_name, void *arg)
{
#ifndef _WIN32
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
    /* _WIN32 */

    /* Do no validation for now.  Need to figure out _WIN32 compatible way */

    return TRUE;
#endif
}

int
validate_readable_dir(char *switch_name, void *arg)
{
#ifndef _WIN32
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
    /* _WIN32 */

    /* Do no validation for now.  Need to figure out a _WIN32 compatible
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


/* defines, parses and (partially) validates the arguments
   given on the command line */

int
train_cmd_ln_parse(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] =
"Description:\n\
Strictly speaking,  bw only implements the first-part of the Baum-Welch \n\
algorithm.  That is it go through forward and backward algortihm and\n\
collect the necessary statistics for parameter estimation.\n\
\n\
The advantage of this architecture is that researcher can easily write \n\
programs to do parameter estimation and they have no need to tweak the \n\
huge and usually difficult Baum-Welch algorithm. \n\
\n\
In terms of functionality, one important thing you need to know is option \n\
-part and -npart.  They can allow you to split the training into N equal parts\n\
Say, if there are M utterances in your control file, then this \n\
will enable you to run the training separately on each (M/N)th \n\
part. This flag may be set to specify which of these parts you want to \n\
currently train on. As an example, if your total number of parts (-npart) is 3, \n\
-part can take one of the values 1,2 or 3.  \n\
\n\
To control the speed of the training, -abeam (control alpha search) \n\
and -bbeam (control beta search) can be used to control the searching \n\
time.  Notice that if the beams are too small, the path may not reach \n\
the end of the search and results in estimation error \n\
Too many lost path may also cause training set likelihood not unable to increase \n\
\n\
Several options allow the user to control the behaviour of bw such \n\
that silence or pauses can be taken care\n\
\n\
Finally, one can use the viterbi training mode of the code.  Notice \n\
though, the code is not always tested by CMU's researcher \n\
\n\
I also included the following paragraph from Rita's web page. ";

  const char examplestr[]=
"Example: \n\
Command used to train continuous HMM \n\
(Beware, this only illustrates how to use this command, for detail on \n\
how to tune it, please consult the manual. ) \n\
bw \n\
-moddeffn mdef -ts2cbfn .cont.\n\
-mixwfn mixw -tmatfn tmatn -meanfn mean -varfn var\n\
-dictfn dict -fdictfn fillerdict \n\
-ctlfn control_files \n\
-part 1 -npart 1 \n\
-cepdir feature_dir -cepext mfc \n\
-lsnfn transcription \n\
-accumdir accumdir \n\
-abeam 1e-200 -bbeam 1e-200 \n\
-meanreest yes -varreest yes \n\
-tmatreest yes -feat 1s_12c_12d_3p_12dd \n\
-ceplen 13 \n\
\n\
If yo want to do parallel training for N machines. Run N trainers with \n\
-part 1 -npart N \n\
-part 2 -npart N \n\
.\n\
.\n\
-part N -npart N ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-hmmdir",
	  ARG_STRING,
	  NULL,
	  "Default directory for acoustic model files (mdef, means, variances, transition_matrices, noisedict)" },

	{ "-moddeffn",
	  ARG_STRING,
	  NULL,
	  "The model definition file for the model inventory to train" },

	{ "-tmatfn",
	  ARG_STRING,
	  NULL,
	  "The transition matrix parameter file name"},

	{ "-mixwfn",
	  ARG_STRING,
	  NULL,
	  "The mixture weight parameter file name"},

	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "The mean parameter file name"},

	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "The var parameter file name"},

	{ "-fullvar",
	  ARG_BOOLEAN,
	  "no",
	  "Variances are full covariance matrices"},

	{ "-diagfull",
	  ARG_BOOLEAN,
	  "no",
	  "Evaluate Gaussian densities using diagonals only"},

	{ "-mwfloor",
	  ARG_FLOAT32,
	  "0.00001",
	  "Mixing weight smoothing floor" },

	{ "-tpfloor",
	  ARG_FLOAT32,
	  "0.0001",
	  "Transition probability smoothing floor" },

	{ "-varfloor",
	  ARG_FLOAT32,
	  "0.00001",
	  "The minimum variance"},
	
	{ "-topn",
	  ARG_INT32,
	  "4",
	  "Compute output probabilities based this number of top scoring densities."},

	{ "-dictfn",
	  ARG_STRING,
	  NULL,
	  "The content word dictionary" },

	{ "-fdictfn",
	  ARG_STRING,
	  NULL,
	  "The filler word dictionary (e.g. SIL, SILb, ++COUGH++)" },

	{ "-ctlfn",
	  ARG_STRING,
	  NULL,
	  "The training corpus control file" },

	{ "-nskip",
	  ARG_INT32,
	  NULL,
	  "The number of utterances to skip at the beginning of a control file" },

	{ "-runlen",
	  ARG_INT32,
	  "-1",	/* until eof */
	  "The number of utterances to process in the (skipped) control file" },

	{ "-part",
	  ARG_INT32,
	  NULL,
	  "Identifies the corpus part number (range 1..NPART)" },

	{ "-npart",
	  ARG_INT32,
	  NULL,
	  "Partition the corpus into this many equal sized subsets" },

	{ "-cepext",
	  ARG_STRING,
	  "mfc",
	  "The cepstrum file extension" },

	{ "-cepdir",
	  ARG_STRING,
	  NULL,
	  "The cepstrum data root directory" },

	{ "-phsegext",
	  ARG_STRING,
	  "phseg",
	  "Phone segmentation file extension" },

	{ "-phsegdir",
	  ARG_STRING,
	  NULL,
	  "Phone segmentation file root directory" },

	{ "-outphsegdir",
	  ARG_STRING,
	  NULL,
	  "Phone segmentation file output root directory" },

	{ "-sentdir",
	  ARG_STRING,
	  NULL,
	  "The sentence transcript file directory"},

	{ "-sentext",
	  ARG_STRING,
	  "sent",
	  "The sentence transcript file extension"},

	{ "-lsnfn",
	  ARG_STRING,
	  NULL,
	  "The corpus word transcript file"},

	{ "-accumdir",
	  ARG_STRING,
	  NULL,
	  "A path where accumulated counts are to be written." },

	{ "-abeam",
	  ARG_FLOAT64,
	  "1e-100",
	  "Evaluate alpha values subject to this beam"},

	{ "-bbeam",
	  ARG_FLOAT64,
	  "1e-100",
	  "Evaluate beta values (update reestimation sums) subject to this beam"},

	{ "-varreest",
	  ARG_BOOLEAN,
	  "yes",
	  "Reestimate variances"},

	{ "-meanreest",
	  ARG_BOOLEAN,
	  "yes",
	  "Reestimate means"},

	{ "-mixwreest",
	  ARG_BOOLEAN,
	  "yes",
	  "Reestimate mixing weights"},

	{ "-tmatreest",
	  ARG_BOOLEAN,
	  "yes",
	  "Reestimate transition probability matrices"},

	{ "-mllrmat",
	  ARG_STRING,
	  NULL,
	  "An MLLR transformation file to apply to the means of the model"},

	{ "-cb2mllrfn",
	  ARG_STRING,
	  ".1cls.",
	  "Codebook-to-MLLR-class mapping file name" },

	{ "-ts2cbfn",
	  ARG_STRING,
	  NULL,
	  "Tied-state-to-codebook mapping file name" },

	{ "-timing",
	  ARG_BOOLEAN,
	  "yes",
	  "Controls whether profiling information is displayed"},
	
	{ "-viterbi",
	  ARG_BOOLEAN,
	  "no",
	  "Controls whether Viterbi training is done"},
	
	{ "-2passvar",
	  ARG_BOOLEAN,
	  "no",
	  "Reestimate variances based on prior means"},

	{ "-spthresh",
	  ARG_FLOAT32,
	  "0.0",
	  "State posterior probability floor for reestimation.  States below this are not counted"},
	
	{ "-maxuttlen",
	  ARG_INT32,
	  "0",
	  "Maximum # of frames for an utt ( 0 => no fixed limit )"},
	
	{ "-ckptintv",
	  ARG_INT32,
	  NULL,
	  "Checkpoint the reestimation sums every -chkptintv utts" },
	
	{ "-outputfullpath",
	  ARG_BOOLEAN,
	  "no",
	  "Output full path of utterance to bw log output" },

	{ "-pdumpdir",
	  ARG_STRING,
	  NULL,
	  "Dump state/mixture posterior probabilities to files in this directory" },

	{ "-latdir",
	  ARG_STRING,
	  NULL,
	  "Directory that contains lattice files" },

	{ "-mmie",
	  ARG_BOOLEAN,
	  "no",
	  "Whether to do MMIE training or not" },

	{ "-mmie_type",
	  ARG_STRING,
	  "rand",
	  "how to get different context for Viterbi run on lattice, such as rand, best or ci. \n\
           \t\t\trand: randomly take the left and right context \n\
           \t\t\tbest: take the left and right context with the best acoustic score \n\
           \t\t\tci:   use context-independent hmm for word boundary models" },

	{ "-latext",
	  ARG_STRING,
	  NULL,
	  "Denominator or Numerator lattice. Use denlat or numlat" },

	{ "-lw",
	  ARG_FLOAT32,
	  "11.5",
	  "Language model weight" },
	/* end */
	
	cepstral_to_feature_command_line_macro(),
	{NULL, 0, NULL, NULL}
    };

    cmd_ln_parse(defn, argc, argv, 1);


    isHelp    = cmd_ln_int32("-help");
    isExample    = cmd_ln_int32("-example");

    if(isHelp){
      printf("%s\n\n",helpstr);
    }

    if(isExample){
      printf("%s\n\n",examplestr);
    }

    if(isHelp || isExample){
      E_INFO("User asked for help or example.\n");
      exit(0);
    }

    return 0;
}

