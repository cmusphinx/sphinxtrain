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
 * File: del_sil_seg.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef DEL_SIL_SEG_H
#define DEL_SIL_SEG_H

#include <s3/prim_type.h>
#include <s3/vector.h>

uint32
del_sil_seg(vector_t *mfcc,
	    uint32 n_frame,
	    uint32 *del_b,
	    uint32 *del_e,
	    uint32 n_del);

#endif /* DEL_SIL_SEG_H */ 


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
