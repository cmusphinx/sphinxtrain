/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: feat_config.h
 * 
 * Description: 
 * 	This header file contains the interface definitions for all
 *	defined feature sets implemented in this library.
 *
 *	To implement a new feature set, you must implement functions
 *	in the feat_intf_t list feat_conf[].  The current 
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef FEAT_CONFIG_H
#define FEAT_CONFIG_H

typedef struct {
    int (*feat_set)(uint32 id);
} feat_intf_t;

#define FEAT_ID_SPHINX_II_STD	0
#define FEAT_ID_SPHINX_III_0	1
#define N_FEAT_ID		2

static feat_intf_t feat_conf[N_FEAT_ID] = {
    {NULL},		/* FEAT_ID_SPHINX_II_STD */
    {NULL},		/* FEAT_ID_SPHINX_III_0  */
}


#endif /* FEAT_CONFIG_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1996/01/30  17:09:59  eht
 * Initial revision
 *
 *
 */
