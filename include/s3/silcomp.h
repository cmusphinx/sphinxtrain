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
 * File: silcomp.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef SILCOMP_H
#define SILCOMP_H

#include <s3/prim_type.h>
#include <s3/vector.h>

void
silcomp_set_del_seg(uint32 *del_b,
		    uint32 *del_e,
		    uint32 n_del);
void
silcomp_set_veclen(uint32 veclen);

uint32
sil_compression(const char *type,
		vector_t *mfcc,
		uint32 in_n_frame);

#endif /* SILCOMP_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.2  1996/03/25  15:46:57  eht
 * Added ability to change input feature vector size
 *
 * Revision 1.1  1996/01/26  18:33:28  eht
 * Initial revision
 *
 *
 */
