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

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>

#ifndef ARG_DEFN_H
    const char helpstr[] =  
"Description: \n\
\n\
Given a set of MLLR transform, mllr_transform can transform \n\
the mean according to formulat y=Ax+b.\n \
\n \
The output and input files are specified by -outmeanfn and \n\
 -inmeanfn respectively. You may also transform the context-\n\
dependent model using the option -cdonly.  In that case you \n\
need to specify a model definition using -moddeffn.";

    const char examplestr[] = 
"Example: \n\
  The simplest case: \n\
  mllr_transform -inmeanfn inMeans -outmeanfn outMeans  -mllrmat matrix \n\
  \n\
  Adapt only on CD phones: \n\
  mllr_transform  -inmeanfn inMeans -outmeanfn outMeans  -mllrmat matrix -cdonly yes -moddeffn mdef \n\
  \n\
  Help and example: \n\
  nmllr_transform -help yes -example yes ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-inmeanfn",
	  ARG_STRING,
	  NULL,
	  "Input Gaussian mean file name"},

	{ "-outmeanfn",
	  ARG_STRING,
	  NULL,
	  "Output Gaussian mean file name"},

	{ "-ingaucntfn",
	  ARG_STRING,
	  NULL,
	  "Input Gaussian accumulation count file name"},

	{ "-outgaucntfn",
	  ARG_STRING,
	  NULL,
	  "Output Gaussian accumulation count file name"},

	{ "-mllrmat",
	  ARG_STRING,
	  NULL,
	  "The MLLR matrix file"},

	{ "-cb2mllrfn",
	  ARG_STRING,
	  ".1cls.",
	  "The codebook-to-MLLR class file. Override option -cdonly"},

        { "-cdonly",
          ARG_BOOLEAN,
          "no",
          "Use CD senones only. -moddeffn must be given."},

        { "-inverse",
          ARG_BOOLEAN,
          "no",
          "Apply the inverse transform (for speaker adaptive training)."},

        { "-moddeffn",
          ARG_STRING,
          NULL,
          "Model Definition file. "},

	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "Variance (baseline-var, or error-var) file (NOT USED!!!)"},

        { "-varfloor",
          ARG_FLOAT32,
          "1e-3",
          "Variance floor value (NOT USED!!!)"},

	{NULL, 0, NULL, NULL},
    };
#define ARG_DEFN_H

#endif /* ARG_DEFN_H */ 
