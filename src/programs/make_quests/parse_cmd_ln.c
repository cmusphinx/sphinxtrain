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
 *
 * File: cmd_ln.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>

#include <stdio.h>
#include <stdlib.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

    const char helpstr[] =  
"Description: \n\
\n\
    This is an implementation of Dr. Rita Singh's automatic question \n\
    generation.  (Copied from Rita's comment) The current algorithm \n\
    clusters CI distributions using a hybrid bottom-up top-down \n\
    clustering algorithm to build linguistic questions for decision \n\
    trees.\n\
    (From Arthur : I need to do some tracing before understand it \n\
    what's the internal of the code)";

    const char examplestr[] =
"Example: \n\
make_quest -moddeffn mdef -meanfn mean -varfn var -mixwfn \n\
mixwfn -npermute 8 -niter 1 -qstperstt 20 -questfn \n\
questions -type .cont.";


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
	  "Model definition file of the ci models" },

	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "means file for tree building using continuous HMMs" },

	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "variances file for tree building using continuous HMMs" },

	{ "-fullvar",
	  ARG_BOOLEAN,
	  "no",
	  "variances file contains full covariances" },

        { "-varfloor",
          ARG_FLOAT32,
          "1.0e-08",
          "The minimum variance"},

	{ "-mixwfn",
	  ARG_STRING,
	  NULL,
	  "PDF's for tree building using semicontinuous HMMs" },

        { "-npermute",
          ARG_INT32,
          "6",
          "The minimum variance"},

        { "-niter",
          ARG_INT32,
          "0",
          "Number of iterations"},

        { "-qstperstt",
          ARG_INT32,
          "8",
          "something per state"},

	{ "-tempfn",
	  ARG_STRING,
	  "/tmp/TEMP.QUESTS",
	  "(Obsolete) File to write temprorary results to " },

	{ "-questfn",
	  ARG_STRING,
	  NULL,
	  "File to write questions to" },

	{ "-type",
	  ARG_STRING,
	  NULL,
	  "HMM type" },

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
