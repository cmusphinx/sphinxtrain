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
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
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
    static arg_def_t defn[] = {
	{ "-tmatfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The transition matrix parameter file name"},

	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The mixture weight parameter file name"},

	{ "-mixws",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Start id of mixing weight subinterval"},

	{ "-mixwe",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "End id of mixing weight subinterval"},

	{ "-gaufn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A Gaussian parameter file name (either for means or vars)"},

	{ "-gaucntfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A Gaussian parameter weighted vector file"},

	{ "-regmatcntfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "MLLR regression matrix count file"},

	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The model definition file"},

	{ "-lambdafn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The interpolation weight file"},

	{ "-lambdamin",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0",
	  "Print int. wt. >= this"},

	{ "-lambdamax",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "1",
	  "Print int. wt. <= this"},

	{ "-norm",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "yes",
	  "Print normalized parameters"},

	{ "-sigfig",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "4",
	  "Number of significant digits in 'e' notation" },

	{ NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
    };

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    if (cmd_ln_validate() == FALSE) {
	/* one or more command line arguments were
	   deemed invalid */
	exit(1);
    }

    cmd_ln_print_configuration();

    return 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:32  awb
 * *** empty log message ***
 * 
 * Revision 1.6  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.5  97/03/07  08:51:42  eht
 * - added -sigfig argument
 * - added -regmatcntfn argument
 * - added interpolation weight arguments
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
