/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * prefetch.c -- a simple interface to set up prefetching of files that
 * a program will be opening soon.
 *
 * By Paul W. Placeway, CMU Speech Group, May 23, 1995.
 * (Thanks to Brian Noble suggesting the open, read one char, close idea.) 
 */

/* #ifndef WIN32 */
#if 0
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>

#include <sys_compat/file.h>
#endif

/*****************************************************************
 * The general idea here is to spawn off a child process on start-up,
 * with a pipe between the master and the child.
 *
 * Whenever the master wants to prefetch a file, it writes the name
 * of the file, terminated by a newline, to the child.
 *
 * The child reads the names of files, and for each one, tries
 * to open the file, read the first byte, and close the file.
 * This will have the effect of putting the file info block, and
 * the first block of data, into the file buffer cache of the kernel.
 *
 * The pipe is set to be non-blocking on the master side, since
 * it would be silly for the master to block waiting for the child,
 * when what we are really trying to do is speed things up.
 ***************************************************************** */

/* #define TESTING */

/* #ifndef WIN32 */
#if 0

static int _fd_to_prefetch = -1;
static pid_t _pid_of_prefetch = -1;

int
prefetch_init()
{

    pid_t pid;
    int p_fd[2];
    FILE *fp_from_prefetch;

    return 0;

    if (pipe(p_fd) < 0) {
	perror("WARNING: pipe()");
	fflush(stderr);
	return -1;
    }

#ifdef TESTING
    printf("MASTER pipe fds == %d, %d\n", p_fd[0], p_fd[1]);
    fflush(stdout);
#endif /* TESTING */

    pid = fork();
    if (pid < 0) {
	perror("WARNING: fork()");
	fflush(stderr);
	return -1;
    }

#ifdef TESTING
    printf("MASTER pid == %d\n", pid);
    fflush(stdout);
#endif /* TESTING */

    if (pid == 0) {		/* child */
	char buf[MAXPATHLEN+1];
	char *cp;
	char dummy;
	int fd;

	close (p_fd[1]);	/* close the writing side of the pipe */

	if ((fp_from_prefetch = fdopen(p_fd[0], "r")) == NULL) {
	    perror("WARNING: fdopen()");
	    fflush(stderr);
	    exit (7);
	}

	for (;;) {
	    if (fgets(buf, MAXPATHLEN, fp_from_prefetch) == NULL) {
#ifdef TESTING
		perror("CHILD: fgets()");
		fflush(stderr);
		printf("CHILD read failed\n");
		fflush(stdout);
#endif /* TESTING */

		exit (0);	/* file error: master probably died */
	    }

				/* remove the trailing newline. */
	    if ((cp = strrchr (buf, '\n')) != NULL)
		*cp = '\0';

#ifdef TESTING
	    printf("CHILD read \"%s\"\n", buf);
	    fflush(stdout);
#endif /* TESTING */

	    if ((fd = open(buf, O_RDONLY, 0)) < 0) {
#ifdef TESTING
		perror("CHILD open()");
		fflush(stderr);
		printf("CHILD could not open \"%s\"; continuing\n", buf);
		fflush(stdout);
#endif /* TESTING */
		continue;
	    }

#ifdef TESTING
	    printf("CHILD opened \"%s\" as fd %d\n", buf, fd);
	    fflush(stdout);
#endif /* TESTING */

	    if (read(fd, &dummy, 1) != 1) {
#ifdef TESTING
		perror("CHILD read()");
		fflush(stderr);
		printf("CHILD could not read from \"%s\"\n", buf);
		fflush(stdout);
#endif /* TESTING */
	    }

	    close(fd);

	}  /* end for(;;) */

    } else {			/* parent */

	close (p_fd[0]);	/* close the reading side of the pipe */

	_fd_to_prefetch = p_fd[1];
	_pid_of_prefetch = pid;

#ifndef _HPUX_SOURCE
	if (fcntl(_fd_to_prefetch, F_SETFL, FNDELAY)) {
	    perror("WARNING: fcntl()");
	    fflush(stderr);
	    return -1;
	}
#else
	if (fcntl(_fd_to_prefetch, F_SETFL, O_NDELAY)) {
	    perror("WARNING: fcntl()");
	    fflush(stderr);
	    return -1;
	}
#endif


	return 0;
    }
}


int
prefetch_hint(fn)
char *fn;
{
    char buf[MAXPATHLEN+1];
    char *p;
    int i, l;
    extern int errno;

    return 0;

    if (fn == NULL)
	return -1;

    if (_fd_to_prefetch < 0)
	return -2;

#ifdef TESTING
    printf("MASTER writing \"%s\"\n", fn);
    fflush(stdout);
#endif /* TESTING */

    strncpy(buf, fn, MAXPATHLEN);
    buf[MAXPATHLEN] = '\0';
    l = strlen(buf);
    buf[l++] = '\n';
    buf[l] = '\0';

    p = buf;
    i = write(_fd_to_prefetch, p, l);
    if (i < 0) {
	if (errno == EWOULDBLOCK)
	    return 1;
	else
	    return -1;
    }

    for (l -= i, p += i; l > 0; l -= i, p += i) {
	i = write(_fd_to_prefetch, p, l);
	if (i < 0) {
	    if (errno == EWOULDBLOCK) {
		i = 0;
	    } else {
		return -1;
	    }
	}
    }

#ifdef TESTING
    printf("MASTER done.\n");
    fflush(stdout);
#endif /* TESTING */

    return 0;			/* success */
}

int
prefetch_finish()
{
    int status;

    if (_fd_to_prefetch < 0)
	return -2;

#ifdef TESTING
    printf("MASTER closing fd %d.\n", _fd_to_prefetch);
    fflush(stdout);
#endif /* TESTING */

    close(_fd_to_prefetch);	/* may cause child to exit */
    _fd_to_prefetch = -1;

#ifdef TESTING
    printf("MASTER waiting for pid %d.\n", _pid_of_prefetch);
    fflush(stdout);
#endif /* TESTING */

    (void) kill (_pid_of_prefetch, SIGINT); /* kill the child just in case */

				/* reap the child process */
    while (waitpid(_pid_of_prefetch, &status, 0) < 0) {
	if (errno != EINTR) {
	    perror("WARNING: waitpid()");
	    fflush(stderr);
	    return -1;
	}
#ifdef TESTING
	printf("MASTER still waiting for pid %d.\n", _pid_of_prefetch);
    fflush(stdout);
#endif /* TESTING */
    }
    _pid_of_prefetch = -1;

#ifdef TESTING
    printf("MASTER finished.\n");
    fflush(stdout);
#endif /* TESTING */

    return 0;
}

#else

/* WIN32 */
/* Just define some stubs for now */

int
prefetch_init()
{
    return 0;
}

int
prefetch_hint(char *fn)
{
    return 0;
}

int
prefetch_finish()
{
    return 0;
}

#endif


#ifdef TESTING

main(argc, argv)
int argc;
char *argv[];
{
    prefetch_init();

    argv++;			/* skip program name */

    while (*argv) {
	prefetch_hint(*argv++);
    }

    sleep (10);

    prefetch_finish();

    exit (0);
}

#endif /* TESTING */
