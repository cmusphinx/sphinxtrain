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
 * File: cmd_ln.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>

#include <stdlib.h>

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Model definition file of the ci models" },

	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "means file for tree building using continuous HMMs" },

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "variances file for tree building using continuous HMMs" },

        { "-varfloor",
          CMD_LN_FLOAT32,
          CMD_LN_NO_VALIDATION,
          "1.0e-08",
          "The minimum variance"},

	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "PDF's for tree building using semicontinuous HMMs" },

        { "-npermute",
          CMD_LN_INT32,
          CMD_LN_NO_VALIDATION,
          "6",
          "The minimum variance"},

	{ "-tempfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "/tmp/TEMP.QUESTS",
	  "File to write temprorary results to (important)" },

	{ "-questfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "File to write questions to" },

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
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
