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
 * File: mk_phone_list.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef MK_PHONE_LIST_H
#define MK_PHONE_LIST_H

#include <s3/acmod_set.h>
#include <s3/lexicon.h>

void
print_phone_list(acmod_id_t *p,
		 uint32 n_p,
		 char *btw,
		 acmod_set_t *phone_set);

acmod_id_t *
mk_phone_list(char **btw_mark,
	      uint32 *n_phone,
	      char **word,
	      uint32 n_word,
	      lexicon_t *lex);

#endif /* MK_PHONE_LIST_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/12/01  20:49:12  eht
 * Coding standard adherance changes
 *
 * Revision 1.1  1995/12/01  20:43:21  eht
 * Initial revision
 *
 * Revision 1.4  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.3  1995/10/10  12:44:06  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/09/08  19:13:11  eht
 * Updated to use the new acmod_set module.  Prior to
 * testing on TI digits
 *
 * Revision 1.1  1995/06/02  20:43:56  eht
 * Initial revision
 *
 *
 */
