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
 * File: param_cnt.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef PARAM_CNT_H
#define PARAM_CNT_H

#include <s3/lexicon.h>
#include <s3/model_def_io.h>
#include <s3/acmod_set.h>

int
param_cnt(lexicon_t *lex,
	  model_def_t *mdef,
	  const char *param_type);

#endif /* PARAM_CNT_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:32  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:38:16  eht
 * Initial revision
 * 
 *
 */
