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
 * File: accum_wt_param.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef ACCUM_WT_PARAM_H
#define ACCUM_WT_PARAM_H

#include <s3/vector.h>
#include <s3/prim_type.h>

int
accum_wt_param(char **accum_dir,
	       vector_t ****mean,
	       vector_t ****var,
	       int32 *pass2var,
	       float32 ****dnom,
	       uint32 *n_mgau,
	       uint32 *n_feat,
	       uint32 *n_density);

#endif /* ACCUM_WT_PARAM_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/03/26  15:17:51  eht
 * Initial revision
 *
 *
 */
