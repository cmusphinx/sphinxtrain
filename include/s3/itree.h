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
 * File: itree.h
 * 
 * Description: 
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 * 
 *********************************************************************/

#ifndef ITREE_H
#define ITREE_H

#include <s3/prim_type.h>

typedef uint32 cell_id_t;
typedef uint32 cell_index_t;

#define NULL_INDEX	(0xffffffff)
#define NO_ID		(0xffffffff)

typedef struct {
    cell_id_t id;		/* id of node */
    cell_index_t child;		/* index of first child context */
    cell_index_t sib;		/* index of next sibling in current context */
} cell_t;

typedef struct {
    cell_t *cell;
    uint32 n_cell;
    uint32 max_n_cell;
} itree_t;

itree_t *
itree_new(uint32 n_cell_hint);

cell_index_t itree_find(itree_t *t,
			cell_index_t *end,
			cell_index_t start,
			cell_id_t id);

cell_index_t
itree_add_sib(itree_t *t,
	      cell_index_t end,
	      cell_id_t id);

cell_index_t
itree_add_child(itree_t *t,
		cell_index_t parent,
		cell_id_t id);

cell_index_t
itree_add_tri(itree_t *t,
	      cell_id_t left_context,
	      cell_id_t right_context,
	      cell_id_t posn,
	      cell_id_t tri_id);

cell_index_t
itree_find_tri(itree_t *t,
	       cell_id_t left_context,
	       cell_id_t right_context,
	       cell_id_t posn);

cell_index_t
itree_child(itree_t *t,
	    cell_index_t parent);

cell_id_t
itree_enum_init(itree_t *t);

cell_id_t
itree_enum(void);

#endif /* ITREE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.3  1996/03/04  15:55:43  eht
 * Added ability to walk the index trees
 *
 * Revision 1.2  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 *
 */
