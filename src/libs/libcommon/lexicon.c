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
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/hash_table.h>
#include <sphinxbase/pio.h>

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

    new->entry_cnt = 0;

    new->ht = hash_table_new(64000, HASH_CASE_YES);

    return new;
}

lex_entry_t *lexicon_append_entry(lexicon_t *lex)
{
    lex_entry_t *new;

    new = ckd_calloc(1, sizeof(lex_entry_t));
    lex->entry_cnt++;

    return new;
}

void lexicon_entry_free(lex_entry_t *entry)
{
    if (entry->ortho)
        ckd_free(entry->ortho);
    if (entry->ci_acmod_id)
        ckd_free(entry->ci_acmod_id);
    if (entry->phone)
        ckd_free(entry->phone);
    ckd_free(entry);
    return;
}

lexicon_t *lexicon_read(lexicon_t *prior_lex,
			const char *filename,
			acmod_set_t *acmod_set)
{
    FILE *lex_fp;
    lineiter_t *line = NULL;
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
	E_FATAL_SYSTEM("Unable to open dictionary file '%s' for reading\n",
		       filename);
    }

    if (prior_lex)
	lex = prior_lex;
    else
	lex = lexicon_new();

    if (lex->phone_set == NULL)
	lex->phone_set = acmod_set;

    start_wid = wid = lex->entry_cnt;
    for (line = lineiter_start_clean(lex_fp); line; line = lineiter_next(line)) {

	if (line->buf[0] == 0) {
	    E_WARN("Dictionary file '%s' has a blank line at line %d\n",
		   filename, lineno);
	    continue;
	}
	
	next_entry = lexicon_append_entry(lex);

	/* allocate space for string.  It will be parsed
	 * by strtok() */
	lex_line = strdup(line->buf);

	/* get the word and make a hash table entry for
	   this lexicon entry */

	word = strtok(lex_line, " \t");	

	if (add_word(word,
		     wid,
		     lex,
		     next_entry) != S3_SUCCESS) {
	    E_ERROR("%s duplicate entry?\n", word);
	    lexicon_entry_free(next_entry);
	    continue;
	}

	/* str2words() counts the # of space separated "words" on a line */
	n_phone = str2words(line->buf, NULL, 0) - 1;

#ifdef LEXICON_VERBOSE
	E_INFO("%s %d phones\n", line->buf, n_phone);
#endif

	/* read the phones, convert to ids and add them
	   to the phone list for this entry */
	if (add_phones(n_phone, next_entry, acmod_set) != S3_SUCCESS) {
	    E_ERROR("pronunciation for %s has undefined phones; skipping.\n", word);
	    lexicon_entry_free(next_entry);
	    continue;
	}
	
	hash_table_enter(lex->ht, next_entry->ortho, (void *)next_entry);
	++wid;	/* only happens should everything be successful */
    }

    E_INFO("%d entries added from %s\n",
	   wid - start_wid, filename);
			  
    lineiter_free(line);
    fclose(lex_fp);
    
    return lex;
}

lex_entry_t *lexicon_lookup(lexicon_t *lex, char *ortho)
{
    lex_entry_t *cur;
    
    if (hash_table_lookup(lex->ht, ortho, (void **)&cur) == 0) {
	return cur;
    }
    return NULL;
}


void lexicon_free(lexicon_t *lexicon)
{
    hash_iter_t *itor;
    
    for (itor = hash_table_iter(lexicon->ht);
             itor; itor = hash_table_iter_next(itor)) {
        lex_entry_t *entry = hash_entry_val(itor->ent);
        lexicon_entry_free(entry);
    }
    hash_table_free(lexicon->ht);
    ckd_free(lexicon);
}
