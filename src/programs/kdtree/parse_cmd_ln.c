/* ====================================================================
 * Copyright (c) 2005 Carnegie Mellon University.  All rights 
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
 * 	Parse the command line for kdtree(1)
 *
 * Author: 
 * 	David Huggins-Daines <dhuggins@cs.cmu.edu>
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/common.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>


/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[]=
"Description: \n"
"Create a kd-tree from a set of Gaussians";

  const char examplestr[]=
"Example: \n\
kdtree \n\
 -meanfn mean \n\
 -varfn variances\n\
 -outfn kdtrees\n\
 -threshold 0.2\n\
 -depth 6\n\
 -absolute FALSE\n\
";


    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-outfn",
	  ARG_STRING,
	  NULL,
	  "Output file for trees"},
	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "Gaussian density mean file to build a tree over"},
	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "Gaussian density variance file to build a tree over"},
	{ "-threshold",
	  ARG_FLOAT32,
	  "0.2",
	  "Threshold for Gaussian boxes"},
	{ "-depth",
	  ARG_INT32,
	  "6",
	  "Depth of kd-tree to be built"},
	{ "-absolute",
	  ARG_BOOLEAN,
	  "FALSE",
	  "Use an absolute threshold for Gaussian boxes"},

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
