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
  This is an example of the input and output format, I copied from Rita's web page, \n\
\n\
This is an hypothetical input of tiestate \n\
# triphone: (null) \n\
# seno map: (null) \n\
# \n\
0.3 \n\
5 n_base\n\
34 n_tri\n\
156 n_state_map\n\
117 n_tied_state\n\
15 n_tied_ci_state\n\
5 n_tied_tmat\n\
#\n\
# Columns definitions\n\
#base lft  rt p attrib tmat      ... state id's ...\n\
  SIL   -   - - filler    0    0    1    2    N\n\
   AE   -   - -    n/a    1    3    4    5    N\n\
   AX   -   - -    n/a    2    6    7    8    N\n\
    B   -   - -    n/a    3    9   10   11    N\n\
    T   -   - -    n/a    4   12   13   14    N\n\
   AE   B   T i    n/a    1   15   16   17    N\n\
   AE   T   B i    n/a    1   18   19   20    N\n\
   AX  AX  AX s    n/a    2   21   22   23    N\n\
   AX  AX   B s    n/a    2   24   25   26    N\n\
   AX  AX SIL s    n/a    2   27   28   29    N\n\
   AX  AX   T s    n/a    2   30   31   32    N\n\
   AX   B  AX s    n/a    2   33   34   35    N\n\
   AX   B   B s    n/a    2   36   37   38    N\n\
   AX   B SIL s    n/a    2   39   40   41    N\n\
   AX   B   T s    n/a    2   42   43   44    N\n\
   AX SIL  AX s    n/a    2   45   46   47    N\n\
   AX SIL   B s    n/a    2   48   49   50    N\n\
   AX SIL SIL s    n/a    2   51   52   53    N\n\
   AX SIL   T s    n/a    2   54   55   56    N\n\
   AX   T  AX s    n/a    2   57   58   59    N\n\
   AX   T   B s    n/a    2   60   61   62    N\n\
   AX   T SIL s    n/a    2   63   64   65    N\n\
   AX   T   T s    n/a    2   66   67   68    N\n\
    B  AE  AX e    n/a    3   69   70   71    N\n\
    B  AE   B e    n/a    3   72   73   74    N\n\
    B  AE SIL e    n/a    3   75   76   77    N\n\
    B  AE   T e    n/a    3   78   79   80    N\n\
    B  AX  AE b    n/a    3   81   82   83    N\n\
    B   B  AE b    n/a    3   84   85   86    N\n\
    B SIL  AE b    n/a    3   87   88   89    N\n\
    B   T  AE b    n/a    3   90   91   92    N\n\
    T  AE  AX e    n/a    4   93   94   95    N\n\
    T  AE   B e    n/a    4   96   97   98    N\n\
    T  AE SIL e    n/a    4   99  100  101    N\n\
    T  AE   T e    n/a    4  102  103  104    N\n\
    T  AX  AE b    n/a    4  105  106  107    N\n\
    T   B  AE b    n/a    4  108  109  110    N\n\
    T SIL  AE b    n/a    4  111  112  113    N\n\
    T   T  AE b    n/a    4  114  115  116    N\n\
\n\
is used as the base to give the following CD-tied model definition file with 39 tied states (senones): \n\
\n\
# triphone: (null)\n\
# seno map: (null)\n\
#\n\
0.3\n\
5 n_base\n\
34 n_tri\n\
156 n_state_map\n\
54 n_tied_state\n\
15 n_tied_ci_state\n\
5 n_tied_tmat\n\
#\n\
# Columns definitions\n\
#base lft  rt p attrib tmat      ... state id's ...\n\
  SIL   -   - - filler    0    0    1    2    N\n\
   AE   -   - -    n/a    1    3    4    5    N\n\
   AX   -   - -    n/a    2    6    7    8    N\n\
    B   -   - -    n/a    3    9   10   11    N\n\
    T   -   - -    n/a    4   12   13   14    N\n\
   AE   B   T i    n/a    1   15   16   17    N\n\
   AE   T   B i    n/a    1   18   16   19    N\n\
   AX  AX  AX s    n/a    2   20   21   22    N\n\
   AX  AX   B s    n/a    2   23   21   22    N\n\
   AX  AX SIL s    n/a    2   24   21   22    N\n\
   AX  AX   T s    n/a    2   25   21   22    N\n\
   AX   B  AX s    n/a    2   26   21   27    N\n\
   AX   B   B s    n/a    2   23   21   27    N\n\
   AX   B SIL s    n/a    2   24   21   27    N\n\
   AX   B   T s    n/a    2   25   21   27    N\n\
   AX SIL  AX s    n/a    2   26   21   28    N\n\
   AX SIL   B s    n/a    2   23   21   28    N\n\
   AX SIL SIL s    n/a    2   24   21   28    N\n\
   AX SIL   T s    n/a    2   25   21   28    N\n\
   AX   T  AX s    n/a    2   26   21   29    N\n\
   AX   T   B s    n/a    2   23   21   29    N\n\
   AX   T SIL s    n/a    2   24   21   29    N\n\
   AX   T   T s    n/a    2   25   21   29    N\n\
    B  AE  AX e    n/a    3   30   31   32    N\n\
    B  AE   B e    n/a    3   33   31   32    N\n\
    B  AE SIL e    n/a    3   34   31   32    N\n\
    B  AE   T e    n/a    3   35   31   32    N\n\
    B  AX  AE b    n/a    3   36   37   38    N\n\
    B   B  AE b    n/a    3   36   37   39    N\n\
    B SIL  AE b    n/a    3   36   37   40    N\n\
    B   T  AE b    n/a    3   36   37   41    N\n\
    T  AE  AX e    n/a    4   42   43   44    N\n\
    T  AE   B e    n/a    4   45   43   44    N\n\
    T  AE SIL e    n/a    4   46   43   44    N\n\
    T  AE   T e    n/a    4   47   43   44    N\n\
    T  AX  AE b    n/a    4   48   49   50    N\n\
    T   B  AE b    n/a    4   48   49   51    N\n\
    T SIL  AE b    n/a    4   48   49   52    N\n\
    T   T  AE b    n/a    4   48   49   53    N";

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
 * Revision 1.4  2004/11/29  01:11:37  egouvea
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
