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
 * File: get_time.c
 * 
 * Description: 
 *	A machine independent way to get a time for the purposes of
 *	computing elapsed time.
 *
 *	Currently, only known to work on OSF/1 and HPUX.
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#include <s3/get_time.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <sys_compat/time.h>

#include <stddef.h>	/* define NULL, etc. */

/*********************************************************************
 *
 * Function: get_time()
 * 
 * Description: 
 *	Return some value (in units of seconds) which increases
 *	at a constant rate with	time.  The purpose of this routine
 *	is to provide a way to compute elapsed times for events.
 *	Since the method for getting the current system time has
 *	varied quite a lot over the years, it is wrapped up in
 *	this routine.
 * 
 * Function Inputs: 
 *	None
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	The current value of elapsed time
 * 
 * Global Outputs: 
 *	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 *********************************************************************/

int
get_time(uint32 *sec, float32 *frac_sec)
{
#ifndef WIN32
    struct timeval t;

    /* Use the BSD call */
    if (gettimeofday(&t, NULL) < 0) {
	E_WARN_SYSTEM("Unable to get time\n");

	return S3_ERROR;
    }

    /* On the Alpha, the header file sys/timers.h says
     * that tv_nsec is in terms of microseconds, but the
     * man page on clock_gettime says that the return value
     * is in terms of nanoseconds.  The name of the field
     * suggests to me that it is nanoseconds, so for now... */

    *sec = t.tv_sec;
    *frac_sec = t.tv_usec * 1e-6;

    return S3_SUCCESS;
#else

    *sec = 0;
    *frac_sec = 0;

    return S3_SUCCESS;
#endif    
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.6  1996/06/17  14:36:41  eht
 * Get get_time to compile under WIN32
 *
 * Revision 1.5  1995/10/24  18:48:06  eht
 * Use gettimeofday() instead of POSIX call since BSD
 * has had more time to propagate than POSIX
 *
 * Revision 1.4  1995/10/13  20:59:21  eht
 * Start to get ready for Windows NT
 *
 * Revision 1.3  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.2  1995/10/05  12:59:17  eht
 * Some necessary changes to get the beggar to work.
 *
 * Revision 1.1  1995/09/15  15:24:06  eht
 * Initial revision
 *
 *
 */
