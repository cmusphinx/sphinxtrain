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
 * 	Command line parser for mk_s3gau
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <stdlib.h>
#include "sphinxbase/cmd_ln.h"

#include "parse_cmd_ln.h"

#include <s3/common.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] =
"Description: \n\
Conversion from sphinx 2 codebook to sphinx3 means and variances";

  const char examplestr[]=
"Example: \n\
mk_s3gau -meanfn s3mean -varfn s3var -cbdir s2dir ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "A SPHINX-III mean density parameter file name" },
	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "A SPHINX-III variance density parameter file name" },
	{ "-cbdir",
	  ARG_STRING,
	  ".",
	  "A directory containing SPHINX-II 1PD codebooks" },
	{ "-varfloor",
	  ARG_FLOAT32,
	  "0.00001",
	  "Minimum variance value" },
	{ "-cepcb",
	  ARG_STRING,
	  "cep.256",
	  "Basename of the cepstrum codebook" },
	{ "-dcepcb",
	  ARG_STRING,
	  "d2cep.256",
	  "Basename of the difference cepstrum codebook" },
	{ "-powcb",
	  ARG_STRING,
	  "p3cep.256",
	  "Basename of the power codebook" },
	{ "-2dcepcb",
	  ARG_STRING,
	  "xcep.256",
	  "Basename of the 2nd order difference cepstrum codebook" },

	{ "-meanext",
	  ARG_STRING,
	  "vec",
	  "Mean codebook extension" },

	{ "-varext",
	  ARG_STRING,
	  "var",
	  "Variance codebook extension" },

	{ "-fixpowvar",
	  ARG_BOOLEAN,
	  "false",
	  "Fix the power variance to the SPHINX-II standards" },

	{ NULL,
	  0,
	  NULL,
	  NULL }
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
