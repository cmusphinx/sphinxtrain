/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * File: parse_cmd_ln.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#include <stdlib.h>
#include "s3/cmd_ln.h"

#include "parse_cmd_ln.h"

int32 parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-moddeffn",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "The model definition file for the model inventory to train" },
	{ "-mixwfn",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      "The mixture weight parameter file name"},

	{ "-tpfloor",
	      CMD_LN_FLOAT32,
	      CMD_LN_NO_VALIDATION,
	      "0.0001",
	      "Transition probability smoothing floor" },

	{ "-hmmdir",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              CMD_LN_NO_DEFAULT,
              "A directory containing SPHINX-II models consistent with -moddeffn" },
	{ "-hmmext",
	      CMD_LN_STRING,
	      CMD_LN_NO_VALIDATION,
	      "chmm",
	      "Extension of a SPHINX-II model file." },


        { "-cepsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "ccode",
              "Extension of cepstrum senone weight file" },
        { "-dcepsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "d2code",
              "Extension of difference cepstrum senone weight file" },
        { "-powsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "p3code",
              "Extension of power senone weight file" },
        { "-2dcepsenoext",
              CMD_LN_STRING,
              CMD_LN_NO_VALIDATION,
              "xcode",
             "Extension of 2nd order difference cepstrum senone weight file" },

	{ NULL,
	      CMD_LN_UNDEF,
	      CMD_LN_NO_VALIDATION,
	      CMD_LN_NO_DEFAULT,
	      NULL }
    };

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    cmd_ln_print_configuration();

    return 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2004/07/21  19:17:24  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.2  2001/02/20 00:28:35  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/11/22 21:23:18  awb
 * *** empty log message ***
 *
 * Revision 1.1  95/09/08  19:11:14  eht
 * Initial revision
 * 
 *
 */
