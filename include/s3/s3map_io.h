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
 * File: s3map_io.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef S3MAP_IO_H
#define S3MAP_IO_H

#include <s3/prim_type.h>

#include <stdlib.h>

#define MAP_FILE_VERSION	"1.2"

int
s3map_read(const char *fn,
	   void **out_map,
	   uint32 *out_n_dom,
	   uint32 *out_n_rng,
	   size_t map_elem_size);

int
s3map_write(const char *fn,
	    void *map,
	    uint32 n_map,
	    uint32 n_rng,
	    size_t map_elem_size);

#endif /* S3MAP_IO_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:39:10  eht
 * Initial revision
 * 
 *
 */
