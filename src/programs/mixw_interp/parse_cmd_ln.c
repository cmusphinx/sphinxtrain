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
 * File: parse_cmd_ln.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <sys_compat/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>


/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
  uint32      isHelp;
  uint32      isExample;

#include "cmd_ln_defn.h"

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
 * Revision 1.3  2004/11/29  01:11:34  egouvea
 * Fixed license terms in some new files.
 * 
 * Revision 1.2  2004/08/10 21:58:56  arthchan2003
 * Incorporate help and example for the four final tools
 *
 * Revision 1.1  2004/06/17 19:39:49  arthchan2003
 * add back all command line information into the code
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
 * Revision 1.14  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.13  1996/08/06  14:03:47  eht
 * -sildelfn argument to specify silence deletion list
 *
 * Revision 1.12  1996/07/29  16:18:48  eht
 * Make -accumdir optional so that it may be omitted for
 * debugging purposes
 * MLLR command line options
 * -veclen to -ceplen
 * -minvar to -varfloor (now named consistently w/ the other floors)
 * added -2passvar switch to allow reestimation based on prior means
 *
 * Revision 1.11  1996/03/26  14:03:24  eht
 * - Added '-timing' argument
 * - changed doc strings for some arguments
 *
 * Revision 1.10  1996/02/02  17:41:47  eht
 * Add alpha and beta beams
 *
 * Revision 1.9  1996/01/26  18:23:49  eht
 * Reformatted argument specifications
 *
 * Revision 1.8  1995/11/30  20:42:07  eht
 * Add argument for transition matrix reestimation
 * Add argument for state parameter definition file
 *
 *
 */

