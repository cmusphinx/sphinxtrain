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
 * File: read_seno_dtree.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef READ_SENO_DTREE_H
#define READ_SENO_DTREE_H

#include <s3/prim_type.h>
#include <s3/dtree.h>
#include <stdio.h>


#define NO_MAP (0xffffffff)

/* Public Fns */

int
read_seno_dtree_file(dtree_t **dt,
		     const char *file_name);

/* Private Fns */

static int
read_header_line(uint32 *n_base,
		 char *id,
		 uint32 *n_cd,
		 FILE *fp);

static int
read_node_line(float32 *out_entropy,
	       uint32 *out_key,
	       uint32 *out_left_key,
	       uint32 *out_right_key,
	       char *quest_str,
	       FILE *fp);

#endif /* READ_SENO_DTREE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.1  96/03/25  15:54:29  eht
 * Initial revision
 * 
 *
 */
