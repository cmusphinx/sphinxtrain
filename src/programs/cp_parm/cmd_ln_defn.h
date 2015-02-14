/* ====================================================================
 * Copyright (c) 1998-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*********************************************************************
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
#define ARG_DEFN_H
  const char helpstr[] =
"Description: \n\
Copy parameters such as means and variances from one model to another model.  \n\
You need to specify a \"copy operation file\" which each operation looks like \n\
this \n\
dest_idx1 src_idx1 \n\
dest_idx2 src_idx2 \n\
dest_idx3 src_idx3 \n\
.\n\
.\n\
.\n\
For example, the first line will instruct cp_param copy src model index 1 to \n\
destionation model index 1. \n\
This tool is still under heavy development at 20040807 ";
  const char examplestr[] =
"Example: \n\
This example copy mean from a single file from the source file to 5 mean vector in  \n\
th destination file. \n\
First you need to prepare a file like this \n\
0 0 \n\
1 0 \n\
2 0 \n\
3 0 \n\
4 0 \n\
\n\
Lets call it cp_op \n\
cp_parm -cpopsfn  cp_op \n\
        -igaufn   globalmean \n\
        -ncbout   5 \n\
        -ogaufn   out.means \n\
        -feat    [Your feature type] ";

    static arg_t defn[] = {
	{ "-help",
	  ARG_BOOLEAN,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  ARG_BOOLEAN,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-cpopsfn",
	  ARG_STRING,
	  NULL,
	  "Copy operation file name" },

	{ "-imixwfn",
	  ARG_STRING,
	  NULL,
	  "Input mixing weight file" },
	{ "-omixwfn",
	  ARG_STRING,
	  NULL,
	  "Output mixing weight file" },
	{ "-nmixwout",
	  ARG_INT32,
	  NULL,
	  "# of mixing weight arrays in the output file" },
	  
	{ "-itmatfn",
	  ARG_STRING,
	  NULL,
	  "Input transition matrix file" },
	{ "-otmatfn",
	  ARG_STRING,
	  NULL,
	  "Output transition matrix file" },
	{ "-ntmatout",
	  ARG_INT32,
	  NULL,
	  "# of transition matrices in the output file" },

	{ "-igaufn",
	  ARG_STRING,
	  NULL,
	  "Input Gaussian density parameter file" },
	{ "-ogaufn",
	  ARG_STRING,
	  NULL,
	  "Output Gaussian density parameter file" },

	{ "-ifullgaufn",
	  ARG_STRING,
	  NULL,
	  "Input Gaussian covariance matrix file" },
	{ "-ofullgaufn",
	  ARG_STRING,
	  NULL,
	  "Output Gaussian covariance matrix file" },

	{ "-ncbout",
	  ARG_INT32,
	  NULL,
	  "# of codebooks in the output file" },
	
	{NULL, 0, NULL, NULL},
    };
#define ARG_DEFN_H

#endif /* ARG_DEFN_H */ 

