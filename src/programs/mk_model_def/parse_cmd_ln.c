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
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#include <s3/cmd_ln.h>

#include "parse_cmd_ln.h"

#include <stdlib.h>

int
parse_cmd_ln(int argc, char *argv[])
{
    static arg_def_t defn[] = {
	{ "-moddeffn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-III model definition file name" },
	{ "-triphonefn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-II triphone file name" },
	{ "-phonelstfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "List of phones (first 4 columns of phone def'ns)"},
	{ "-mapfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "A SPHINX-II senone mapping file name" },

	{ "-n_cdstate",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "total # of CD senones/tied_states (even though mapfn may reference fewer)"},
	{ "-n_cistate",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "# of CI senones/tied_states"},
	{ "-n_tmat",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "# of tied transition matrices"},
	{ "-n_state_pm",
	  CMD_LN_INT32,
	  CMD_LN_NO_VALIDATION,
	  "5",
	  "# of states/model"},
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
 * Revision 1.2  2000/09/29  22:35:14  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.3  97/07/16  11:35:08  eht
 * Added -phonelstfn argument
 * 
 * Revision 1.2  97/03/07  09:01:21  eht
 * reformatting
 * 
 * Revision 1.1  1995/06/02  20:34:16  eht
 * Initial revision
 *
 *
 */
