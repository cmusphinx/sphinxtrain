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
 * File: next_utt_states.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef NEXT_UTT_STATES_H
#define NEXT_UTT_STATES_H

#include <s3/state.h>
#include <s3/prim_type.h>
#include <s3/lexicon.h>
#include <s3/model_inventory.h>
#include <s3/model_def.h>

state_t *next_utt_states(uint32 *n_state,
			 lexicon_t *lex,
			 model_inventory_t *inv,
			 model_def_t *mdef,
			 char *transcript);

#endif /* NEXT_UTT_STATES_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.5  1996/07/29  16:22:02  eht
 * Mixxing includes
 *
 * Revision 1.4  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.3  1995/10/10  12:44:06  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/08/09  20:20:10  eht
 * Use mdef structure for tying info rather than DAG thing
 *
 * Revision 1.1  1995/06/02  20:43:56  eht
 * Initial revision
 *
 *
 */
