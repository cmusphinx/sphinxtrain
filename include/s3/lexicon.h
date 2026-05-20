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
 * File: lexicon.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef LEXICON_H
#define LEXICON_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <s3/acmod_set.h>
#include <sphinxbase/prim_type.h>
#include <sphinxbase/hash_table.h>
#include <sphinxbase/strfuncs.h>

typedef uint32 word_id_t;
#define WORD_NO_ID	(0xffffffff)

typedef struct lex_entry_str {
    char *ortho;
    word_id_t word_id;
    char **phone;
    acmod_id_t *ci_acmod_id;
    uint32  phone_cnt;

    /* Linked list of pronunciation variants of the same base word.
     * Entries with full orthos "reading", "reading(2)", "reading(3)"
     * share base word "reading" and are chained via this pointer.
     * The chain head is what lexicon_lookup_variants() returns.
     * Single-variant words have next_variant == NULL.
     */
    struct lex_entry_str *next_variant;
} lex_entry_t;

typedef struct lexicon_s {
    uint32 entry_cnt;
    hash_table_t *ht;       /* full ortho ("reading(2)") -> lex_entry_t */
    hash_table_t *base_ht;  /* base word ("reading")     -> head of variant chain */
    acmod_set_t *phone_set;
} lexicon_t;

lexicon_t *lexicon_new(void);

lexicon_t *
lexicon_read(lexicon_t *prior_lex,
	     const char *lex_file_name,
	     acmod_set_t *phone_set);

lex_entry_t *
lexicon_lookup(lexicon_t *lexicon,
	       char *word);

/* Return the head of the linked list of pronunciation variants for
 * `base_word` (i.e. the word with any "(N)" suffix stripped).
 * Returns NULL if no entries match. Walk the chain via
 * entry->next_variant; ordering follows insertion order.
 */
lex_entry_t *
lexicon_lookup_variants(lexicon_t *lexicon,
                        const char *base_word);

/* Small accessor helpers for callers that treat lex_entry_t as opaque. */
const char *
lex_entry_ortho(const lex_entry_t *e);

lex_entry_t *
lex_entry_next_variant(const lex_entry_t *e);

void lexicon_free(lexicon_t *lexicon);

#ifdef __cplusplus
}
#endif
#endif /* LEXICON_H */ 
