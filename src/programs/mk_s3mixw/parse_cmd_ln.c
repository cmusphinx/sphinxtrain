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

#include "parse_cmd_ln.h"

#include <stdlib.h>

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-mixwfn",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "A SPHINX-III mixture weight parameter file name" },
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
	{ "-ci2cd",
	      CMD_LN_BOOLEAN,
	      CMD_LN_NO_VALIDATION,
	      "false",
	      "CD weights initialized to CI weights (-hmmdir is to a set of CI models)" },
	{ "-hmmdir",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "A directory containing SPHINX-III models consistent with -moddeffn" },
	{ "-cepsenoext",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      "ccode",
	      "Extension of cepstrum senone weight file" },
	{ "-dcepsenoext",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      "d2code",
	      "Extension of difference cepstrum senone weight file" },
	{ "-powsenoext",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      "p3code",
	      "Extension of power senone weight file" },
	{ "-2dcepsenoext",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      "xcode",
	      "Extension of 2nd order difference cepstrum senone weight file" },
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
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  95/06/02  20:33:30  eht
 * Initial revision
 * 
 *
 */
