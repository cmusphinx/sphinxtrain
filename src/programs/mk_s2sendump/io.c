/*
 * io.c -- Packaged I/O routines.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 08-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added stat_mtime().
 * 
 * 11-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added _myfopen().
 * 
 * 05-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include "io.h"
#include "err.h"


FILE *fopen_comp (char *file, char *mode, int32 *ispipe)
{
    char command[16384];
    FILE *fp;
    int32 k, isgz;
    
    k = strlen (file);
    
#if (WIN32)
    *ispipe = (k > 3) &&
	((strcmp (file+k-3, ".gz") == 0) || (strcmp (file+k-3, ".GZ") == 0));
    isgz = *ispipe;
#else
    *ispipe = 0;
    isgz = 0;
    if ((k > 2) && ((strcmp (file+k-2, ".Z") == 0) || (strcmp (file+k-2, ".z") == 0))) {
	*ispipe = 1;
    } else {
	if ((k > 3) &&
	    ((strcmp (file+k-3, ".gz") == 0) || (strcmp (file+k-3, ".GZ") == 0))) {
	    *ispipe = 1;
	    isgz = 1;
	}
    }
#endif

    if (*ispipe) {
#if (WIN32)
	if (strcmp (mode, "r") == 0) {
	    sprintf (command, "gzip.exe -d -c %s", file);
	    if ((fp = _popen (command, mode)) == NULL) {
		E_ERROR_SYSTEM("_popen (%s,%s) failed\n", command, mode);
		return NULL;
	    }
	} else {
	    E_ERROR ("fopen_comp not implemented for mode = %s\n", mode);
	    return NULL;
	}
#else
	if (strcmp (mode, "r") == 0) {
	    if (isgz)
		sprintf (command, "gunzip -c %s", file);
	    else
		sprintf (command, "zcat %s", file);
	    
	    if ((fp = popen (command, mode)) == NULL) {
		E_ERROR_SYSTEM("popen (%s,%s) failed\n", command, mode);
		return NULL;
	    }
	} else if (strcmp (mode, "w") == 0) {
	    if (isgz)
		sprintf (command, "gzip > %s", file);
	    else
		sprintf (command, "compress -c > %s", file);
	    
	    if ((fp = popen (command, mode)) == NULL) {
		E_ERROR_SYSTEM("popen (%s,%s) failed\n", command, mode);
		return NULL;
	    }
	} else {
	    E_ERROR ("fopen_comp not implemented for mode = %s\n", mode);
	    return NULL;
	}
#endif
    } else {
        fp = fopen (file, mode);
    }

    return (fp);
}


void fclose_comp (FILE *fp, int32 ispipe)
{
    if (ispipe) {
#if (WIN32)
	_pclose (fp);
#else
	pclose (fp);
#endif
    } else
	fclose (fp);
}


FILE *fopen_compchk (char *file, int32 *ispipe)
{
    char tmpfile[16384];
    int32 k, isgz;
    struct stat statbuf;
    
    k = strlen (file);
    
#if (WIN32)
    *ispipe = (k > 3) &&
	((strcmp (file+k-3, ".gz") == 0) || (strcmp (file+k-3, ".GZ") == 0));
    isgz = *ispipe;
#else
    *ispipe = 0;
    isgz = 0;
    if ((k > 2) && ((strcmp (file+k-2, ".Z") == 0) || (strcmp (file+k-2, ".z") == 0))) {
	*ispipe = 1;
    } else {
	if ((k > 3) &&
	    ((strcmp (file+k-3, ".gz") == 0) || (strcmp (file+k-3, ".GZ") == 0))) {
	    *ispipe = 1;
	    isgz = 1;
	}
    }
#endif
    
    strcpy (tmpfile, file);
    if (stat (tmpfile, &statbuf) != 0) {
	/* File doesn't exist; try other compressed/uncompressed form, as appropriate */
	E_ERROR_SYSTEM("stat(%s) failed\n", tmpfile);
	
	if (*ispipe) {
	    if (isgz)
		tmpfile[k-3] = '\0';
	    else
		tmpfile[k-2] = '\0';
	    
	    if (stat (tmpfile, &statbuf) != 0)
		return NULL;
	} else {
	    strcpy (tmpfile+k, ".gz");
	    if (stat (tmpfile, &statbuf) != 0) {
#if (! WIN32)
		strcpy (tmpfile+k, ".Z");
		if (stat (tmpfile, &statbuf) != 0)
		    return NULL;
#else
		return NULL;
#endif
	    }
	}
	
	E_WARN("Using %s instead of %s\n", tmpfile, file);
    }
    
    return (fopen_comp (tmpfile, "r", ispipe));
}


#define FREAD_RETRY_COUNT	60

int32 fread_retry(void *pointer, int32 size, int32 num_items, FILE *stream)
{
    char *data;
    uint32 n_items_read;
    uint32 n_items_rem;
    uint32 n_retry_rem;
    int32 loc;

    n_retry_rem = FREAD_RETRY_COUNT;

    data = pointer;
    loc = 0;
    n_items_rem = num_items;
	
    do {
	n_items_read = fread(&data[loc], size, n_items_rem, stream);

	n_items_rem -= n_items_read;

	if (n_items_rem > 0) {
	    /* an incomplete read occurred */

	    if (n_retry_rem == 0)
		return -1;

	    if (n_retry_rem == FREAD_RETRY_COUNT) {
		E_ERROR_SYSTEM("fread() failed; retrying...\n");
	    }

	    --n_retry_rem;
	    
	    loc += n_items_read * size;
#if (! WIN32)
	    sleep(1);
#endif
	}
    } while (n_items_rem > 0);

    return num_items;
}


#define STAT_RETRY_COUNT	10

int32 stat_retry (char *file, struct stat *statbuf)
{
    int32 i;
    
    for (i = 0; i < STAT_RETRY_COUNT; i++) {
	if (stat (file, statbuf) == 0)
	    return 0;
	
	if (i == 0) {
	    E_ERROR_SYSTEM("stat(%s) failed; retrying...\n", file);
	}
#if (! WIN32)
	sleep (10);
#endif
    }
    
    return -1;
}


int32 stat_mtime (char *file)
{
    struct stat statbuf;
    
    if (stat (file, &statbuf) != 0)
	return -1;
    
    return ((int32)statbuf.st_mtime);
}


FILE *_myfopen (char *file, char *mode, char *pgm, int32 line)
{
    FILE *fp;
    
    if ((fp = fopen(file, mode)) == NULL) {
	fflush (stdout);
	fprintf (stderr, "FATAL_ERROR: \"%s\", line %ld: fopen(%s,%s) failed; ",
		 pgm, line, file, mode);
	perror("");
	fflush (stderr);
	
	exit(errno);
    }
    
    return fp;
}
