/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: agc_max.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	faa
 *********************************************************************/

#ifndef AGC_MAX_H
#define AGC_MAX_H

#include <s3/prim_type.h>

void
agc_max (float32 *cep,		/* The cepstrum data */
	 uint32 fcnt,	/* Number of cepstrum frame availiable */
	 uint32 cf_cnt);/* Number of coeff's per frame */


#endif /* AGC_MAX_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1996/01/30  17:09:59  eht
 * Initial revision
 *
 * Revision 1.2  1995/10/10  12:35:52  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:45:04  eht
 * Initial revision
 *
 *
 */
