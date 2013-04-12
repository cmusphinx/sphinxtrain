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

#include "parse_cmd_ln.h"

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/feat.h>
#include <sphinxbase/err.h>

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
agg_seg accumulate feature vectors and use the accumulated values to quantize \n\
the vector space.  This functionality is useful in semi-continuous training \n\
initialization. Feature vectors are skipped according to the option -stride. \n\
\n\
Some options are specific to Sphinx-2.\n\
Please type -example yes to get an example argument list.";

    const char examplestr[]=
"Example: \n\
\n\
agg_seg -segdmpdirs segdmpdir -segdmpfn dumpfile -segtype all -ctlfn \n\
ctl -cepdir cepdir -cepext mfc -ceplen 13 -stride 10 ";


    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-segdmpdirs",
	  ARG_STRING_LIST,
	  NULL,
	  "Segment dump directories" },
	{ "-segdmpfn",
	  ARG_STRING,
	  NULL,
	  "Segment dump file" },
	{ "-segidxfn",
	  ARG_STRING,
	  NULL,
	  "Segment index into the dump file." },
	{ "-segtype",
	  ARG_STRING,
	  "st",
	  "Type of segments to dump.  {all,st,phn}" },

	{ "-cntfn",
	  ARG_STRING,
	  NULL,
	  "Per id count file" },

	{ "-lsnfn",
	  ARG_STRING,
	  NULL,
	  "Lexical transcript file (contains all utts in ctl file)" },
	{ "-sentdir",
	  ARG_STRING,
	  NULL,
	  "Root directory of sent (lexical transcript) files" },
	{ "-sentext",
	  ARG_STRING,
	  NULL,
	  "Extension of sent (lexical transcript) files" },
	{ "-ctlfn",
	  ARG_STRING,
	  NULL,
	  "The control file name (enumerates utts in corpus)" },
	{ "-mllrctlfn",
	  ARG_STRING,
	  NULL,
	  "Lists the MLLR transforms for each utterance" },
	{ "-mllrdir",
	  ARG_STRING,
	  NULL,
	  "Directory for MLLR matrices" },
	{ "-nskip",
	  ARG_INT32,
	  "0",
	  "The number of utterances to skip in the control file"},
	{ "-runlen",
	  ARG_INT32,
	  "-1",
	  "The number of utterances to process after skipping"},
	{ "-moddeffn",
	  ARG_STRING,
	  NULL,
	  "Model definition file containing all the triphones in the corpus.  State/transition matrix definitions are ignored." },
	{ "-ts2cbfn",
	  ARG_STRING,
	  NULL,
	  "Tied state to codebook mapping file (may be '.semi.' or '.cont.')"},
	{ "-cb2mllrfn",
	  ARG_STRING,
	  ".1cls.",
	  "codebook to MLLR class mapping file (may be '.1cls.')"},
	{ "-dictfn",
	  ARG_STRING,
	  NULL,
	  "Dictionary containing pronunciations for the words."},
	{ "-fdictfn",
	  ARG_STRING,
	  NULL,
	  "Dictionary containing pronunciations for the fillers."},

	{ "-segdir",
	  ARG_STRING,
	  NULL,
	  "Root directory of the state segmentation files"},
	{ "-segext",
	  ARG_STRING,
	  "v8_seg",
	  "Extension of the state segmentation files"},

	{ "-cepdir",
	  ARG_STRING,
	  NULL,
	  "Root directory of the cepstrum files"},
	{ "-cepext",
	  ARG_STRING,
	  "mfc",
	  "Extension of the cepstrum files"},
	{ "-cachesz",
	  ARG_INT32,
	  "200",
	  "Feature cache size in Mb"},

	{ "-stride",
	  ARG_INT32,
	  "1",
	  "Take every -stride'th frame when producing dmp" },

	{ "-part",
	  ARG_INT32,
	  NULL,
	  "Identifies the corpus part number (range 1..NPART)" },

	{ "-npart",
	  ARG_INT32,
	  NULL,
	  "Partition the corpus into this many equal sized subsets" },


	cepstral_to_feature_command_line_macro(),
	{NULL, 0, NULL, NULL}
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

    return 0;
}
