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
 * File: parse_cmd_ln.c
 * 
 * Description: 
 * 	These routines define and parse the command line for init_gau.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/feat.h>
#include <s3/s3.h>

#include <sys_compat/misc.h>

#include <stdio.h>
#include <assert.h>



/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] = 
"Description: (Copy from Rita's web manual) To initialize the means and variances, global values of \n\
 these parameters are first estimated and then copied into appropriate \n\
 positions in the parameter files. The global mean is computed using \n\
 all the vectors you have in your feature files. This is usually a \n\
 very large number, so the job is divided into many parts. At this \n\
 stage you tell the Sphinx how many parts you want it to divide this \n\
 operation into (depending on the computing facilities you have) and \n\
 the Sphinx \"accumulates\" or gathers up the vectors for each part \n\
 separately and writes it into an intermediate buffer on your \n\
 machine. The executable init_gau is used for this purpose.";

  const char examplestr[]=
"Example:\n\
\n\
init_gau -accumdir accumdir -ctlfn controlfn -part 1 -npart 1 -cepdir cepdir -feat 1s_12c_12d_3p_12dd -ceplen 13 ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-moddeffn",
	  ARG_STRING,
	  NULL,
	  "Model definition file for the single density HMM's to initialize"},
	{ "-ts2cbfn",
	  ARG_STRING,
	  NULL,
	  "Tied-state-to-codebook mapping file"},
	{ "-accumdir",
	  ARG_STRING,
	  NULL,
	  "Where to write mean/var counts"},
	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "Mean file for variance initialization"},
	{ "-fullvar",
	  ARG_BOOLEAN,
	  "no",
	  "Accumulate for full covariance matrices"},
	{ "-ctlfn",
	  ARG_STRING,
	  NULL,
	  "Control file of the training corpus"},
	{ "-nskip",
	  ARG_INT32,
	  NULL,
	  "# of lines to skip in the control file"},
	{ "-runlen",
	  ARG_INT32,
	  NULL,
	  "# of lines to process in the control file (after any skip)"},
	{ "-part",
	  ARG_INT32,
	  NULL,
	  "Identifies the corpus part number (range 1..NPART)" },
	{ "-npart",
	  ARG_INT32,
	  NULL,
	  "Partition the corpus into this many equal sized subsets" },
	{ "-lsnfn",
	  ARG_STRING,
	  NULL,
	  "All word transcripts for the training corpus (consistent order w/ -ctlfn!)"},
	{ "-dictfn",
	  ARG_STRING,
	  NULL,
	  "Dictionary for the content words"},
	{ "-fdictfn",
	  ARG_STRING,
	  NULL,
	  "Dictionary for the filler words"},
	{ "-segdir",
	  ARG_STRING,
	  NULL,
	  "Root directory of the training corpus state segmentation files."},
	{ "-segext",
	  ARG_STRING,
	  "v8_seg",
	  "Extension of the training corpus state segmentation files."},

	{ "-scaleseg",
	  ARG_BOOLEAN,
	  "no",
	  "Scale existing segmentation to fit new parameter stream length."},
	{ "-cepdir",
	  ARG_STRING,
	  NULL,
	  "Root directory of the training corpus cepstrum files."},

	{ "-cepext",
	  ARG_STRING,
	  "mfc",
	  "Extension of the training corpus cepstrum files."},

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
