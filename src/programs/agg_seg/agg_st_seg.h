/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: agg_st_seg.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef AGG_ST_SEG_H
#define AGG_ST_SEG_H

#include <s3/model_def.h>
#include <s3/lexicon.h>
#include <s3/segdmp.h>

int
agg_st_seg(model_def_t *mdef,
	   lexicon_t *lex,
	   uint32 *ts2cb,
	   uint32 *cb2mllr,
	   segdmp_type_t type);

#endif /* AGG_ST_SEG_H */ 

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
 *
 */
