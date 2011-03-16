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

#include <sphinxbase/cmd_ln.h>
#include <s3/s3.h>

#include <sys_compat/misc.h>
#include <sys_compat/file.h>

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
"Description: \n\
   Initialization of mixture weight ";

    const char examplestr[] =  
"Example: \n\
\n\
init_mixw -src_moddeffn src_moddeffn -src_ts2cbfn .semi. -src_mixwfn \n\
src_mixwfn -src_meanfn src_meanfn -src_varfn src_varfn -src_tmatfn \n\
src_tmatfn -dest_moddeffn dest_moddeffn -dest_ts2cbfn \n\
.semi. -dest_mixwfn dest_mixwfn -dest_me anfn dest_meanfn -dest_varfn \n\
dest_varfn -dest_tmatfn dest_tmatfn";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},
	{ "-src_moddeffn",
	  ARG_STRING,
	  NULL,
	  "The source model definition file name"},
	{ "-src_ts2cbfn",
	  ARG_STRING,
	  NULL,
	  "The source state definition file name"},
	{ "-src_mixwfn",
	  ARG_STRING,
	  NULL,
	  "The source mixing weight file name"},
	{ "-src_meanfn",
	  ARG_STRING,
	  NULL,
	  "The source mean file name"},
	{ "-src_varfn",
	  ARG_STRING,
	  NULL,
	  "The source variance file name"},
	{ "-fullvar",
	  ARG_BOOLEAN,
	  "no",
	  "Source and destination are full covariance matrices"},
	{ "-src_tmatfn",
	  ARG_STRING,
	  NULL,
	  "The source transition matrix file name"},

	{ "-dest_moddeffn",
	  ARG_STRING,
	  NULL,
	  "The destination model definition file name"},
	{ "-dest_ts2cbfn",
	  ARG_STRING,
	  NULL,
	  "The destination state definition file name"},
	{ "-dest_mixwfn",
	  ARG_STRING,
	  NULL,
	  "The destination mixing weight file name"},
	{ "-dest_meanfn",
	  ARG_STRING,
	  NULL,
	  "The destination mean file name"},
	{ "-dest_varfn",
	  ARG_STRING,
	  NULL,
	  "The destination variance file name"},
	{ "-dest_tmatfn",
	  ARG_STRING,
	  NULL,
	  "The destination transition matrix file name"},

	{NULL, 0, NULL, NULL}
    };

    cmd_ln_parse(defn, argc, argv, TRUE);

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

