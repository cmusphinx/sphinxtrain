/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: cmd_ln.h
 * 
 * Description: 
 *	This module defines, parses and manages the validation
 * of command line arguments.
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 *
 *********************************************************************/

#ifndef S3CMD_LN_H
#define S3CMD_LN_H

#include <s3/prim_type.h>

#ifndef NULL
#define NULL	(void *)0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

typedef int (*validation_fn_t)(char *switch_name, void *arg);

#define CMD_LN_NO_VALIDATION	(validation_fn_t)NULL
#define CMD_LN_NO_DEFAULT	(char *)NULL

typedef enum {
    CMD_LN_UNDEF,
    CMD_LN_INT32,
    CMD_LN_FLOAT32,
    CMD_LN_FLOAT64,
    CMD_LN_STRING,
    CMD_LN_STRING_LIST,
    CMD_LN_BOOLEAN
} arg_type_t;

typedef struct {
    char *switch_name;	/* name of the command line switch */
    arg_type_t type;	/* type of the argument */
    validation_fn_t validate_arg; /* a function which validates
				     argument values */
    void *default_value; /* the default value of the argument, if any */
    char *doc;		 /* a documentation string for the argument */
} arg_def_t;

/* Initializes the command line parsing subsystem */
void
cmd_ln_initialize(void);

void
cmd_ln_print_configuration(void);


/* Defines the set of acceptable command line arguments
 * given the list of command line argument defintions
 * contained in the switch_definition_array.  An element
 * of the array contains the following information:
 *
 *	. switch name (e.g. "-dictionary")
 *
 *	. type of the argument following the switch
 *
 *	. an optional validation function for the
 *	  value given on the command line.
 *
 *	. a translation function for the
 *	  value given on the command line.  This
 *	  function is optional except for
 *	  enumerated types.
 *
 * This function allocates memory, if necessary, for 
 * parsed command line arguments.
*/
int
cmd_ln_define(arg_def_t *defn);

/*
 * Directs the module to skip/not skip the printing
 * of the entire comment line to stderr.  Default
 * is not to skip.
 */
void
cmd_ln_skip_print(int state);

/*
 * Reads the command line and converts the arguments,
 * if necessary, according to their definitions.
 */
int
cmd_ln_parse(int argc, char *argv[]);

/*
 * Calls the validation functions for each of the
 * command line arguments.
 */
int
cmd_ln_validate(void);

/* Returns a pointer to the parsed command line argument.
   (The returned pointer must not be given to the
   function free())
   It is const because the caller should not modify the thing
   referred to by this pointer. */

const void *
cmd_ln_access(char *switch_name);

/* Prints out the command line argument definitions to stderr */

void
cmd_ln_print_definitions(void);

#endif /* S3CMD_LN_H */ 


/*
 * Log record.  Maintained by CVS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.6  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.5  1996/07/29  16:40:50  eht
 * added missing function prototype
 *
 * Revision 1.4  1996/03/25  15:48:45  eht
 * Added void prototype for functions w/ no arguments
 *
 * Revision 1.3  1996/01/26  18:29:54  eht
 * Define TRUE and FALSE correctly
 *
 * Revision 1.2  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
