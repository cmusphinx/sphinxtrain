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

  const char helpstr[]=
"Description : \n\
  Create a model definition file with tied state from model definition file without tied states. ";

  const char examplestr[]=
"Example: \n\
  tiestate -imoddeffn imdef -omoddeffn omdef -treedir trees -psetfn questions \n\
\n\
  This is an example of the input and output format, Find more details at, \n\
  http://www.speech.cs.cmu.edu/sphinxman";

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

	{ "-imoddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Untied-state model definition file"},

	{ "-omoddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Tied-state model definition file"},

	{ "-treedir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "SPHINX-III tree directory containing pruned trees"},

	{ "-psetfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Phone set definiton file" },

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
 * Revision 1.6  2005/04/07  21:23:40  egouvea
 * Improved the documentation, making it easier to find pointers, fixed the setup script, and fixed some of the doxygen comments
 * 
 * Revision 1.5  2004/11/29 01:43:52  egouvea
 * Replaced handling of help or example so that user gets an INFO message instead of a scarier FATAL_ERROR
 *
 * Revision 1.4  2004/11/29 01:11:37  egouvea
 * Fixed license terms in some new files.
 *
 * Revision 1.3  2004/11/29 00:49:28  egouvea
 * Added missing include files to prevent warnings about printf not being defined
 *
 * Revision 1.2  2004/08/09 20:59:48  arthchan2003
 * help and example of tiestate
 *
 * Revision 1.1  2004/06/17 19:39:51  arthchan2003
 * add back all command line information into the code
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:32  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
