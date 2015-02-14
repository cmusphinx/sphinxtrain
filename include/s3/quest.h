/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * File: quest.h
 * 
 * Description: 
 * 	This file defines the data structures used to implement
 *	the state-tying decision tree questions.
 *
 * Author: 
 * 	Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef QUEST_H
#define QUEST_H
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>
#include <s3/acmod_set.h>

#include <stdio.h>


typedef struct pset_s {
    char *name;		/* name of the phone set (e.g. sonor, vowel, etc.) */
    acmod_id_t *phone;	/* list of member phones */
    uint32 n_phone;	/* cardinality of the set */
    /*
     * Exactly one of the following must be non-NULL for all questions
     */
    uint32 *member;	/* member[p] == TRUE if p is in the phone[] list */
    uint32 *posn;	/* posn[pn] == TRUE if position pn satisfies the query */
} pset_t;

/*
 * Simple Question
 *
 * Simple questions are of the form:
 *	(context[-1] \isin SONOR)
 *	(context[1] \isin LIQUID)
 * 	etc.
 *
 * where SONOR and LIQUID are phone sets derived,
 * for instance, linguistically.
 */
typedef struct quest_s {
    int32 ctxt;		/* .
			   .
			   -2 => two phones to left
			   -1 => one phone to left
			    1 => one phone to right
			    2 => two phones to right
			   .
			   .
			   */
    uint32 neg;		/* TRUE if question q is to be negated */
    uint32 pset;	/* phone set id (index into master list) */

    /*
     * Exactly one of the following must be non-NULL for all questions
     */
    uint32 *member;	/* member[phone] is true if phone is a
			   member of the phone class */
    uint32 *posn;	/* posn[pn] == TRUE if position pn satisfies the query */
} quest_t;

/*
 * Compound question:
 *
 * Compound questions are boolean expressions in
 * standard sum of products form (where + == OR and * == AND).
 * where the boolean variables are instances of simple
 * questions above.
 *
 * For some compound question Q, Q.conj_q[i] is the
 * the ith term in the sum.  Q.conj_q[i][j] is the
 * jth simple question in the ith term of the
 * product.  The # of terms in each product is
 * found by Q.prod_len[i] and the total # of terms
 * in the sum is Q.sum_len.
 */
typedef struct comp_quest_s {
    quest_t **conj_q;
    uint32 *prod_len;
    uint32 sum_len;
} comp_quest_t;

/*
 * Public Fns
 */

int
s3parse_comp_quest(pset_t *pset,
		   uint32 n_pset,
		   comp_quest_t *q,
		   char *q_str);

void
parse_compound_q(comp_quest_t *q,
		 char *q_str);

char *
parse_conj(quest_t **term,
	   uint32 *n_simple_q,
	   char *q_str);

void
print_comp_quest(FILE *fp,
		 pset_t *pset,
		 comp_quest_t *q);

void
print_quest(FILE *fp,
	    pset_t *pset,
	    quest_t *q);

uint32
cnt_q_term(char *q_str);

int
eval_comp_quest(comp_quest_t *q,
		uint32 *feat,
		uint32 n_feat);

int
eval_quest(quest_t *q,
	   uint32 *feat,
	   uint32 n_feat);

/* simplify the conjunctions of
 * a composite question by removing
 * sets that are supersets of other terms
 * in the conjunction.
 */

int
simplify_comp_quest(comp_quest_t *q,
		    uint32 n_phone);

/*
 * Private Fns
 */

#if 0
static void
parse_simple_q(quest_t *q,
	       char *q_str);
#endif

#ifdef __cplusplus
}
#endif
#endif /* QUEST_H */ 
