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
 * File: parse_cmd_ln.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#include <stdlib.h>
#include "s3/cmd_ln.h"

#include "parse_cmd_ln.h"

#include <s3/common.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

int32 parse_cmd_ln(int argc, char *argv[])
{

  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] =
"Description: \n\
Conversion from sphinx 3 files to sphinx 2 senones.  \n\
Note: by Arthur: not sure whether it is obsolete";

  const char examplestr[]=
"Example: \n\
(by Arthur: I believe this tool is obsolete, we may want to delete it later on. ) \n\";

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
	      "The model definition file for the model inventory to train" },
	{ "-mixwfn",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "The mixture weight parameter file name"},

	{ "-tpfloor",
	      CMD_LN_FLOAT32,
	      CMD_LN_NO_VALIDATION,
	      "0.0001",
	      "Transition probability smoothing floor" },

	{ "-hmmdir",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              CMD_LN_NO_DEFAULT,
              "A directory containing SPHINX-II models consistent with -moddeffn" },
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
 * Revision 1.5  2004/11/29  01:43:49  egouvea
 * Replaced handling of help or example so that user gets an INFO message instead of a scarier FATAL_ERROR
 * 
 * Revision 1.4  2004/08/10 08:31:54  arthchan2003
 * s2 to s3 conversion tools
 *
 * Revision 1.3  2004/07/21 19:17:24  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.2  2001/02/20 00:28:35  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/11/22 21:23:18  awb
 * *** empty log message ***
 *
 * Revision 1.1  95/09/08  19:11:14  eht
 * Initial revision
 * 
 *
 */
