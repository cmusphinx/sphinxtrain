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
 * File: s2_ddcep.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef S2_DDCEP_H
#define S2_DDCEP_H

#include <s3/vector.h>
#include <s3/prim_type.h>

void
s2_ddcep_set_veclen(uint32 veclen);

void
s2_sec_ord_dcep_feat(vector_t **out,
		     vector_t *mfcc,
		     uint32 n_frame);

#endif /* S2_DDCEP_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1996/03/25  15:36:54  eht
 * Allow for settable input feature length
 *
 * Revision 1.1  1996/01/30  17:09:59  eht
 * Initial revision
 *
 *
 */
