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
converegd, on a non-lsf setup.\n\
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
"delint -accumdirs accumdir -moddeffn mdef -mixwfn mixw -cilambda 0.9 -feat  c/1..L-1/,d/1..L-1/,c/0/d/0/dd/0/,dd/1..L-1/ -ceplen 13 -maxiter 4000";

    static arg_def_t defn[] = {
	{ "-help",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The model definition file name"},
	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The mixture weight parameter file name"},
	{ "-accumdirs",
	  CMD_LN_STRING_LIST,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A path where accumulated counts are to be read." },
	{ "-cilambda",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.9",
	  "Weight of CI distributions with respect to uniform distribution"},
	{ "-maxiter",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "100",
	  "max # of iterations if no lambda convergence"},
	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "4s_12c_24d_3p_12dd",
	  "feature stream definition" },
	{ "-ceplen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "13",
	  "Input feature vector length (e.g. MFCC)"},
	{ NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
    };

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    if (cmd_ln_validate() == FALSE) {
	/* one or more command line arguments were
	   deemed invalid */
	exit(1);
    }

    isHelp    = *(uint32 *) cmd_ln_access("-help");
    isExample    = *(uint32 *) cmd_ln_access("-example");


    if(isHelp){
      printf("%s\n\n",helpstr);
    }

    if(isExample){
      printf("%s\n\n",examplestr);
    }

    if(isHelp || isExample){
      E_INFO("User asked for help or example.\n");
      exit(1);
    }
    if(!isHelp && !isExample){
      cmd_ln_print_configuration();
    }


    return 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.6  2004/11/29  01:43:45  egouvea
 * Replaced handling of help or example so that user gets an INFO message instead of a scarier FATAL_ERROR
 * 
 * Revision 1.5  2004/08/08 03:49:56  arthchan2003
 * delint help and example string
 *
 * Revision 1.4  2004/07/21 18:30:34  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.6  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.5  1996/03/25  15:40:25  eht
 * Added ability to set input feature vector length
 *
 * Revision 1.4  1996/01/26  18:07:00  eht
 * Add the "-feat" argument
 *
 * Revision 1.3  1995/09/07  20:01:05  eht
 * include defn of TRUE/FALSE for machines like HP's running HPUX
 *
 * Revision 1.2  1995/08/09  00:38:05  eht
 * Another development version
 *
 * Revision 1.1  1995/06/02  20:56:53  eht
 * Initial revision
 *
 *
 */
