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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2005/06/05  22:03:38  arthchan2003
 * Fix warning of parse_cmd_ln.c.
 * 
 * Revision 1.1  2005/06/05 22:00:33  arthchan2003
 * Log. Rewriting QUICK_COUNT using SphinxTrain command line functions. Several changes.
 * 1, Removal of -B -t because they were actually not implemented.
 * 2, Add SphinxTrain's style command line, help string and example string.
 * 3, Safe-guarding a, invalid file names, b, user didn't specify SIL in the phone list.
 * 4, Change all quit to E_INFO, also delete obsolete quit.c.  Will change the windows setup respectively.
 * 5, Fix bug 1151880.  That was caused by the use of magic phrase symbol &, the loop will read stuff out of memoery boundary  when & occurs at the end of the word.  This was fixed by specifically checking this particular condition in quick_phone.c.
 *
 * Windows setup is not yet checked in. Will do right after the linux check-in.
 *
 * Major revamped by Arthur Chan at 2005 Jun 5
 *
 */

#include "parse_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>

#include <sys_compat/misc.h>
#include <stdlib.h>
#include <stdio.h>

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

    const char helpstr[] =  
"Description: \n\
\n\
Generate a set of context-dependent phones list (usually triphones)  given a dictionary. \n\
\n\
A phone list is first created in the following format: \n\
\n\
phone1 0 0 0 0 \n\
phone2 0 0 0 0 \n\
phone3 0 0 0 0 \n\
\n\
The phone list of CI model training must be used to generate this. \n\
\n\
Next a temporary dictionary is generated, which has all words except the filler word (word enclosed in ++()++).  \n\
The entry. \n\
SIL SIL \n\
must be added to the temporary dictionary and the dictionary must be sort in alphabetic order. \n\
\n\
-q:  a mandatory flag that tell QUICK_COUNT to consider all word pairs while constructing the phone list \n\
-p: the formatted phone list \n\
-b: a temporary dictionary file \n\
-o: output triphone list \n\
";
    const char examplestr[]=
"Example : \n\
\n\
QUICK_COUNT -q yes -p phonelist -b dictionary -o outputlist \n\
";

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

	{ "-v",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Verbose" },

	{ "-q",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Flag to consider all word pairs" },

	{ "-b",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Base file or the dictionary file" },


	{ "-p",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Phone list file" },

	{ "-P",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Another argument for phone list file. Equivalent to -p. (keep it for backward compatibility purpose.)" },
	  
	{ "-o",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output triphone list"},
	  
	{ "-i",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Input control file"},
	  
	{ "-I",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Another argument for input control file. Equivalent to -i. (Keep it for backward compatibility purpose)"},

	{ "-f",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Find examples (?) "},

	{ "-S",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Use single path to make triphones "},

	{ "-s",
	  CMD_LN_STRING_LIST,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Directory of sentences "},

#if 0  /* Actually not being used */
	{ "-t",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "1",
	  "Minimum Frequency" },
	{ "-B",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A bigram that is used " },
#endif

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


    cmd_ln_print_configuration();

    return 0;
}

