/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * file: cmd_ln.c
 * 
 * Description: 
 *	This library parses command line arguments and provides
 *	an interface for accessing them.
 *
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include <s3/common.h>
#include <s3/s3.h>	/* defines TRUE and FALSE among other things */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void
cmd_ln_initialize(void)
{
}


/*********************************************************************
 *
 * Function: alloc_type	(local scope)
 * 
 * Description: 
 * 	Initializes the command line parsing subsystem
 * 
 * Traceability: 
 * 
 * Function Inputs: 
 *	arg_type_t t -
 *		A command line argument type.
 * 
 * Return Values: 
 *	A pointer to an allocated item of the given type.  An
 *	exception to this is the string type.  There is no
 *	allocation needed for this as it is already represented
 *	by a pointer type.
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

static void *
alloc_type(arg_type_t t)
{
    void *mem = NULL;

    switch (t) {
	case CMD_LN_INT32:

	mem = ckd_malloc(sizeof(int32));
	break;
	
	case CMD_LN_FLOAT32:

	mem = ckd_malloc(sizeof(float32));
	break;
	
	case CMD_LN_FLOAT64:

	mem = ckd_malloc(sizeof(float64));
	break;

	case CMD_LN_STRING:
	mem = NULL;	/* no allocations done for string type */
	break;
	
	case CMD_LN_STRING_LIST:
	E_FATAL("alloc_type(CMD_LN_STRING_LIST) not supported\n");
	break;
	
	case CMD_LN_BOOLEAN:

	mem = ckd_malloc(sizeof(int32));
	break;

	case CMD_LN_UNDEF:
	E_FATAL("undefined argument type\n");
    }

    return mem;
}

static void *
arg_to_str(arg_type_t t, void *val)
{
    static char *big_str = NULL;
    char **strlst;
    uint32 i, l;

    if (big_str) {
	ckd_free(big_str);
	big_str = NULL;
    }

    switch (t) {
    case CMD_LN_INT32:
	big_str = ckd_malloc(16);
	sprintf(big_str, "%d", *(int32 *)val);
	break;
	
    case CMD_LN_FLOAT32:
	big_str = ckd_malloc(16);
	sprintf(big_str, "%e", *(float32 *)val);
	break;
	
    case CMD_LN_FLOAT64:
	big_str = ckd_malloc(16);
	sprintf(big_str, "%e", *(float64 *)val);
	break;
	
    case CMD_LN_STRING:
	big_str = ckd_malloc(strlen((char *)val) + 1);
	strcpy(big_str, val);
	break;
	
    case CMD_LN_STRING_LIST:
	strlst = (char **)val;

	l = strlen(strlst[0]);
	for (i = 1; strlst[i] != NULL; i++) {
	    l += strlen(strlst[i]) + 1;
	}
	++l;

	big_str = ckd_malloc(l);

	strcpy(big_str, strlst[0]);
	for (i = 1; strlst[i] != NULL; i++) {
	    strcat(big_str, " ");
	    strcat(big_str, strlst[i]);
	}

	break;
	
    case CMD_LN_BOOLEAN:
	big_str = ckd_malloc(4);
	
	sprintf(big_str, "%s",
		(*(int32 *)val ? "yes" : "no"));
	break;
	
    case CMD_LN_UNDEF:
	E_FATAL("undefined argument type\n");
    }

    return big_str;
}

static arg_def_t *defn_list = NULL;
static uint32 defn_list_len = 0;
static void **parsed_arg_list = NULL;

#define SWITCH_HEADER	"[Switch]"
#define DEFAULT_HEADER	"[Default]"
#define VALUE_HEADER	"[Value]"
#define DOC_HEADER	"[Description]"

uint32
strlst_len(char **lst)
{
    uint32 len;
    uint32 i;

    
    len = strlen(lst[0]);

    for (i = 1; lst[i] != NULL; i++) {
	len += strlen(lst[i]) + 1;
    }

    return len;
}

