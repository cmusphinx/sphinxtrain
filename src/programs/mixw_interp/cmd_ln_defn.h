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
 * 	Command line definitions for bw
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef CMD_LN_DEFN_H
#define CMD_LN_DEFN_H
    static arg_def_t defn[] = {

        { "-SImixwfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The SI mixture weight parameter file name"},

        { "-SDmixwfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The SD mixture weight parameter file name"},

        { "-tokencntfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The token count file"},

        { "-outmixwfn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "The output interpolated mixture weight parameter file name"},

        { "-SIlambda",
          CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
          "0.5",
          "Weight given to SI mixing weights" },

	{ NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
    };


#endif /* CMD_LN_DEFN_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 *
 */
