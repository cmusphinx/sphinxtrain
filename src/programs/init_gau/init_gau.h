/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: init_gau.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef INIT_GAU_H
#define INIT_GAU_H

#include <s3/lexicon.h>
#include <s3/model_def_io.h>
#include <s3/acmod_set.h>

int
init_gau(lexicon_t *lex,
	 model_def_t *mdef);

#endif /* INIT_GAU_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.2  1996/01/30  17:11:47  eht
 * *** empty log message ***
 *
 * Revision 1.1  1995/12/01  20:56:09  eht
 * Initial revision
 *
 *
 */
