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
 * File: ts2cb.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef TS2CB_H
#define TS2CB_H

#include <s3/prim_type.h>

#define CONT_LABEL	".cont."
#define SEMI_LABEL	".semi."

uint32 *
semi_ts2cb(uint32 n_ts);

uint32 *
cont_ts2cb(uint32 n_ts);

#endif /* TS2CB_H */ 


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
