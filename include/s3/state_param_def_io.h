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
 * File: state_param_def_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef STATE_PARAM_DEF_IO_H
#define STATE_PARAM_DEF_IO_H

#define STATE_PARAM_DEF_VERSION	"0.1"

#include <s3/prim_type.h>
#include <s3/model_def_io.h>

int
state_param_def_read(model_def_t *mdef,
		     const char **comment,
		     const char *file_name);

int
state_param_def_write(const char *file_name,
		      uint32 *den_id,
		      uint32 n_den_id,
		      char *comment);

int
ck_state_param_def(uint32 *den_map,
		   uint32 n_map,
		   uint32 n_mgau);

#endif /* STATE_PARAM_DEF_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1996/07/29  16:33:40  eht
 * added error checking routine
 *
 * Revision 1.1  1996/01/26  17:59:47  eht
 * Initial revision
 *
 *
 */
