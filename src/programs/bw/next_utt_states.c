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
#include <s3/mk_wordlist.h>
#include <s3/ckd_alloc.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>

#include <s3/state_seq.h>

#include "next_utt_states.h"

state_t *next_utt_states(uint32 *n_state,
			 lexicon_t *lex,
			 model_inventory_t *inv,
			 model_def_t *mdef,
			 char *trans)
{
    char **word;
    uint32 n_word;
    uint32 n_phone;
    char *btw_mark;
    acmod_set_t *acmod_set;
    acmod_id_t *phone;
    state_t *state_seq;

    word  = mk_wordlist(trans, &n_word);

    phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);
    if (phone == NULL) {
	E_WARN("Unable to produce CI phones for utt\n");

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
    state_seq_print(state_seq, *n_state, mdef, acmod_set);
#endif

    ckd_free(phone);
    ckd_free(btw_mark);
    ckd_free(word);

    return state_seq;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.13  1996/07/29  16:18:29  eht
 * Moved state_seq module into libcommon
 *
 * Revision 1.12  1995/12/01  20:52:07  eht
 * cvt2triphone() and mk_phone_list() now in libcommon since other commands
 * need them now.
 *
 * Revision 1.11  1995/11/02  16:18:40  eht
 * Fix so that if a phone seq is not able to be produced,
 * the utterance is just skipped
 *
 * Revision 1.10  1995/10/12  18:30:22  eht
 * Made state.h a "local" header file
 *
 * Revision 1.9  1995/10/10  12:43:50  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.8  1995/10/09  15:37:18  eht
 * added missing include of <s3/ckd_alloc.h>
 *
 * Revision 1.7  1995/10/09  15:32:34  eht
 * forgot semi-colon
 *
 * Revision 1.6  1995/10/09  14:55:33  eht
 * Change interface to new ckd_alloc routines
 *
 * Revision 1.5  1995/09/14  14:22:39  eht
 * Changed call to state_seq_print() to conform to new interface
 *
 * Revision 1.4  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.3  1995/06/28  14:33:34  eht
 * Made changes to allow tying to be determined by model_def_t
 * data structure rather than the tying DAG
 *
 * Revision 1.2  1995/06/27  19:25:25  eht
 * Fixed bug in transition matrix mapping found by Chengxiang Lu
 *
 * Revision 1.1  1995/06/02  20:41:22  eht
 * Initial revision
 *
 *
 */
