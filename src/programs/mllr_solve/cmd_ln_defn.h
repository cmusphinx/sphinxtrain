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

#ifndef CMD_LN_DEFN_H
    static arg_def_t defn[] = {
	{ "-outmllrfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output MLLR regression matrices file"},

	{ "-accumdir",
	  CMD_LN_STRING_LIST,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Paths containing reestimation sums from bw" },

	{ "-meanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Baseline Gaussian density mean file"},

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "variance (baseline-var, or error-var) file"},


	{ "-cb2mllrfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  ".1cls.",
	  "Codebook to mllr class mapping index file (If it is given, ignore -cdonly)"},

        { "-cdonly",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "no",
          "Use only CD senones for MLLR (If yes, -moddeffn should be given.)"},

	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Model Definition file (to get CD starting point for MLLR)"},

        { "-mllrmult",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "yes",
          "Reestimate full multiplicative term of MLLR adatpation of means (yes/no)"},

        { "-mllradd",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "yes",
          "Reestimate shift term of MLLR adaptation of means (yes/no)"},

        { "-varfloor",
          CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
          "1e-3",
          "var floor value"},

    { NULL, CMD_LN_UNDEF,CMD_LN_NO_VALIDATION,CMD_LN_NO_DEFAULT, NULL }
    };
#define CMD_LN_DEFN_H

#endif /* CMD_LN_DEFN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2004/07/27  12:09:26  arthchan2003
 * Missing the whole directory of mllr_solve
 * 
 *
 */
