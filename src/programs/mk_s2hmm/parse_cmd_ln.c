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
 *********************************************************************
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

#include <s3/cmd_ln.h>

#include "parse_cmd_ln.h"

#include <stdlib.h>

#include <s3/common.h>
#include <s3/s3.h>

#include <sys/stat.h>
#include <sys/types.h>

int32 parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[]=
"Description: \n\
\n\
Convert s3 model definition file, mixture weight and transition \n\
matrices to s2 hmm format.  ";

  const char examplestr[]=
"Example: \n\
\n\
mk_s2hmm -meanfn s3mdef -varfn s3mixw -tmat s3tmat -hmmdir s2dir  ";

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
	  "Model definition file for the S3 models" },
	{ "-tmatfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "S3 transition matrix parameter file name"},
	{ "-tpfloor",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.0001",
	  "Transition probability smoothing floor" },	
	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "S3 mixing weight parameter file name"},
	{ "-hmmdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "S2 model output directory"},
	{ "-hmmext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "chmm",
	  "Extension of a SPHINX-II model file." },
        { "-cepsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "ccode",
              "Extension of cepstrum senone weight file" },
        { "-dcepsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "d2code",
              "Extension of difference cepstrum senone weight file" },
        { "-powsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "p3code",
              "Extension of power senone weight file" },
        { "-2dcepsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "xcode",
             "Extension of 2nd order difference cepstrum senone weight file" },
	{ "-mtype",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "sdm",
	  "Model type {sdm,dhmm}"},
	{ NULL,
	  CMD_LN_UNDEF,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  NULL }
    };

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

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
 * Revision 1.5  2004/11/29  01:43:48  egouvea
 * Replaced handling of help or example so that user gets an INFO message instead of a scarier FATAL_ERROR
 * 
 * Revision 1.4  2004/08/10 08:08:27  arthchan2003
 * Sphinx 3 to Sphinx 2 conversion tools' helps and examples
 *
 * Revision 1.3  2004/07/21 18:30:37  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.2  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/11/22 21:23:18  awb
 * *** empty log message ***
 *
 * Revision 1.4  97/07/23  14:34:12  eht
 * Added missing command line arguments for seno file extensions.
 * 
 * Revision 1.3  97/07/16  11:29:26  eht
 * Update so that discrete s2hmm's could be written
 * 
 * Revision 1.2  95/11/30  21:04:34  eht
 * Get rid of superfluous arguments
 * 
 * Revision 1.1  1995/09/08  19:11:14  eht
 * Initial revision
 *
 *
 */
