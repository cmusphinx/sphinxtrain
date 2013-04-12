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
 * File: mk_phone_list.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 	Create a phone sequence given a word sequence.
 *
 * Author: 
 * 	Eric H. Thayer ($Author$)
 *********************************************************************/

#include <s3/mk_phone_list.h>

#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>


/*********************************************************************
 *
 * Function:
 *	print_phone_list
 * 
 * Description: 
 *	Prints a list of phones to standard output.
 * 
 * Traceability: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

void
print_phone_list(acmod_id_t *p,
		 uint32 n_p,
		 char *btw,
		 acmod_set_t *acmod_set)
{
    uint32 i, j, k;
    uint32 mpl, l;
    char fmt[16];
    uint32 ppl;

    for (i = 0, mpl = 0; i < n_p; i++) {
	l = strlen(acmod_set_id2name(acmod_set, p[i]));
	if (l > mpl) mpl = l;
    }

    sprintf(fmt, "%%%ds%%s", mpl);

    ppl = 80 / mpl;

    /* print out the phone sequence in rows of at most PPL phones */
    for (i = 0; i < n_p; i += ppl) {
	for (j = i, k = (i + ppl > n_p ? n_p : i + ppl); j < k; j++) {
	    printf(fmt, "", (btw[j] ? "+" : " "));
	}
	printf("\n");

	for (j = i, k = (i + ppl > n_p ? n_p : i + ppl); j < k; j++) {
	    printf(fmt, acmod_set_id2name(acmod_set, p[j]), " ");
	}
	printf("\n");
	printf("\n");
    }
}

/*********************************************************************
 *
 * Function: 
 * 	mk_phone_list
 *
 * Description: 
 * 	Create a sequence of CI phones and word boundary markers for
 *	given a sequence of words.
 *
 * Traceability: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/

acmod_id_t *mk_phone_list(char **btw_mark,
			  uint32 *n_phone,
			  char **word,
			  uint32 n_word,
			  lexicon_t *lex)
{
    uint32 n_p;
    lex_entry_t *e;
    char *btw;
    unsigned int i, j, k;
    acmod_id_t *p;

    if (n_word == 0) {
	E_WARN("Word sequence is empty\n");
	return NULL;
    }

    /*
     * Determine the # of phones in the sequence.
     */
    for (i = 0, n_p = 0; i < n_word; i++) {
	e = lexicon_lookup(lex, word[i]);
	if (e == NULL) {
	    E_WARN("Unable to lookup word '%s' in the dictionary\n", word[i]);

	    return NULL;
	}
	n_p += e->phone_cnt;
    }

    /*
     * Allocate the phone sequence
     */
    p = ckd_calloc(n_p, sizeof(acmod_id_t));

    /*
     * Allocate the between word markers
     */
    btw = ckd_calloc(n_p, sizeof(char));

    for (i = 0, k = 0; i < n_word; i++) {	/* for each word */
	e = lexicon_lookup(lex, word[i]);
	if (e->phone_cnt == 0) /* Ignore words with no pronunciation */
	  continue;
	for (j = 0; j < e->phone_cnt-1; j++, k++) {	/* for all but the last phone in the word */
	    p[k] = e->ci_acmod_id[j];
	}
	p[k] = e->ci_acmod_id[j];	/* move over the last phone */

	btw[k] = TRUE;			/* mark word boundary following
					   kth phone */

	++k;
    }

    *btw_mark = btw;
    *n_phone = n_p;

    assert(k == n_p);

    return p;
}

acmod_id_t *
mk_word_phone_list(char **btw_mark,
		   uint32 *n_phone,
		   char *word,
		   lexicon_t *lex)
{
  uint32 n_p;
  lex_entry_t *e;
  char *btw;
  unsigned int i;
  acmod_id_t *p;
  
  /*
   * Determine the # of phones in the word.
   */
  
  e = lexicon_lookup(lex, word);
  if (e == NULL) {
    E_WARN("Unable to lookup word '%s' in the dictionary\n", word);
    
    return NULL;
  }
  n_p = e->phone_cnt;
  
  /*
   * Allocate the phone sequence
   */
  p = ckd_calloc(n_p, sizeof(acmod_id_t));
  
  /*
   * Allocate the between word markers
   */
  btw = ckd_calloc(n_p, sizeof(char));
  
  if (n_p != 0)
    {
      for (i = 0; i < n_p; i++) {
	p[i] = e->ci_acmod_id[i];
      }
      
      btw[n_p-1] = TRUE;/* mark word boundary following
			   kth phone */
    }
  
  *btw_mark = btw;
  *n_phone = n_p;
  
  return p;
}

acmod_id_t *
mk_boundary_phone(char *word,
		  uint32 n_begin,
		  lexicon_t *lex)
{
  uint32 n_p;
  lex_entry_t *e;
  acmod_id_t *p;
  
  e = lexicon_lookup(lex, word);
  if (e == NULL) {
    E_WARN("Unable to lookup word '%s' in the dictionary\n", word);
    
    return NULL;
  }
  n_p = e->phone_cnt;
  
  p = ckd_calloc(1, sizeof(acmod_id_t));
  
  if (n_p != 0) {
    if (n_begin == 1)
      p[0] = e->ci_acmod_id[0];
    else
      p[0] = e->ci_acmod_id[n_p-1];
  }
  
  return p;
}
