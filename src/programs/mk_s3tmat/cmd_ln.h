/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: cmd_ln.h
 * 
 * Description: 
 * 	Command line processing call
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef PARSE_CMDLN_H
#define PARSE_CMDLN_H

#include <s3/prim_type.h>

int32
parse_cmd_ln(int argc,
	     char *argv[]);

#endif /* PARSE_CMDLN_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:38:16  eht
 * Initial revision
 * 
 * Revision 1.2  95/10/10  13:10:11  eht
 * Changed to use <s3/prim_type.h>
 * 
 * Revision 1.1  1995/06/02  20:31:53  eht
 * Initial revision
 *
 *
 */
