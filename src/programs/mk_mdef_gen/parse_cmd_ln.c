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

#include <sphinxbase/cmd_ln.h>

#include "parse_cmd_ln.h"

#include <stdlib.h>

#include <sphinxbase/cmd_ln.h>
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
    Make untied mdef \n\
\n\
Usually, this program is used in \n\
1) generate CI mdef \n\
2) generate CD mdef given dictionary and transcription \n\
3) generate CD mdef given dictionary \n\
Also see examples";

  const char examplestr[]=
"Example: \n\
Create CI model definition file \n\
mk_mdef_gen -phnlstfn phonefile -ocimdef ci_mdeffile -n_state_pm 3\n\
\n\
Create untied CD model definition file given dictionary and transcription\n\
mk_mdef_gen -phnlstfn rawphonefile -dictfn dict -fdictfn filler_dict \n\
-lsnfn transcription -ountiedmdef untie_mdef -n_state_pm 3 \n\
-maxtriphones 10000 \n\
Create all CD model definition file given only the dictionary \n\
mk_mdef_gen -phnlstfn rawphone -oalltphnmdef untie_mdef -dictfn dict \n\
 -fdictfn filler_dict -n_state_pm 3.";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-ignorewpos",
	  ARG_BOOLEAN,
	  "no",
	  "Ignore word position in context dependency"},
	{ "-phnlstfn",
	  ARG_STRING,
	  NULL,
	  "List of phones"},
	{ "-inCImdef",
	  ARG_STRING,
	  NULL,
	  "Input CI model definition file.\n\t\t\tIf given -phnlstfn ignored\n"},
	{ "-triphnlstfn",
	  ARG_STRING,
	  NULL,
	  "A SPHINX-III triphone file name.\n\t\t\tIf given -phnlstfn, -incimdef ignored\n" },
	{ "-inCDmdef",
	  ARG_STRING,
	  NULL,
	  "Input CD model definition file.\n\t\t\tIf given -triphnfn, -phnlstfn, -incimdef ignored\n"},
	{ "-dictfn",
	  ARG_STRING,
	  NULL,
	  "Dictionary"},
	{ "-fdictfn",
	  ARG_STRING,
	  NULL,
	  "Filler dictionary"},
	{ "-lsnfn",
	  ARG_STRING,
	  NULL,
	  "Transcripts file"},
	{ "-n_state_pm",
	  ARG_INT32,
	  "3",
	  "No. of states per HMM"},
	{ "-ocountfn",
	  ARG_STRING,
	  NULL,
	  "Output phone and triphone counts file"},
	{ "-ocimdef",
	  ARG_STRING,
	  NULL,
	  "Output CI model definition file"},
	{ "-oalltphnmdef",
	  ARG_STRING,
	  NULL,
	  "Output all triphone model definition file"},
	{ "-ountiedmdef",
	  ARG_STRING,
	  NULL,
	  "Output untied model definition file"},
	{ "-minocc",
	  ARG_INT32,
	  "1",
	  "Min occurances of a triphone must occur for inclusion in mdef file"},
	{ "-maxtriphones",
	  ARG_INT32,
	  "100000",
	  "Max. number of triphones desired in mdef file"},
	{ NULL,
	  0,
	  NULL,
	  NULL }
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
}
