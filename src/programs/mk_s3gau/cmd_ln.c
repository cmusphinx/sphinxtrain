/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: cmd_ln.c
 * 
 * Description: 
 * 	Command line parser for mk_s3gau
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/cmd_ln.h>

#include "cmd_ln.h"

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-III mean density parameter file name" },
	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-III variance density parameter file name" },
	{ "-cbdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A directory containing SPHINX-II 1PD codebooks" },
	{ "-varfloor",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Minimum variance value" },
	{ "-cepcb",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "cep.256",
	  "Basename of the cepstrum codebook" },
	{ "-dcepcb",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "d2cep.256",
	  "Basename of the difference cepstrum codebook" },
	{ "-powcb",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "p3cep.256",
	  "Basename of the power codebook" },
	{ "-2dcepcb",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "xcep.256",
	  "Basename of the 2nd order difference cepstrum codebook" },

	{ "-meanext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "vec",
	  "Mean codebook extension" },

	{ "-varext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "var",
	  "Variance codebook extension" },

	{ "-fixpowvar",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "false",
	  "Fix the power variance to the SPHINX-II standards" },

	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "4s_12c_24d_3p_12dd",
	  "Defines the feature set to use" },

	{ "-ceplen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "13",
	  "Defines the input feature vector (e.g. MFCC) len" },

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
 * Revision 1.1  97/03/07  08:56:51  eht
 * Initial revision
 * 
 *
 */
