/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: parse_cmd_ln.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/common.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

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
 * Revision 1.4  2004/11/29  01:43:51  egouvea
 * Replaced handling of help or example so that user gets an INFO message instead of a scarier FATAL_ERROR
 * 
 * Revision 1.3  2004/08/07 20:25:47  arthchan2003
 * Add help and example string for mllr_solve. Fix help and example logic for mllr_solve and mllr_transfrom
 *
 * Revision 1.2  2004/08/03 07:23:12  arthchan2003
 * Check in the code for usage and example of mllr_transform
 *
 * Revision 1.1  2004/07/26 05:04:20  arthchan2003
 * mllr_transform committed, it is an adaptation of Sam Joo's mllr_adapt
 *
 * Revision 1.1  97/03/07  08:53:38  eht
 * Initial revision
 * 
 * Revision 1.4  1996/01/30  17:06:44  eht
 * Include "-gaucntfn" argument and coalesce "-meanfn" and "-varfn"
 * into "-gaufn"
 *
 * Revision 1.3  1995/09/07  20:03:56  eht
 * Include defn of TRUE/FALSE for machines like HP's running HPUX
 *
 * Revision 1.2  1995/08/09  20:37:06  eht
 * *** empty log message ***
 *
 * Revision 1.1  1995/06/02  20:36:50  eht
 * Initial revision
 *
 *
 */
