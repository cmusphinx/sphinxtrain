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
 * 
 * Author: 
 * 
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>

#include <stdio.h>
#include <stdlib.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

    const char helpstr[] =  
"Description: \n\
\n\
    This is an implementation of Dr. Rita Singh's automatic question \n\
    generation.  (Copied from Rita's comment) The current algorithm \n\
    clusters CI distributions using a hybrid bottom-up top-down \n\
    clustering algorithm to build linguistic questions for decision \n\
    trees.\n\
    (From Arthur : I need to do some tracing before understand it \n\
    what's the internal of the code)";

    const char examplestr[] =
"Example: \n\
make_quest -moddeffn mdef -meanfn mean -varfn var -mixwfn \n\
mixwfn -npermute 8 -niter 1 -qstperstt 20 -tempfn temp -questfn \n\
questions";


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
	  "Model definition file of the ci models" },

	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "means file for tree building using continuous HMMs" },

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "variances file for tree building using continuous HMMs" },

        { "-varfloor",
          CMD_LN_FLOAT32,
          CMD_LN_NO_VALIDATION,
          "1.0e-08",
          "The minimum variance"},

	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "PDF's for tree building using semicontinuous HMMs" },

        { "-npermute",
          CMD_LN_INT32,
          CMD_LN_NO_VALIDATION,
          "6",
          "The minimum variance"},

        { "-niter",
          CMD_LN_INT32,
          CMD_LN_NO_VALIDATION,
          "0",
          "Number of iterations"},

        { "-qstperstt",
          CMD_LN_INT32,
          CMD_LN_NO_VALIDATION,
          "8",
          "something per state"},

	{ "-tempfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "/tmp/TEMP.QUESTS",
	  "File to write temprorary results to (important)" },

	{ "-questfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "File to write questions to" },

	{ "-type",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "HMM type" },

	{ NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
    };

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    if (cmd_ln_validate() == FALSE) {
	E_FATAL("Unable to validate command line arguments\n");
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
      E_FATAL("User ask for help or example, stop before proceed\n");
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
 * Revision 1.5  2004/11/29  01:11:34  egouvea
 * Fixed license terms in some new files.
 * 
 * Revision 1.4  2004/11/29 00:49:22  egouvea
 * Added missing include files to prevent warnings about printf not being defined
 *
 * Revision 1.3  2004/08/10 22:32:43  arthchan2003
 * Fix the dollar problem of make_quests
 *
 * Revision 1.2  2004/08/10 21:58:51  arthchan2003
 * Incorporate help and example for the four final tools
 *
 * Revision 1.1  2004/06/17 19:39:49  arthchan2003
 * add back all command line information into the code
 *
 * Revision 1.4  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.3  2000/11/17 16:10:41  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
