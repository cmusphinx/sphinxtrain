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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
/*********************************************************************
 *
 * File: lexicon.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/lexicon.h>
#include <s3/ckd_alloc.h>
#include <s3/n_words.h>
#include <s3/read_line.h>

#include <s3/s3.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static int
add_word(char *ortho,
	 uint32 wid,
	 lexicon_t *l,
	 lex_entry_t *e)
{
    e->ortho = ortho;
    e->word_id = wid;

    if (hash_enter(l->ht,
		   e->ortho,
		   (void *)e) != 0) {
	E_FATAL("hash add failed\n");
    }
    
    return S3_SUCCESS;
}

static int
add_phones(uint32 n_phone,
	   lex_entry_t *e,
	   acmod_set_t *acmod_set)
{
    uint32 i;
    char *nxt_phone;

    e->phone = ckd_calloc(n_phone, sizeof(char *));

    e->ci_acmod_id = ckd_calloc(n_phone, sizeof(uint32));

    e->phone_cnt = n_phone;

    for (i = 0; (nxt_phone = strtok(NULL, " \t")); i++) {
	e->phone[i] = nxt_phone;
	e->ci_acmod_id[i] =
	    acmod_set_name2id(acmod_set, nxt_phone);

	if (e->ci_acmod_id[i] == NO_ACMOD) {
	    E_ERROR("Unknown phone %s\n", nxt_phone);
	    ckd_free(e->phone);
	    e->phone = NULL;

	    ckd_free(e->ci_acmod_id);
	    e->ci_acmod_id = NULL;

	    e->phone_cnt = 0;

	    return S3_ERROR;
	}
    }
    
    assert(i == n_phone);

    return S3_SUCCESS;
}

lexicon_t *lexicon_new()
{
    lexicon_t *new;
    
    new = ckd_calloc(1, sizeof(lexicon_t));

    new->head = new->tail = NULL;

    new->entry_cnt = 0;

    new->ht = hash_new("lex", 64000);

    return new;
}

lex_entry_t *lexicon_append_entry(lexicon_t *lex)
{
    lex_entry_t *new;

    new = ckd_calloc(1, sizeof(lex_entry_t));

    if (lex->head == NULL) {
	lex->head = lex->tail = new;
    }
    else {
	lex->tail->next = new;

	lex->tail = new;
    }

    lex->entry_cnt++;

    return new;
}

lexicon_t *lexicon_read(lexicon_t *prior_lex,
			const char *filename,
			acmod_set_t *acmod_set)
{
    FILE *lex_fp;
    char line[1024];
    char *lex_line;
    lexicon_t *lex;
    lex_entry_t *next_entry = NULL;
    uint32    wid, start_wid;
    uint32    n_phone;
    uint32    lineno = 0;
    int       reuse_entry = FALSE;
    char      *word;

    lex_fp = fopen(filename, "r");
    if (lex_fp == NULL) {
	E_FATAL_SYSTEM("Unable to open lexicon %s for reading\n",
		       filename);
    }

    if (prior_lex)
	lex = prior_lex;
    else
	lex = lexicon_new();
    
    for (start_wid = wid = lex->entry_cnt;
	 read_line(line, 1023, &lineno, lex_fp) != NULL;
	 /* wid incremented in body of loop */ ) {
	if (line[0] == '\n') {
	    E_WARN("Lexicon %s has a blank line at line %d\n",
		   filename, lineno);
	    continue;
	}

	if (!reuse_entry) {
	    next_entry = lexicon_append_entry(lex);
	}
	else {
	    reuse_entry = FALSE; /* reset to standard case */
	}

	/* allocate space for string.  It will be parsed
	 * by strtok() */
	lex_line = strdup(line);

	/* get the word and make a hash table entry for
	   this lexicon entry */

	word = strtok(lex_line, " \t");	

	if (add_word(word,
		     wid,
		     lex,
		     next_entry) != S3_SUCCESS) {
	    E_ERROR("%s duplicate entry?\n", word);

	    reuse_entry = TRUE;	/* Since this line is skipped, reuse
				 * the lex entry for the next line */
	    continue;
	}

	/* n_words() counts the # of space separated "words" on a line */
	n_phone = n_words(line)-1;

#ifdef LEXICON_VERBOSE
	E_INFO("%s %d phones\n", line, n_phone);
#endif

	/* read the phones, convert to ids and add them
	   to the phone list for this entry */
	if (add_phones(n_phone, next_entry, acmod_set) != S3_SUCCESS) {
	    E_ERROR("pronunciation for %s has undefined phones; skipping.\n", word);
	    
	    reuse_entry = TRUE;

	    continue;
	}

	++wid;	/* only happens should everything be successful */
    }

    E_INFO("%d entries added from %s\n",
	   wid - start_wid, filename);
			  
    fclose(lex_fp);
    
    return lex;
}

lex_entry_t *lexicon_lookup(lexicon_t *lex, char *ortho)
{
    lex_entry_t *cur;
    
    if (hash_lookup(lex->ht, ortho, (void **)&cur) == 0) {
	return cur;
    }
    else
	return NULL;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.11  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.10  1996/07/29  16:36:56  eht
 * Incorporate Ravi's new hashing module
 *
 * Revision 1.9  1995/11/10  19:27:23  eht
 * Fix the case when a word has a zero length pronunciation
 *
 * Revision 1.7  1995/10/10  18:35:40  eht
 * Included <s3/n_words.h> for n_words() prototype
 *
 * Revision 1.6  1995/10/10  17:50:43  eht
 * *** empty log message ***
 *
 * Revision 1.5  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.4  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.3  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.2  1995/09/07  19:02:14  eht
 * Added ability to ignore (with a warning message) blank lines
 *
 * Revision 1.1  1995/06/02  14:52:54  eht
 * Initial revision
 *
 *
 */
