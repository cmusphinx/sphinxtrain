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
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/common.h>
#include <s3/s3.h>	/* pick up defn of TRUE/FALSE */

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

    const char helpstr[] =  
"Description: (copied from Rita's web page.) \n\
\n\
Deleted interpolation is the final step in creating semi-continuous \n\
models. The output of deleted interpolation are semi-continuous models \n\
in sphinx-3 format. These have to be further converted to sphinx-2 \n\
format, if you want to use the SPHINX-II decoder. \n\
\n\
Deleted interpolation is an iterative process to interpolate between \n\
CD and CI mixture-weights to reduce the effects of overfitting. The \n\
data are divided into two sets, and the data from one set are used to \n\
estimate the optimal interpolation factor between CI and CD models \n\
trained from the other set. Then the two data sets are switched and \n\
this procedure is repeated using the last estimated interpolation \n\
factor as an initialization for the current step. The switching is \n\
continued until the interpolation factor converges.\n\
\n\
To do this, we need *two* balanced data sets. Instead of the actual\n\
data, however, we use the Bauim-Welch buffers, since the related math\n\
is convenient. we therefore need an *even* number of buffers that can\n\
be grouped into two sets. DI cannot be performed if you train using\n\
only one buffer. At least in the final iteration of the training, you\n\
must perform the training in (at least) two parts. You could also do\n\
this serially as one final iteration of training AFTER BW has\n\
converged, on a non-lsf setup.\n\
\n\
Note here that the norm executable used at the end of every Baum-Welch \n\
iteration also computes models from the buffers, but it does not\n\
require an even number of buffers. BW returns numerator terms and\n\
denominator terms for the final estimation, and norm performs the\n\
actual division. The number of buffers is not important, but you would \n\
need to run norm at the end of EVERY iteration of BW, even if you did\n\
the training in only one part. When you have multiple parts norm sums\n\
up the numerator terms from the various buffers, and the denominator\n\
terms, and then does the division. ";

    const char examplestr[]=
"delint -accumdirs accumdir -moddeffn mdef -mixwfn mixw -cilambda 0.9 -maxiter 4000";

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
	  "The model definition file name"},
	{ "-mixwfn",
	  ARG_STRING,
	  NULL,
	  "The mixture weight parameter file name"},
	{ "-accumdirs",
	  ARG_STRING_LIST,
	  NULL,
	  "A path where accumulated counts are to be read." },
	{ "-cilambda",
	  ARG_FLOAT32,
	  "0.9",
	  "Weight of CI distributions with respect to uniform distribution"},
	{ "-maxiter",
	  ARG_INT32,
	  "100",
	  "max # of iterations if no lambda convergence"},

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

