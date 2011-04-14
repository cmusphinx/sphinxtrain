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

#ifndef ARG_DEFN_H
    const char helpstr[] =  
"Description: \n\
\n\
Given a set of mean accumulator, mllr_solve can compute the transform \n\
matrix based on the maximum likelihood criteria. \n\
\n\
The mean and variance are required to be input in arguments -meanfn and -varfn \n\
For linear regression equation y=Ax+b, \n\
If you specific only -mllrmult, then only A will be estimated. \n\
If you specific only -mllradd, then only b will be estimated. ";

    const char examplestr[] = 
"Example: \n\
  The simplest case: \n\
  mllr_solve -outmllrfn output.matrix -accumdir accumdir -meanfn mean -varfn var \n\
  \n\
  Adapt based on only CD-senones \n\
  mllr_solve -outmllrfn output.matrix -accumdir accumdir -meanfn mean -varfn var -cdonly yes -moddeffn mdef. \n\
  \n\
  Only adapt on A :\n\
  mllr_solve -outmllrfn output.matrix -accumdir accumdir -meanfn mean -varfn var -mllrmult yes -mllradd no \n\
  \n\
  help and example:\n\
  mllr_solve -help yes -example yes ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-outmllrfn",
	  ARG_STRING,
	  NULL,
	  "Output MLLR regression matrices file"},

	{ "-accumdir",
	  ARG_STRING_LIST,
	  NULL,
	  "One or more paths containing reestimation sums from bw" },

	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "Baseline Gaussian density mean file"},

	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "variance (baseline-var, or error-var) file"},

	{ "-fullvar",
	  ARG_BOOLEAN,
	  "no",
	  "Variances are full covariance matrices"},

	{ "-cb2mllrfn",
	  ARG_STRING,
	  ".1cls.",
	  "Codebook to mllr class mapping index file (If it is given, ignore -cdonly)"},

        { "-cdonly",
          ARG_BOOLEAN,
          "no",
          "Use only CD senones for MLLR (If yes, -moddeffn should be given.)"},

	{ "-moddeffn",
	  ARG_STRING,
	  NULL,
	  "Model Definition file (to get CD starting point for MLLR)"},

        { "-mllrmult",
          ARG_BOOLEAN,
          "yes",
          "Reestimate full multiplicative term of MLLR adatpation of means (yes/no)"},

        { "-mllradd",
          ARG_BOOLEAN,
          "yes",
          "Reestimate shift term of MLLR adaptation of means (yes/no)"},

        { "-varfloor",
          ARG_FLOAT32,
          "1e-3",
          "var floor value"},
	
	{NULL, 0, NULL, NULL},
    };
#define ARG_DEFN_H

#endif /* ARG_DEFN_H */ 

