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
 * File: cmn.h
 * 
 * Description: 
 * 	Do various types of cepstral mean normalization.
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef CMN_H
#define CMN_H

#include <s3/prim_type.h>

void
cmn_set_veclen(uint32 l);

void
cmn(float32 *mfcc,
    uint32 n_frame);

#endif /* CMN_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1996/08/05  12:42:38  eht
 * fixed typo in prototype
 *
 * Revision 1.2  1996/03/25  15:46:57  eht
 * Added ability to set input feature vector size
 *
 * Revision 1.1  1996/01/26  18:33:28  eht
 * Initial revision
 *
 *
 */
