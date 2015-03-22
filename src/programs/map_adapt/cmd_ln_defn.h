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
 * 	Command line definitions for map_adapt
 *
 * Author: 
 * 	David Huggins-Daines <dhuggins@cs.cmu.edu>
 *********************************************************************/

#ifndef ARG_DEFN_H
#define ARG_DEFN_H
const char helpstr[] = 
"Description: \n"
"\n"
"Given a speaker-independent (or other baseline) model and a\n"
"set of forward-backward statistics collected from adaptation\n"
"data using this model as initialization, map_adapt will update\n"
"the model parameters to maximize the a posteriori probability\n"
"of the adaptation data given a prior distribution derived from\n"
"the baseline model.\n";

const char examplestr[] =
"Example: \n"
"map_adapt -mapmeanfn map_model/means -meanfn baseline/means \\\n"
"          -varfn baseline/variances -accumdir bwaccumdir\n";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-meanfn",
	  ARG_STRING,
	  NULL,
	  "Baseline (speaker-independent) Gaussian density mean file"},

	{ "-varfn",
	  ARG_STRING,
	  NULL,
	  "Baseline (speaker-independent) Gaussian density variance file"},

	{ "-tmatfn",
	  ARG_STRING,
	  NULL,
	  "Baseline (speaker-independent) transition matrix parameter file name"},

	{ "-mixwfn",
	  ARG_STRING,
	  NULL,
	  "Baseline (speaker-independent) mixture weight parameter file name"},

	{ "-accumdir",
	  ARG_STRING_LIST,
	  NULL,
	  "One or more paths containing reestimation sums from running bw on adaptation data" },

	{ "-fixedtau",
	  ARG_BOOLEAN,
	  "no",
	  "Use a uniform value for the prior hyperparameter tau"},

	{ "-bayesmean",
	  ARG_BOOLEAN,
	  "yes",
	  "Use simple Bayesian updating for the means, ignoring tau (recommended)."},

	{ "-tau",
	  ARG_FLOAT32,
	  "10.0",
	  "The uniform value for the tau (prior weight) hyperparameter"},

	{ "-mwfloor",
	  ARG_FLOAT32,
	  "0.00001",
	  "Mixing weight smoothing floor" },

	{ "-varfloor",
	  ARG_FLOAT32,
	  "0.00001",
	  "Variance smoothing floor" },

	{ "-tpfloor",
	  ARG_FLOAT32,
	  "0.0001",
	  "Transition probability smoothing floor" },

        { "-mapmeanfn",
          ARG_STRING,
          NULL,
          "The output MAP mean file"},

        { "-mapvarfn",
          ARG_STRING,
          NULL,
          "The output MAP var file"},

        { "-mapmixwfn",
          ARG_STRING,
          NULL,
          "The output MAP mixture weight file"},

        { "-maptmatfn",
          ARG_STRING,
          NULL,
          "The output MAP transition matrix file"},

	{ "-ts2cbfn",
	  ARG_STRING,
	  NULL,
	  "Tied-state-to-codebook mapping file name" },

        { "-moddeffn",
          ARG_STRING,
          NULL,
          "The model definition file for the model inventory to train" },
                                      
                                      
	{NULL, 0, NULL, NULL},
    };


#endif /* ARG_DEFN_H */ 
