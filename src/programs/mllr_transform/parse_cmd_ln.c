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
#include "cmd_ln_defn.h"

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    cmd_ln_print_configuration();

    return 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2004/07/26  05:04:20  arthchan2003
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
