/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: mk_phone_seq.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MK_PHONE_SEQ_H
#define MK_PHONE_SEQ_H

#include <s3/acmod_set.h>
#include <s3/prim_type.h>
#include <s3/lexicon.h>

int
mk_phone_seq(acmod_id_t **out_phone,
	     uint32 *out_n_phone,
	     char *trans,
	     acmod_set_t *acmod_set,
	     lexicon_t *lex);

#endif /* MK_PHONE_SEQ_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:39:10  eht
 * Initial revision
 * 
 *
 */
