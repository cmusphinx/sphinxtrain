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
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/cmd_ln.h>

#include "parse_cmd_ln.h"

#include <stdlib.h>

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
\n\
(Copied from Eric Thayers' comment) \n\
    Make SPHINX-III model definition files from a variety \n\
    input sources.  One input source is a SPHINX-II senone mapping \n\
    file and triphone file.  Another is a list of phones (in which \n\
    case the transition matrices are tied within base phone class \n\
    and the states are untied)";

    const char examplestr[]=
"Example: \n\
[Under construction]";

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
	  "A SPHINX-III model definition file name" },
	{ "-triphonefn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-II triphone file name" },
	{ "-phonelstfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "List of phones (first 4 columns of phone def'ns)"},
	{ "-mapfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-II senone mapping file name" },

	{ "-n_cdstate",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "total # of CD senones/tied_states (even though mapfn may reference fewer)"},
	{ "-n_cistate",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "# of CI senones/tied_states"},
	{ "-n_tmat",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "# of tied transition matrices"},
	{ "-n_state_pm",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "5",
	  "# of states/model"},
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
 * Revision 1.5  2004/08/10  21:58:56  arthchan2003
 * Incorporate help and example for the four final tools
 * 
 * Revision 1.4  2004/07/21 18:30:36  egouvea
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
 * Revision 1.3  97/07/16  11:35:08  eht
 * Added -phonelstfn argument
 * 
 * Revision 1.2  97/03/07  09:01:21  eht
 * reformatting
 * 
 * Revision 1.1  1995/06/02  20:34:16  eht
 * Initial revision
 *
 *
 */
