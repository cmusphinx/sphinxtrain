/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
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

#include <s3/acmod_set.h>
#include <s3/list.h>
#include <s3/hash.h>

#include <s3/prim_type.h>

typedef uint32 word_id_t;
#define WORD_NO_ID	(0xffffffff)

typedef struct lex_entry_str {
    char *ortho;
    word_id_t word_id;
    char **phone;
    acmod_id_t *ci_acmod_id;
    uint32  phone_cnt;

    struct lex_entry_str *next;
} lex_entry_t;

typedef struct {
/*    lex_entry_t *entry; */

    lex_entry_t *head;
    lex_entry_t *tail;

    uint32 entry_cnt;
    hash_t ht;
} lexicon_t;

lexicon_t *lexicon_new(void);

lexicon_t *
lexicon_read(lexicon_t *prior_lex,
	     const char *lex_file_name,
	     acmod_set_t *phone_set);

lex_entry_t *
lexicon_lookup(lexicon_t *lexicon,
	       char *word);

#endif /* LEXICON_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.4  1996/07/29  16:40:50  eht
 * make a name more consise
 *
 * Revision 1.3  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.2  1995/09/08  19:13:52  eht
 * Updated to remove references to pset module and add references
 * to acmod_set module
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