void
cmd_ln_print_configuration()
{
    uint32 i;
    uint32 len;
    uint32 mx_sw_len = 0;
    uint32 mx_df_len = 0;
    char fmt_str[64];

    if (defn_list == NULL) {
	E_WARN("No switches defined.  None printed\n");

	return;
    }

    for (i = 0; i < defn_list_len; i++) {
	len = strlen(defn_list[i].switch_name);
	if (len > mx_sw_len) mx_sw_len = len;

	if (defn_list[i].default_value) {
	    if (defn_list[i].type != CMD_LN_STRING_LIST)
		len = strlen(defn_list[i].default_value);
	    else
		len = strlst_len(defn_list[i].default_value);

	    if (len > mx_df_len) mx_df_len = len;
	}
    }

    if (mx_sw_len < strlen(SWITCH_HEADER)) mx_sw_len = strlen(SWITCH_HEADER);
    if (mx_df_len < strlen(DEFAULT_HEADER)) mx_df_len = strlen(DEFAULT_HEADER);
    
    sprintf(fmt_str, "%%-%ds %%-%ds %%-%ds\n",
	    mx_sw_len, mx_df_len, strlen(VALUE_HEADER));
    fprintf(stderr, fmt_str, SWITCH_HEADER, DEFAULT_HEADER, VALUE_HEADER);

    for (i = 0; i < defn_list_len; i++) {
	fprintf(stderr,
		fmt_str,
		defn_list[i].switch_name,
		(defn_list[i].default_value ? defn_list[i].default_value : ""),
		(parsed_arg_list[i]? arg_to_str(defn_list[i].type,
						parsed_arg_list[i]) : ""));
    }
    
    fflush(stderr);
}

void
cmd_ln_print_definitions()
{
    uint32 i;
    uint32 len;
    uint32 mx_sw_len = 0;
    uint32 mx_df_len = 0;
    uint32 mx_ds_len = 0;
    char fmt_str[64];

    if (defn_list == NULL) {
	E_WARN("No switches defined.  None printed\n");

	return;
    }

    for (i = 0; i < defn_list_len; i++) {
	len = strlen(defn_list[i].switch_name);
	if (len > mx_sw_len) mx_sw_len = len;

	if (defn_list[i].default_value) {
	    len = strlen(defn_list[i].default_value);
	    if (len > mx_df_len) mx_df_len = len;
	}

	if (defn_list[i].doc) {
	    len = strlen(defn_list[i].doc);
	    if (len > mx_ds_len) mx_ds_len = len;
	}
    }

    if (mx_sw_len < strlen(SWITCH_HEADER)) mx_sw_len = strlen(SWITCH_HEADER);
    if (mx_df_len < strlen(DEFAULT_HEADER)) mx_df_len = strlen(DEFAULT_HEADER);
    if (mx_ds_len < strlen(DOC_HEADER)) mx_ds_len = strlen(DOC_HEADER);

    sprintf(fmt_str, "%%-%ds %%-%ds %%-%ds\n", mx_sw_len, mx_df_len, mx_ds_len);
    fprintf(stderr, fmt_str, SWITCH_HEADER, DEFAULT_HEADER, DOC_HEADER);

    for (i = 0; i < defn_list_len; i++) {
	fprintf(stderr,
		fmt_str,
		defn_list[i].switch_name,
		(defn_list[i].default_value ? defn_list[i].default_value : ""),
		(defn_list[i].doc ? defn_list[i].doc : ""));
    }

    fflush(stderr);
}

static void
free_and_alloc(void **arg_ptr, arg_type_t t)
{
    if (*arg_ptr)
	ckd_free(*arg_ptr);
    
    *arg_ptr = alloc_type(t);
}

static void
free_and_alloc_strlst(void **arg_ptr, uint32 new_len)
{
    if (*arg_ptr)
	ckd_free(*arg_ptr);
    
    *arg_ptr = ckd_calloc(new_len, sizeof(char *));
}

