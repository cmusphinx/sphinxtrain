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
 * File: file.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef FILE_H
#define FILE_H


#if defined(WIN32)

#if !defined(CYGNUS)

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXPATHLEN FILENAME_MAX

#else

/* CYGNUS && WIN32 */
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#else

/* !WIN32 */
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>

#endif

#if !defined(WIN32)
#define O_BINARY 0	/* WIN32 i/o does text conversion by default */
#endif

#endif /* FILE_H */

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
