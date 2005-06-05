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
 * 	This file contains the code to define and parse the command
 *	line for the inc_comp command.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <stdio.h>
#include <stdlib.h>
#include <s3/cmd_ln.h>
#include <s3/err.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] = 
"Description: \n\
\n\
Increase the number of mixture of a continuous HMM.  Notice that option \n\
-ninc actually means the number of mixture one wants to split from the \n\
 original models. For example, if you have already 8 mixture and you \n\
 want to go to 16, then 8 should be specified.  \n\
\n\
Usually, the number of mixture is a power of two.  You are also \n\
recommended to split the number of mixture from 1 -> 2 -> 4 -> 8 \n\
-> and so on. \n\
";


  const char examplestr[] = 
"Example : \n\
\n\
inc_comp \n\
 -ninc 16 \n\
 -dcountfn mixture_weights \n\
 -inmixwfn mixture_weights \n\
 -outmixwfn out_mixture_weights \n\
 -inmeanfn means \n\
 -outmeanfn out_means \n\
 -invarfn variance \n\
 -outvarfn out_variance \n\
 -ceplen 13";

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

	{ "-ninc",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "1",
	  "The # of densities to split"},

	{ "-inmixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The weight file for all N den/mix"},

	{ "-outmixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The output mixing weight file name w/ N+NINC density weights/mixture"},

	{ "-inmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The source mean file w/ N means"},

	{ "-outmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The new mean file w/ N+NINC means"},

	{ "-invarfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The source variance file w/ N means"},

	{ "-outvarfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The new variance file w/ N+NINC means"},

	{ "-dcountfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The density counts for the N source den/mix"},

        { "-ceplen",
          CMD_LN_INT32,
          CMD_LN_NO_VALIDATION,
          "13",
          "The length of the input feature (e.g. MFCC) vectors"},
 

	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Defines the acoustic feature set."},

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
 * Revision 1.9  2005/06/05  22:12:03  arthchan2003
 * Bug no. 1160673, fix the help string of inc_comp.
 * 
 * Revision 1.8  2004/11/29 01:43:45  egouvea
 * Replaced handling of help or example so that user gets an INFO message instead of a scarier FATAL_ERROR
 *
 * Revision 1.7  2004/11/29 00:49:20  egouvea
 * Added missing include files to prevent warnings about printf not being defined
 *
 * Revision 1.6  2004/08/26 05:45:33  arthchan2003
 * update of help and example strings of inc_comp
 *
 * Revision 1.5  2004/08/08 04:07:24  arthchan2003
 * help and example strings of inc_comp
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
 * Revision 1.4  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1996/07/29  16:26:54  eht
 * development release
 *
 * Revision 1.2  1996/01/26  18:22:55  eht
 * Development version
 *
 * Revision 1.1  1995/12/15  18:37:07  eht
 * Initial revision
 *
 *
 */
