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
 * File: fread_retry.c
 * 
 * Description: 
 * 
 * Author: 
 *	Eric H. Thayer
 * 
 *********************************************************************/

#include <s3/fread_retry.h>
#include <sys_compat/misc.h>	/* defines sleep() for WIN32 */
#include <s3/err.h>

#include <sys_compat/misc.h>

#define FREAD_RETRY_COUNT	60

size_t fread_retry(void *pointer,
		   size_t size,
		   size_t num_items,
		   FILE *stream)
{
    char *data;
    unsigned int n_items_read;
    unsigned int n_items_rem;
    size_t loc;
    unsigned int n_retry_rem;

    n_retry_rem = FREAD_RETRY_COUNT;

    data = pointer;
    loc = 0;
    n_items_rem = num_items;
	
    do {
	n_items_read = fread(&data[loc], size, n_items_rem, stream);

	n_items_rem -= n_items_read;

	if (n_items_rem > 0) {
	    /* an incomplete read occurred */

	    if (n_retry_rem == 0) {
		E_FATAL_SYSTEM("Unable to fread() data after %u retries\n",
			       FREAD_RETRY_COUNT);
	    }
	    else if (n_retry_rem == FREAD_RETRY_COUNT) {
		E_WARN_SYSTEM("Unable to fread() data completely the first time; Retrying...\n");
	    }

	    --n_retry_rem;
	    
	    loc += n_items_read * size;

	    sleep(1);
	}
    } while (n_items_rem > 0);

    return num_items;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/03/17  15:01:49  eht
 * Initial revision
 * 
 *
 */
