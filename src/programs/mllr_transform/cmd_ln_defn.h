/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1998 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: cmd_ln_defn.h
 * 
 * Description: 
 * 	Command line argument definition
 *
 * Author: 
 *      Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/cmd_ln.h>
#include <s3/err.h>

#ifndef CMD_LN_DEFN_H

    static arg_def_t defn[] = {
	{ "-outmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The output (adapted) Gaussian mean file name"},

	{ "-inmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A input Gaussian mean file name"},

	{ "-mllrmat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The Mllr matrices file"},

	{ "-cb2mllrfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  ".1cls.",
	  "The codebook-to-MLLR class file (If it is given, ignore -cdonly)"},

        { "-cdonly",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "no",
          "Use CD senones only (If yes, -moddeffn should be given.)"},

        { "-varfloor",
          CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
          "1e-2",
          "var floor (It's for smoothing out mean. A larger value than other cases.)"},

        { "-moddeffn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "Model Definition file (to get CD starting point for MLLR)"},

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A Gaussian var file name for smoothing (hack!!!)"},

        { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION,    CMD_LN_NO_DEFAULT, NULL }
    };
#define CMD_LN_DEFN_H

#endif /* CMD_LN_DEFN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2004/07/26  05:04:20  arthchan2003
 * mllr_transform committed, it is an adaptation of Sam Joo's mllr_adapt
 * 
 *
 */
