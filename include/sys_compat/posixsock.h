/*
 * posixsock.h
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
     Brian G. Milnes
     Speech Group
     Carnegie Mellon University 
     5-Mar-95

     An include file to hide posix socket differences.

 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * 
 * 02-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added errno handling.
 * 
 */

#ifndef _POSIXSOCK_H_
#define _POSIXSOCK_H_

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

/*
 * Windows NT uses a different socket interface and return values from BSD.
 * For compatibility and portability, use the following even with BSD.
 */

#define SOCKET			int32		/* Returned by socket/accept */
#define INVALID_SOCKET		-1		/* Returned by socket/accept */
#define SOCKET_ERROR		-1		/* Returned by most socket operations */
#define SOCKET_ERRNO		errno
#define SOCKET_WOULDBLOCK	EWOULDBLOCK
#define SOCKET_IOCTL		ioctl
#define closesocket		close		/* To close a SOCKET */

#define BOOL int32

#endif
