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
 * File: s2_read_seno.h
 * 
 * Description: 
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef S2_READ_SENO_H
#define S2_READ_SENO_H

#include <s3/model_inventory.h>
#include <s3/prim_type.h>

float32 ***
s2_read_seno(model_inventory_t *mi,
	     const char *seno_dir,
	     const char *init_seno_map_filename,
	     float32 weight_floor,
	     uint32 *state_of);

float32 ***
s2_read_seno_2(acmod_set_t *phone_set,
	       uint32 *cluster_offset,
	       const char *seno_dir,
	       const char *init_seno_map_filename,
	       float32 weight_floor,
	       uint32 *state_of);

float32 ***
s2_read_seno_3(acmod_set_t *phone_set,
	       uint32 *cluster_offset,
	       const char *seno_dir,
	       uint32 **in_smap,
	       float32 weight_floor,
	       uint32 *state_of);


#endif /* S2_READ_SENO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
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
