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
"Description:\n\
\n\
Given a set of questions. Build decision tree for a set of feature of\n\
a particular phone.  By default, decision tree are not built for \n\
filler phones and the phone tagged with SIL.  One very confusing \n\
parameters of this tool is -stwt, if you are training a n-state \n\
HMM, you need to specify n values after this flag.  ";

    const char examplestr[] =
" bld_tree -treefn tree -moddeffn mdef -mixwfn mixw -meanfn mean -varfn \n\
var -psetfn questions -stwt 1.0 0.05 0.01 -state 0 -ssplitmin 1 \n\
-ssplitmax 7 -ssplitthr 1e-10 -csplitmin 1 -csplitmax 2000 -csplitthr \n\
1e-10 -cntthresh 10";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-treefn",
	  ARG_STRING,
	  NULL,
	  "Name of output tree file to produce" },

	{ "-moddeffn",
	  ARG_STRING,
	  NULL,
	  "Model definition file of the discrete models" },

        { "-ts2cbfn",
          ARG_STRING,
          ".semi.",
          "The type of models to build trees on" },

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
          "0.00001",
          "The minimum variance"},

        { "-cntthresh",
          ARG_FLOAT32,
          "0.00001",
          "Ignore all states with counts less than this"},

	{ "-mixwfn",
	  ARG_STRING,
	  NULL,
	  "PDF's for tree building using semicontinuous HMMs" },

	{ "-psetfn",
	  ARG_STRING,
	  NULL,
	  "phone set definitions for phonetic questions" },

	{ "-phone",
	  ARG_STRING,
	  NULL,
	  "Build trees over n-phones having this base phone"},

	{ "-allphones",
	  ARG_BOOLEAN,
	  "no",
	  "Build trees over all n-phones"},

	{ "-state",
	  ARG_INT32,
	  NULL,
	  "Build tree for this state position. E.g. For a three state HMM, this value can be 0,1 or 2. For a 5 state HMM, this value can be 0,1,2,3 or 4, and so on "},

	{ "-mwfloor",
	  ARG_FLOAT32,
	  "1e-4",
	  "Mixing weight floor for tree building"},

	{ "-stwt",
	  ARG_STRING_LIST,
	  NULL,
	  "Weights on neighboring states, This flag needs a string of numbers equal to the number of HMM-states"},

	{ "-ssplitthr",
	  ARG_FLOAT32,
	  "8e-4",
	  "Simple node splitting threshold" },

	{ "-ssplitmin",
	  ARG_INT32,
	  "1",
	  "Minimum of simple tree splits to do."},

	{ "-ssplitmax",
	  ARG_INT32,
	  "5",
	  "The maximum number of bifurcations in the simple tree before it is used to build complex questions."},

	{ "-csplitthr",
	  ARG_FLOAT32,
	  "8e-4",
	  "Compound node splitting threshold" },

	{ "-csplitmin",
	  ARG_INT32,
	  "1",
	  "Minimum # of compound tree splits to do" },

	{ "-csplitmax",
	  ARG_INT32,
	  "100",
	  "Minimum # of compound tree splits to do" },

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
