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
 * File: mixw_param_io.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef MIXW_PARAM_IO_H
#define MIXW_PARAM_IO_H

#define MIXW_PARAM_VERSION "0.1"

#include <s3/prim_type.h>
int32
mixw_param_bundle(const char *dir,
		  float32 ****out_mixw,
		  unsigned int n_mixw,
		  unsigned int *out_n_feat,
		  unsigned int *out_n_cw);

int32
mixw_param_unbundle(const char *dir,
		    float32 ***mixw,
		    unsigned int n_mixw,
		    unsigned int n_feat,
		    unsigned int n_cw);

int32
mixw_param_write(const char *file_name,
		 const char *comment,
		 float32 ***mixw,
		 uint32 n_mixw,
		 uint32 n_feat,
		 uint32 n_cw);

int32
mixw_param_read(const char **out_comment,
		float32 ****out_mixw,
		uint32 *out_n_mixw,
		uint32 *out_n_feat,
		uint32 *out_n_cw,
		const char *file_name);

#endif /* MIXW_PARAM_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.3  1996/07/29  16:33:40  eht
 * added bundle/unbundle functions
 *
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
