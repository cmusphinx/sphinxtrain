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
 * File: timer.h
 * 
 * Description: 
 *	Simple elapsed time functions
 *
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <s3/prim_type.h>

int
timer_start(uint32 id);

float32
timer_stop(uint32 id);

#endif /* TIMER_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1995/10/09  20:55:35  eht
 * Initial revision
 *
 *
 */
