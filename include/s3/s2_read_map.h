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
 * file: s2_read_map.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef S2_READ_MAP_H
#define S2_READ_MAP_H

#include <s3/acmod_set.h>
#include <s3/prim_type.h>

int
s2_read_seno_mapping_file(uint32 **smap,
			  uint32 *cluster_size,
			  const char *seno_mapping_file,
			  acmod_set_t *phone_set);

int
s2_convert_smap_to_global(acmod_set_t *phone_set,
			  uint32 **smap,
			  uint32 **state_of,
			  uint32 *cluster_size);

#endif /* S2_READ_MAP_H */ 


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
 * Revision 1.3  95/10/10  13:10:34  eht
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
