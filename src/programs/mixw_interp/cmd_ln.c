/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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
 * Revision 1.1  2000/09/24  21:38:31  awb
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

