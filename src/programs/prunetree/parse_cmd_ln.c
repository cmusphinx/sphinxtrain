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
 Using prunetree, the bifurcations in the decision trees which \n\
 resulted in the minimum increase in likelihood are progressively \n\
 removed and replaced by the parent node. The selection of the \n\
 branches to be pruned out is done across the entire collection of \n\
 decision trees globally.";

  const char examplestr[] =  
"Example: \n\
\n\
prunetree \n\
  -itreedir input_tree_dir \n\
  -nseno 5000 \n\
  -otreedir output_tree_dir \n\
  -moddefn mdef \n\
  -psetfn questions \n\
  -minocc 100 ";

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
	  "CI model definition file" },

	{ "-psetfn",
	  ARG_STRING,
	  NULL,
	  "Phone set definition file" },

	{ "-itreedir",
	  ARG_STRING,
	  NULL,
	  "Input tree directory" },
	{ "-otreedir",
	  ARG_STRING,
	  NULL,
	  "Output tree directory" },

	{ "-nseno",
	  ARG_INT32,
	  NULL,
	  "# of senones defined by the output trees"},

	{ "-minocc",
	  ARG_FLOAT32,
	  "0.0",
	  "Prune nodes w/ fewer than this # of observations"},

	{ "-allphones",
	  ARG_BOOLEAN,
	  "no",
	  "Prune a single tree for each state of all phones"},
	  
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
