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
    
    const char helpstr[] = 
"Description: 

Given a set of MLLR transform, mllr_transform can transform 
the mean according to formulat y=Ax+b.
The output and input files are specified by -outmeanfn and
 -inmeanfn respectively. You may also transform the context-
dependent model using the option -cdonly.  In that case you
 need to specify a model definition using -moddeffn.";

    const char examplestr[] = 
"Example: 

The simplest case:

mllr_transform -inmeanfn inMeans -outmeanfn outMeans  -mllrmat matrix 

Adapt only on CD phones:

mllr_transform  -inmeanfn inMeans -outmeanfn outMeans  -mllrmat matrix 
-cdonly yes -moddeffn mdef 

Help and example:

mllr_transform -help yes -example yes
";

    static arg_def_t defn[] = {
	{ "-help",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-inmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Input Gaussian mean file name"},

	{ "-outmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output Gaussian mean file name"},


	{ "-mllrmat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The MLLR matrix file"},

	{ "-cb2mllrfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  ".1cls.",
	  "The codebook-to-MLLR class file. Override option -cdonly"},

        { "-cdonly",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "no",
          "Use CD senones only. -moddeffn must be given."},

        { "-varfloor",
          CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
          "1e-2",
          "Value of the variance floor. Mainly for smoothing the mean."},

        { "-moddeffn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "Model Definition file. "},

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Gaussian variance file name. For smoothing."},


        { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION,    CMD_LN_NO_DEFAULT, NULL }
    };
#define CMD_LN_DEFN_H

#endif /* CMD_LN_DEFN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2004/08/03  07:31:17  arthchan2003
 * make the changes more latex friendly\n
 * 
 * Revision 1.2  2004/08/03 07:23:12  arthchan2003
 * Check in the code for usage and example of mllr_transform
 *
 * Revision 1.1  2004/07/26 05:04:20  arthchan2003
 * mllr_transform committed, it is an adaptation of Sam Joo's mllr_adapt
 *
 *
 */
