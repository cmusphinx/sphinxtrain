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
 * File: s2_write_cb.h
 * 
 * Description: 
 * 
 * Author:
 *	Eric H. Thayer (eht@cs.cmu.edu)
 *
 *********************************************************************/

#ifndef S2_WRITE_CB_H
#define S2_WRITE_CB_H

#include <s3/gauden.h>

int
s2_write_cb(gauden_t *g,
	    const char *out_dir_name,
	    const char **cb_base,
	    const char *mean_ext,
	    const char *var_ext,
	    int do_floor);

#endif /* S2_WRITE_CB_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/10  13:10:34  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.1  1995/09/08  19:13:52  eht
 * Initial revision
 *
 *
 */
