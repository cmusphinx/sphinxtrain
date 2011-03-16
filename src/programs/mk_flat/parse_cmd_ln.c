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
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#include <sphinxbase/cmd_ln.h>

#include <s3/s3.h>

#include <sys_compat/misc.h>

#include "parse_cmd_ln.h"

#include <stdio.h>
#include <stdlib.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[]=
"Description:\n\
(Copied from Rita's web page) \n\
In flat-initialization, all mixture weights are set to be equal for \n\
all states, and all state transition probabilities are set to be \n\
equal. Unlike in continuous models, the means and variances of the \n\
codebook Gaussians are not given global values, since they are already \n\
estimated from the data in the vector quantization step. To \n\
flat-initialize the mixture weights, each component of each \n\
mixture-weight distribution of each feature stream is set to be a \n\
number equal to 1/N, where N is the codebook size. The mixture_weights \n\
and transition_matrices are initialized using the executable mk_flat ";

  const char examplestr[]=
"Example:\n\
\n\
mk_flat -moddeffn CFS3.ci.mdef -topo CFS3.topology -mixwfn mixture_weights  \n\
-tmatfn transition_matrices -nstream 1 -ndensity 1 ";
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
	      "A SPHINX-III model definition file name" },
	{ "-mixwfn",
	      ARG_STRING,
	      NULL,
	      "An output SPHINX-III mixing weight file name" },
	{ "-topo",
	      ARG_STRING,
	      NULL,
	      "A template model topology matrix file" },
	{ "-tmatfn",
	      ARG_STRING,
	      NULL,
	      "An output SPHINX-III transition matrix file name" },
	{ "-nstream",
	      ARG_INT32,
	      "4",
	      "Number of independent observation streams" },
	{ "-ndensity",
	      ARG_INT32,
	      "256",
	      "Number of densities per mixture density" },
	{ NULL,
	      0,
	      NULL,
	      NULL }
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
