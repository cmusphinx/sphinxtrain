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
      "Description: \nThis program computes a mixture gaussian distribution for a set of \nspeech cepstral files for use with CDCN. The first mode is computed on only\nthe silence portions of the speech to comply with the requirements of the \nalgorithm.";

    const char examplestr[]=
"Example: \n\
\n\
cdcn_stats -cepext mfc \n\
        -ctlfn stuff.ctl \n\
        -outfn output.cdcn.dist \n\
        -nmodes 32 \n\
        -noisewidth 1.5 \n\
        -tmpfn CDCN.tmp \n\
        -ceplen 13 \n\
        -stride 10 ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-ctlfn",
	  ARG_STRING,
	  NULL,
	  "The control file name (enumerates utts in corpus)" },
	{ "-outfn",
	  ARG_STRING,
	  NULL,
	  "The output distribution file name" },
	{ "-cepdir",
	  ARG_STRING,
	  NULL,
	  "Root directory of the cepstrum files"},
	{ "-cepext",
	  ARG_STRING,
	  "mfc",
	  "Extension of the cepstrum files"},

	{ "-maxframes",
	  ARG_INT32,
	  "-1",
	  "Maximum number of frames to read (or -1 for unlimited)"},

	{ "-cbfn",
	  ARG_STRING,
	  NULL,
	  "A previous codebook to restart EM training from" },

	{ "-nmodes",
	  ARG_INT32,
	  "32",
	  "# of Gaussians to train"},
	{ "-noisewidth",
	  ARG_FLOAT32,
	  "1.5",
	  "Width of noise band"},
	{ "-ceplen",
	  ARG_INT32,
	  "13",
	  "# of coefficients per cepstrum frame"},
	{ "-stride",
	  ARG_INT32,
	  "1",
	  "Take every -stride'th frame when computing distribution" },

	{ "-vqiter",
	  ARG_INT32,
	  "5",
	  "Max # of interations of VQ to get initial codebook"},
	{ "-vqthresh",
	  ARG_FLOAT32,
	  "1e-6",
	  "Convergence ratio for VQ"},
	{ "-emiter",
	  ARG_INT32,
	  "10",
	  "# of interations of EM to estimate distributions"},
	{ "-emthresh",
	  ARG_FLOAT32,
	  "1e-6",
	  "Convergence ratio for EM"},

	{ "-tmpfn",
	  ARG_STRING,
	  "CDCN.DIST.TEMP",
	  "The temporary file name" },

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