int
parse_arg(uint32 defn_idx, uint32 argc, char *argv[], int start)
{
    arg_type_t arg_type;
    int n_parsed = -1;
    uint32 end;
    uint32 len;
    char *cur_arg;
    uint32 i;
    char **strlst;

    arg_type = defn_list[defn_idx].type;

    cur_arg = argv[start];

    switch (arg_type) {
    case CMD_LN_INT32:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	*((int *)parsed_arg_list[defn_idx]) = atoi(cur_arg);
	n_parsed = 1;
	break;

    case CMD_LN_FLOAT32:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	*((float *)parsed_arg_list[defn_idx]) = (float)atof(cur_arg);
	n_parsed = 1;
	break;

    case CMD_LN_FLOAT64:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	*((double *)parsed_arg_list[defn_idx]) = atof(cur_arg);
	n_parsed = 1;
	break;

    case CMD_LN_STRING:
	parsed_arg_list[defn_idx] = (void *)cur_arg;
	n_parsed = 1;
	break;

    case CMD_LN_STRING_LIST:
	if (start > 0) {
	    /* start > 0 always the case if parsing a command line. */
	    for (i = start+1; i < argc; i++) {
		if (argv[i][0] == '-')
		    break;
	    }
	}
	else {
	    /* start == 0 always the case if parsing a default */

	    assert(argc == 0);	/* caller must ensure this. */

	    for (i = start; argv[i] != NULL; i++);
	}
	
	end = i;

	len = end - start;
	len++;	/* need space for terminating NULL */

	free_and_alloc_strlst(&parsed_arg_list[defn_idx], len);

	strlst = parsed_arg_list[defn_idx];
	for (i = start; i < end; i++) {
	    strlst[i-start] = argv[i];
	}

	n_parsed = len-1;
	break;
	
    case CMD_LN_BOOLEAN:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	if ((cur_arg[0] == 'y') || (cur_arg[0] == 't') ||
	    (cur_arg[0] == 'Y') || (cur_arg[0] == 'T')) {
	    *(int *)parsed_arg_list[defn_idx] = TRUE;
	    n_parsed = 1;
	}
	else if ((cur_arg[0] == 'n') || (cur_arg[0] == 'f') ||
		 (cur_arg[0] == 'N') || (cur_arg[0] == 'F')) {
	    *(int *)parsed_arg_list[defn_idx] = FALSE;
	    n_parsed = 1;
	}
	else {
	    E_ERROR("Unparsed boolean value '%s'\n", cur_arg);
	    n_parsed = -1;
	}
	break;

    case CMD_LN_UNDEF:
	E_FATAL("Definition for argument %s has undefined type\n",
		defn_list[defn_idx].switch_name);

    default:
	E_FATAL("No case in switch() {} for enum value.\n");
    }

    return n_parsed;
}

int
cmd_ln_define(arg_def_t *defn)
{
    uint32 i;

    assert(defn != NULL);

    defn_list = defn;

    for (i = 0; defn_list[i].switch_name != NULL; i++);
    defn_list_len = i;

    parsed_arg_list = ckd_calloc(defn_list_len, sizeof(void *));

    for (i = 0; i < defn_list_len; i++) {
	if (defn_list[i].default_value) {
	    if (defn_list[i].type != CMD_LN_STRING_LIST) {
		parse_arg(i,
			  0,	/* ignored if start == 0 */
			  (char **)&defn_list[i].default_value,
			  0);
	    }
	    else {
		parse_arg(i,
			  0,	/* ignored if start == 0 */
			  defn_list[i].default_value,
			  0);
	    }
	}
    }

    return 0;
}
static int did_parse = FALSE;
static int skip_cmd_print = FALSE;

void
cmd_ln_skip_print(int state)
{
    skip_cmd_print = state;
}

static void
cmd_ln_print_cmd_ln(int argc, char *argv[])
{
    int i;

    if (skip_cmd_print)
	return;

    fprintf(stderr, "%s", argv[0]);

    for (i = 1; i < argc; i++) {
	if ((argv[i][0] == '-') && isalpha(argv[i][1])) {
	    fprintf(stderr, " \\\n");
	}
	fprintf(stderr, " %s", argv[i]);
    }
    fprintf(stderr, "\n\n");
}


