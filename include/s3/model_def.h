/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: model_def.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer
 *********************************************************************/

#ifndef MODEL_DEF_H
#define MODEL_DEF_H

#include <s3/prim_type.h>
#include <s3/acmod_set.h>

/*
 * This structure defines the tied transitions
 * and tied states for a phone in the system
 */
typedef struct {
    acmod_id_t p;
    uint32 tmat;
    uint32 *state;
    uint32 n_state;
    char **attrib;	     /* the attribute list for the
			      * phone (e.g. non-speech) */
} model_def_entry_t;

/*
 * This structure contains the definitions for all
 * phones and some parameters about the definitions.
 */
typedef struct {
    acmod_set_t *acmod_set;	/* Data for names <-> id mapping, etc. */
    model_def_entry_t *defn;
    uint32 n_defn;

    uint32 n_total_state;

    uint32 n_tied_ci_state;
    uint32 n_tied_state;
    uint32 n_tied_tmat;

    uint32 *cb;		/* tied state -> codebook ID mapping */
    uint32 n_cb;	/* The # of distinct codebook ID's */

    uint32 max_n_state;	/* max # of states over all models */
    uint32 min_n_state;	/* min # of states over all models */

    acmod_id_t *ts2ci;	/* maps any tied state to ci model ID */
} model_def_t;

#endif /* MODEL_DEF_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/07/29  16:33:40  eht
 * Initial revision
 *
 *
 */
