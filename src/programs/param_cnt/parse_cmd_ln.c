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
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/s3.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] =
"Description:  \n\
\n\
Find the number of times each of the triphones listed \n\
\n\
in a given model definition file (by -moddeffhn) occured in a set \n\
of transcription, specified by -lsnfn";

  const char examplestr[] =
"Example : \n\
param_cnt -moddeffn mdef -ts2cbfn .cont. -ctlfn controlfile -lsnfn \n\
transcripts -dictfn dict -fdictfn fillerdict -paramtype phone";

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

	{ "-ctlfn",
	  ARG_STRING,
	  NULL,
	  "Control file of the training corpus"},

	{ "-part",
	  ARG_INT32,
	  NULL,
	  "Identifies the corpus part number (range 1..NPART)" },

	{ "-npart",
	  ARG_INT32,
	  NULL,
	  "Partition the corpus into this many equal sized subsets" },

	{ "-nskip",
	  ARG_INT32,
	  NULL,
	  "# of lines to skip in the control file"},

	{ "-runlen",
	  ARG_INT32,
	  NULL,
	  "# of lines to process in the control file (after any skip)"},

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

	{ "-paramtype",
	  ARG_STRING,
	  "state",
	  "Parameter type to count {'state', 'cb', 'phone'}"},

	{ "-outputfn",
	  ARG_STRING,
	  NULL,
	  "If specified, write counts to this file"},

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
