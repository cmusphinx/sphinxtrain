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
 * File: ts_cnt.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef TS_CNT_H
#define TS_CNT_H

#include <s3/lexicon.h>
#include <s3/model_def_io.h>
#include <s3/acmod_set.h>

int
ts_cnt(uint32 *cnt,		/* observation counts */

       model_def_t *mdef,	/* model definitions */

       uint16 *seg,		/* Viterbi (CI phone,state) pairs for all frames */
       uint32 n_frame,

       acmod_id_t *phone,	/* CI phone sequence (already validated) */
       char *btw_mark,
       uint32 n_phone);

#endif /* TS_CNT_H */ 


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
 * Revision 1.1  1996/03/25  15:22:04  eht
 * Initial revision
 *
 *
 */