int
cmd_ln_parse(int argc, char *argv[])
{
    int i;
    uint32 j;
    int err;
    int n_arg_parsed;

    if (defn_list == NULL) {
	E_WARN("No switches defined.  None parsed\n");

	did_parse = TRUE;

	return 0;
    }

    cmd_ln_print_cmd_ln(argc, argv);

    for (i = 1, err = 0; i < argc; i++) {
	if (argv[i][0] != '-') {
	    E_ERROR("Expecting '%s -switch_1 <arg_1> -switch_2 <arg_2> ...'\n",
		    argv[0]);

	    err = 1;
	    break;
	}

	for (j = 0; j < defn_list_len; j++) {
	    if (strcmp(argv[i], defn_list[j].switch_name) == 0) {

		n_arg_parsed = parse_arg(j, argc, argv, i+1);
		if (n_arg_parsed < 0) {
		    err = 1;
		}

		i += n_arg_parsed;	/* i incremented for each switch as well */

		break;
	    }
	}

	if (j == defn_list_len) {
	    E_ERROR("Unknown switch %s seen\n", argv[i]);

	    err = 1;
	}
    }

    if (err) {
	exit(1);
    }

    did_parse = TRUE;
    
    return 0;
}

int
cmd_ln_validate()
{
    uint32 i;
    int err;

    if (!did_parse) {
	E_FATAL("cmd_ln_parse() must be called before cmd_ln_validate()\n");
    }

    for (i = 0, err = 0; i < defn_list_len; i++) {
	if (defn_list[i].validate_arg) {
	    if (defn_list[i].validate_arg(defn_list[i].switch_name,
					  parsed_arg_list[i]) == FALSE) {
		err = 1;
	    }
	}
    }

    if (err) return FALSE;

    return TRUE;
}

const void *cmd_ln_access(char *switch_name)
{
    uint32 i;

    if (!did_parse) {
	E_FATAL("cmd_ln_parse() must be called before cmd_ln_access()\n");
    }

    for (i = 0; i < defn_list_len; i++) {
	if (strcmp(defn_list[i].switch_name, switch_name) == 0) {
	    break;
	}
    }

    if (i == defn_list_len) {
	E_FATAL("Unknown switch %s\n", switch_name);
    }

    return parsed_arg_list[i];
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:13  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.14  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.13  1996/07/29  16:36:56  eht
 * added a print command line function to allow easier debugging
 * dealt w/ missing case of switch statement
 *
 * Revision 1.12  1996/06/17  14:31:21  eht
 * Got rid of value length when determining total space to allocate for a line.  This will get rid of the
 * problem where one gigantic line will make all the lines gigantic on printout
 *
 * Revision 1.11  1995/12/15  18:37:07  eht
 * Added some type cases for memory alloc/free
 *
 * Revision 1.10  1995/10/10  15:13:22  eht
 * Undo (unsigned) typecast because on further inspection it is
 * unnecessary (strlen() returns a size_t value which is\
 * unsigned)
 *
 * Revision 1.8  1995/10/10  14:46:37  eht
 * Added (unsigned) typecast to strlen() return value to make some
 * compilers happy.
 *
 * Revision 1.7  1995/10/10  13:55:57  eht
 * Moved definitions of free_and_alloc* stuff to above where
 * references are.
 *
 * Revision 1.6  1995/10/10  13:53:27  eht
 * Included <string.h> since strcpy, etc. are used.
 * Got rid of a few unreferenced local variables.
 * Moved parse_arg() definition above reference.
 *
 * Revision 1.5  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.4  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.3  1995/09/07  19:57:24  eht
 * Add defn of TRUE and FALSE for machines like
 * HPUX HP machines
 *
 * Revision 1.2  1995/06/02  14:52:54  eht
 * Use pwp's error printing package
 *
 * Revision 1.1  1995/02/13  15:47:41  eht
 * Initial revision
 *
 *
 */
