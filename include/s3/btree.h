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
 * File: btree.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef BTREE_H
#define BTREE_H

#include <s3/prim_type.h>

/* Data structures & type defn's */

typedef struct bt_node_struct {
    struct bt_node_struct *p;	/* parent node */
    uint32 key;			/* some key value */
    struct bt_node_struct *l;	/* left child */
    struct bt_node_struct *r;	/* right child */
} bt_node_t;

typedef struct bt_struct {
    bt_node_t *root;
} bt_t;

typedef void (*node_op_fn_t)(void *data, bt_node_t *node);

#define NO_KEY	0xffffffff

/* Public Fns */

/* Creation */
bt_t *
bt_new(void);

/* Tree building functions */
bt_node_t *
bt_new_node(uint32 key);

bt_node_t *
bt_add_left(bt_node_t *p, uint32 key);

bt_node_t *
bt_add_right(bt_node_t *p, uint32 key);


/* Tree deallocation functions */
void
bt_free_tree(bt_t *t);

void
bt_free_subtree(bt_node_t *n);

void
bt_free_node(bt_node_t *n);

/*
 * Search tree for key
 */
bt_node_t *
bt_find_node(bt_t *tree, uint32 key);

bt_node_t *
bt_find_node_subtree(bt_node_t *node, uint32 key);

/* Tree traversal functions */
void
bt_visit_pre(bt_node_t *n, void *data, node_op_fn_t op);	/* pre-order */

void
bt_visit(bt_node_t *n, void *data, node_op_fn_t op);		/* in order */

void
bt_visit_post(bt_node_t *n, void *data, node_op_fn_t op);	/* post-order */

void
bt_print(bt_node_t *n, void *data, node_op_fn_t op);

/* Private Fns */

#endif /* BTREE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.3  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.2  96/03/25  15:46:57  eht
 * development version
 * 
 * Revision 1.1  1996/03/04  15:55:43  eht
 * Initial revision
 *
 *
 */
