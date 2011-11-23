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
 * File: next_utt_states.c
 * 
 * Description: 
 * 	Get the word transcript for the next utterance and convert it
 *	into a sequence of states ready for forward/backward.
 *
 * Author: 
 * 	Eric H. Thayer, eht@cs.cmu.edu
 *********************************************************************/

#include <s3/lexicon.h>
#include <s3/model_inventory.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>

#include <s3/state_seq.h>

#include "next_utt_states.h"

state_t *next_utt_states(uint32 *n_state,
			 lexicon_t *lex,
			 model_inventory_t *inv,
			 model_def_t *mdef,
			 char *trans
			 )
{
    char **word;
    char *utterance;
    uint32 n_word;
    uint32 n_phone;
    char *btw_mark;
    acmod_set_t *acmod_set;
    acmod_id_t *phone;

    state_t *state_seq;

    utterance = ckd_salloc(trans);
    n_word = str2words(utterance, NULL, 0);
    word = ckd_calloc(n_word, sizeof(char*));
    str2words(utterance, word, n_word);
    
    phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);

    if (phone == NULL) {
	E_WARN("Unable to produce phonetic transcription for the utterance '%s'\n", trans);
	ckd_free(word);
	return NULL;
    }

    acmod_set = inv->acmod_set;

#ifdef NEXT_UTT_STATES_VERBOSE
    print_phone_list(phone, n_phone, btw_mark, acmod_set);
#endif

    cvt2triphone(acmod_set, phone, btw_mark, n_phone);

#ifdef NEXT_UTT_STATES_VERBOSE
    print_phone_list(phone, n_phone, btw_mark, acmod_set);
#endif
    
    state_seq = state_seq_make(n_state, phone, n_phone, inv, mdef);

#ifdef NEXT_UTT_STATES_VERBOSE
    state_seq_print(state_seq, *n_state, mdef);
#endif

    ckd_free(phone);
    ckd_free(btw_mark);
    ckd_free(word);
    ckd_free(utterance);

    return state_seq;
}

state_t *next_utt_states_mmie(uint32 *n_state,
			      lexicon_t *lex,
			      model_inventory_t *inv,
			      model_def_t *mdef,
			      char *curr_word,
			      acmod_id_t *l_phone,
			      acmod_id_t *r_phone
			      )
{
  uint32 n_phone;
  char *btw_mark;
  acmod_set_t *acmod_set;
  acmod_id_t *phone;
  
  state_t *state_seq;
  
  phone = mk_word_phone_list(&btw_mark, &n_phone, curr_word,lex);
  
  if (phone == NULL) {
    E_WARN("Unable to produce phonetic transcription for the word '%s'\n", curr_word);
    return NULL;
  }
  
  acmod_set = inv->acmod_set;
  
  cvt2triphone_mmie(acmod_set, phone, l_phone, r_phone, btw_mark, n_phone);
  
  state_seq = state_seq_make(n_state, phone, n_phone, inv, mdef);
  
  ckd_free(phone);
  ckd_free(btw_mark);
  
  return state_seq;
}
