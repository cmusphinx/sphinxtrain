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

#include <s3/cmd_ln.h>

#include "cmd_ln.h"

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-tmatfn",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "A SPHINX-III transition matrix parameter file name" },
	{ "-moddeffn",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "A SPHINX-III model definition file name" },
	{ "-floor",
	      CMD_LN_FLOAT32,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "Floor weight value to apply before renormalization" },
	{ "-hmmdir",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "A directory containing SPHINX-III models consistent with -moddeffn" },
	{ "-hmmext",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      "chmm",
	      "Extension of a SPHINX-II model file." },
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
 * Revision 1.1  97/01/21  13:04:17  eht
 * Initial revision
 * 
 * Revision 1.1  95/06/02  20:34:58  eht
 * Initial revision
 * 
 *
 */
