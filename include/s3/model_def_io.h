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
 * File: model_def_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef MODEL_DEF_IO_H
#define MODEL_DEF_IO_H

#include <s3/acmod_set.h>
#include <s3/prim_type.h>

#include <s3/model_def.h>

#include <s3/s3.h>


#define MODEL_DEF_VERSION "0.3"

int32
model_def_read(model_def_t **out_mdef,
	       const char *file_name);

int32
model_def_write(model_def_t *mdef,
		const char *file_name);

#endif /* MODEL_DEF_IO_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.7  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.6  96/07/29  16:33:40  eht
 * added bundle/unbundle functions
 * 
 * Revision 1.5  1996/03/25  15:52:26  eht
 * Commented
 *
 * Revision 1.4  1996/01/26  17:55:01  eht
 * Include mapping from tied states to mixture Gaussian densities in the model_def_t
 * structure
 *
 * Revision 1.3  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/09/08  19:13:52  eht
 * Updated to replace pset module with acmod_set module
 *
 * Revision 1.1  1995/08/15  13:46:15  eht
 * Initial revision
 *
 *
 */
