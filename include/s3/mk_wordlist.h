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
 * File: mk_wordlist.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef MK_WORDLIST_H
#define MK_WORDLIST_H

#include <s3/prim_type.h>

char **mk_wordlist(char *str,
		   uint32 *n_word);

#endif /* MK_WORDLIST_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.2  1995/08/15  14:22:28  eht
 * Changed the name of the fn
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
