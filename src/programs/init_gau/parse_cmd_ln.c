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
 * 	These routines define and parse the command line for init_gau.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/s3.h>

#include <sys_compat/misc.h>

#include <stdio.h>
#include <assert.h>


/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Model definition file for the single density HMM's to initialize"},
	{ "-ts2cbfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Tied-state-to-codebook mapping file"},
	{ "-accumdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Where to write mean/var counts"},
	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Mean file for variance initialization"},
	{ "-ctlfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Control file of the training corpus"},
	{ "-nskip",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "# of lines to skip in the control file"},
	{ "-runlen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "# of lines to process in the control file (after any skip)"},
	{ "-part",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Identifies the corpus part number (range 1..NPART)" },
	{ "-npart",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Partition the corpus into this many equal sized subsets" },
	{ "-lsnfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "All word transcripts for the training corpus (consistent order w/ -ctlfn!)"},
	{ "-dictfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Dictionary for the content words"},
	{ "-fdictfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Dictionary for the filler words"},
	{ "-segdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Root directory of the training corpus state segmentation files."},
	{ "-segext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "v8_seg",
	  "Extension of the training corpus state segmentation files."},
/* ADDED BY BHIKSHA, 7 JAN 98; SCALE SEGMENTATIONS FOR FEATURE STREAMS
   OF A DIFFERENT LENGTH THAN CEPSTRA */
	{ "-scaleseg",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Scale existing segmentation to fit new parameter stream length."},
/* END CHANGES BY BHIKSHA */
	{ "-cepdir",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Root directory of the training corpus cepstrum files."},
	{ "-cepext",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "mfc",
	  "Extension of the training corpus cepstrum files."},
	{ "-silcomp",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "none",
	  "Controls silence compression."},
	{ "-cmn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "current",
	  "Controls cepstral mean normalization."},
	{ "-varnorm",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Controls variance normalization."},
	{ "-agc",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "max",
	  "Controls automatic gain control."},
	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Controls which feature extraction algorithm is used."},
	{ "-ceplen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "13",
	  "# of components in cepstrum vector"},
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
	E_FATAL("Unable to validate command line\n");
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
 * Revision 1.8  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.7  96/08/06  14:43:02  eht
 * Added error output
 * 
 * Revision 1.6  1996/08/06  14:15:34  eht
 * Create prototype for parse_cmd_ln()
 *
 * Revision 1.5  1996/03/25  15:44:11  eht
 * reformatting
 *
 * Revision 1.4  1996/01/26  18:22:07  eht
 * Add the "-feat" argument
 *
 * Revision 1.3  1995/12/14  20:02:49  eht
 * Added arguments for front end processing (-cmn, -silcomp, -agc)
 *
 * Revision 1.2  1995/12/01  20:55:40  eht
 * interim development version
 *
 * Revision 1.1  1995/12/01  17:01:50  eht
 * Initial revision
 *
 * Revision 1.1  1995/10/05  12:54:02  eht
 * Initial revision
 *
 * Revision 1.2  1995/09/07  19:59:50  eht
 * Include defn for TRUE/FALSE
 *
 * Revision 1.1  1995/06/02  20:55:11  eht
 * Initial revision
 *
 *
 */
