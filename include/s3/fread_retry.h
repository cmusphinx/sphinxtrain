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
 * file: fread_retry.h
 * 
 * traceability: 
 * 
 * description: 
 * 
 * author: 
 * 
 * reviews: <To be filled in by SQA.>
 * 
 *********************************************************************/

#ifndef FREAD_RETRY_H
#define FREAD_RETRY_H

#include <stdio.h>
#include <sys/types.h>

size_t fread_retry(void *pointer,
		   size_t size,
		   size_t num_items,
		   FILE *stream);

#endif /* FREAD_RETRY_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/24  18:49:34  eht
 * Get size_t type
 *
 * Revision 1.1  1995/09/08  19:13:52  eht
 * Initial revision
 *
 *
 */
