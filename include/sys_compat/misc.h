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
 * File: misc.h
 * 
 * Description: 
 * 	Miscellaneous Unix compatibility definitions
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef MISC_H
#define MISC_H

/* Nothing so far for Unix type machines */

#ifdef WIN32

#ifndef CYGNUS
#include <windows.h>

#define sleep(x) Sleep((x)*1000)

double
drand48(void);

#endif

#else
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
#include <netdb.h>	/* Solaris man page say's MAX... defined in <sys/param.h>
			   but it is NOT THERE! */
#endif

#endif /* WIN32 */

#endif /* MISC_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:40:04  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1995/10/30  16:14:37  eht
 * Initial revision
 *
 *
 */
