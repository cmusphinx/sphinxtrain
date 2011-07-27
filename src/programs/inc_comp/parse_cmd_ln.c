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
 * File: parse_cmd_ln.c
 * 
 * Description: 
 * 	This file contains the code to define and parse the command
 *	line for the inc_comp command.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <stdio.h>
#include <stdlib.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] = 
"Description: \n\
\n\
Increase the number of mixture of a continuous HMM.  Notice that option \n\
-ninc actually means the number of mixture one wants to split from the \n\
 original models. For example, if you have already 8 mixture and you \n\
 want to go to 16, then 8 should be specified.  \n\
\n\
Usually, the number of mixture is a power of two.  You are also \n\
recommended to split the number of mixture from 1 -> 2 -> 4 -> 8 \n\
-> and so on. \n\
";


  const char examplestr[] = 
"Example : \n\
\n\
inc_comp \n\
 -ninc 16 \n\
 -dcountfn mixture_weights \n\
 -inmixwfn mixture_weights \n\
 -outmixwfn out_mixture_weights \n\
 -inmeanfn means \n\
 -outmeanfn out_means \n\
 -invarfn variance \n\
 -outvarfn out_variance";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-ninc",
	  ARG_INT32,
	  "1",
	  "The # of densities to split"},

	{ "-inmixwfn",
	  ARG_STRING,
	  NULL,
	  "The weight file for all N den/mix"},

	{ "-outmixwfn",
	  ARG_STRING,
	  NULL,
	  "The output mixing weight file name w/ N+NINC density weights/mixture"},

	{ "-inmeanfn",
	  ARG_STRING,
	  NULL,
	  "The source mean file w/ N means"},

	{ "-outmeanfn",
	  ARG_STRING,
	  NULL,
	  "The new mean file w/ N+NINC means"},

	{ "-invarfn",
	  ARG_STRING,
	  NULL,
	  "The source variance file w/ N means"},

	{ "-fullvar",
	  ARG_BOOLEAN,
	  "no",
	  "variances file contains full covariances" },

	{ "-outvarfn",
	  ARG_STRING,
	  NULL,
	  "The new variance file w/ N+NINC means"},

	{ "-dcountfn",
	  ARG_STRING,
	  NULL,
	  "The density counts for the N source den/mix"},

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

