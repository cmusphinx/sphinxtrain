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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * File: parse_cmd_ln.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/common.h>
#include <s3/s3.h>	/* pick up defn of TRUE/FALSE */

#include <stdio.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

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
	  "The model definition file name"},
	{ "-mixwfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The mixture weight parameter file name"},
	{ "-accumdirs",
	  CMD_LN_STRING_LIST,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A path where accumulated counts are to be read." },
	{ "-cilambda",
	  CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
	  "0.9",
	  "Weight of CI distributions with respect to uniform distribution"},
	{ "-maxiter",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "100",
	  "max # of iterations if no lambda convergence"},
	{ "-feat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  "4s_12c_24d_3p_12dd",
	  "feature stream definition" },
	{ "-ceplen",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "13",
	  "Input feature vector length (e.g. MFCC)"},
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
	exit(1);
    }

    cmd_ln_print_configuration();

    return 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:14  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.6  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.5  1996/03/25  15:40:25  eht
 * Added ability to set input feature vector length
 *
 * Revision 1.4  1996/01/26  18:07:00  eht
 * Add the "-feat" argument
 *
 * Revision 1.3  1995/09/07  20:01:05  eht
 * include defn of TRUE/FALSE for machines like HP's running HPUX
 *
 * Revision 1.2  1995/08/09  00:38:05  eht
 * Another development version
 *
 * Revision 1.1  1995/06/02  20:56:53  eht
 * Initial revision
 *
 *
 */
