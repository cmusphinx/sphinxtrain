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
 * File: mk_wordlist.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/mk_wordlist.h>
#include <s3/ckd_alloc.h>
#include <s3/n_words.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

/* BEWARE: this routine overwrites str */
char **mk_wordlist(char *str,
		   uint32 *n_word)
{
    uint32 n_w;
    uint32 i;
    char **wl;

    n_w = n_words(str);

    wl = ckd_calloc(n_w, sizeof(char *));

    wl[0] = strtok(str, " \t");
    for (i = 1; i < n_w; i++) {
	wl[i] = strtok(NULL, " \t");
    }

    assert(strtok(NULL, " \t") == NULL);

    *n_word = n_w;

    return wl;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.5  1995/10/23  20:32:21  eht
 * Add missing include of <stdio.h>
 *
 * Revision 1.4  1995/10/10  17:41:00  eht
 * Put standard comment in
 * Include <s3/n_words.h>
 *
 *
 */
