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
 * 	Parse the command line for norm(1)
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

static int
validate_read_file(char *switch_name, void *arg)
{
    char *fn = arg;
    FILE *fp;

    fp = fopen(fn, "r");
    if (fp != NULL) {
	fclose(fp);
	return TRUE;
    }
    else
	return FALSE;
}

static int
validate_optional_read_file(char *switch_name, void *arg)
{
    if (arg) return validate_read_file(switch_name, arg);
    else return TRUE;
}

/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-accumdir",
	  CMD_LN_STRING_LIST,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Paths containing reestimation sums from bw" },
	{ "-oaccumdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Path to contain the overall reestimation sums" },
	{ "-tmatfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Transition prob. matrix file to produce (if any)"},
	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Mixing weight file to produce (if any)"},
	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Gaussian density mean file to produce (if any)"},
	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Gaussian density variance file to produce (if any)"},
	{ "-regmatfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "MLLR regression matrices file to produce (if any)"},
	{ "-dcountfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Gaussian density count file to produce"},
	{ "-inmixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Use mixing weights from this file if never observed"},
	{ "-inmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Use mean from this file if never observed"},
	{ "-invarfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Use var from this file if never observed"},
	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The feature set to use." },
	{ "-ceplen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The vector length of the source features (e.g. MFCC)" },

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
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.8  97/07/16  11:22:39  eht
 * Allow an inmixfn for those mixing weights that were unseen in the accumulators
 * 
 * Revision 1.7  97/03/07  08:53:11  eht
 * - added -inmeanfn and -invarfn arguments for unseen means and vars
 * 
 *
 */
