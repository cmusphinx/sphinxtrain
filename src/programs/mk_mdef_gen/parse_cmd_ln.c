/* ====================================================================
 * Copyright (c) 2000 Carnegie Mellon University.  All rights 
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
 * Command line arguments for mk_mdef_gen
 * Derived from EHT's parse_cmd_ln routines
 *
 * Author: Rita Singh
 *********************************************************************/

#include <s3/cmd_ln.h>

#include "parse_cmd_ln.h"

#include <stdlib.h>

#include <s3/cmd_ln.h>
#include <s3/s3.h>

#include <sys_compat/misc.h>
#include <sys_compat/file.h>

#include <stdio.h>
#include <assert.h>


void parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

  const char helpstr[] = 
"Description: \n\
\n\
(Copied from Rita's comment and I think it is a pretty good description.) \n\
    Multi-function routine to generate mdef for context-independent \n\
    training, untied training, and all-triphones mdef for state tying.\n\
 Flow: \n\
    if (triphonelist) make CI phone list and CD phone list \n\
	  if alltriphones mdef needed, make mdef \n\
    if (rawphonelist) Make ci phone list,  \n\
        if cimdef needed, make mdef \n\
        Generate alltriphones list from dictionary \n\
        if alltriphones mdef needed, make mdef \n\
    if neither triphonelist or rawphonelist quit \n\
    Count triphones and triphone types in transcript \n\
    Adjust threshold according to min-occ and maxtriphones \n\
    Prune triphone list \n\
    Make untied mdef ";

  const char examplestr[]=
"Example: \n\
Create CI model definition file \n\
mk_mdef_gen -phnlstfn phonefile -ocimdef ci_mdeffile -n_state_pm 3\n\
\n\
Create untied CD model definition file \n\
mk_mdef_gen -phnlstfn rawphonefile -dictfn dict -fdictfn filler_dict \n\
-lsnfn transcription -ountiedmdef untie_mdef -n_state_pm 3 \n\
-maxtriphones 10000 \n\
Create tied CD model definition file \n\
mk_mdef_gen -phnlstfn rawphone -oalltphnmdef untie_mdef -dictfn dict \n\
 -fdictfn filler_dict -n_state_pm 3.";

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

	{ "-phnlstfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "List of phones"},
	{ "-inCImdef",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Input CI model definition file.\n\t\t\tIf given -phnlstfn ignored\n"},
	{ "-triphnlstfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-III triphone file name.\n\t\t\tIf given -phnlstfn, -incimdef ignored\n" },
	{ "-inCDmdef",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Input CD model definition file.\n\t\t\tIf given -triphnfn, -phnlstfn, -incimdef ignored\n"},
	{ "-dictfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Dictionary"},
	{ "-fdictfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Filler dictionary"},
	{ "-lsnfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Transcripts file"},
	{ "-n_state_pm",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "3",
	  "No. of states per HMM"},
	{ "-ocountfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output phone and triphone counts file"},
	{ "-ocimdef",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output CI model definition file"},
	{ "-oalltphnmdef",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output all triphone model definition file"},
	{ "-ountiedmdef",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output untied model definition file"},
	{ "-minocc",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "1",
	  "Min occurances of a triphone must occur for inclusion in mdef file"},
	{ "-maxtriphones",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "100000",
	  "Max. number of triphones desired in mdef file"},
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
}


