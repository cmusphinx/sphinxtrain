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
 * File: btree.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/


#include <s3/btree.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

bt_t *
bt_new()
{
    bt_t *out;

    out = (bt_t *)ckd_calloc(1, sizeof(bt_t));

    return out;
}

bt_node_t *
bt_new_node(uint32 key)
{
    bt_node_t *out;

    out = (bt_node_t *)ckd_calloc(1, sizeof(bt_node_t));

    out->key = key;

    return out;
}

void
bt_free_node(bt_node_t *n)
{
    ckd_free(n);
}

void
bt_free_tree(bt_t *t)
{
    bt_free_subtree(t->root);
    ckd_free((void *)t);
}

bt_node_t *
bt_find_node_subtree(bt_node_t *node, uint32 key)
{
    bt_node_t *ret = NULL;

    if (node->key == key) {
	return node;
    }

    if (node->l) {
	ret = bt_find_node_subtree(node->l, key);
    }
    else {
    }


    if (ret)
	return ret;

    if (node->r) {
	ret = bt_find_node_subtree(node->r, key);
    }
    else {
    }

    return ret;
}

bt_node_t *
bt_find_node(bt_t *t, uint32 key)
{
    bt_node_t *ret;

    if (t->root) {
	ret = bt_find_node_subtree(t->root, key);
    }
    else {
	return NULL;
    }

    return ret;
}

void
bt_free_subtree(bt_node_t *n)
{
    bt_free_subtree(n->l);
    bt_free_subtree(n->r);

    bt_free_node(n);
}

bt_node_t *
bt_add_left(bt_node_t *p, uint32 key)
{
    bt_node_t *l;

    l = bt_new_node(key);
    l->p = p;
    p->l = l;
    
    return l;
}

bt_node_t *
bt_add_right(bt_node_t *p, uint32 key)
{
    bt_node_t *r;

    r = bt_new_node(key);
    r->p = p;
    p->r = r;

    return r;
}

void
bt_visit_pre(bt_node_t *n, void *data, node_op_fn_t op)
{
    if (n == NULL)
	return;

    assert(op != NULL);

    op(data, n);

    bt_visit_pre(n->l, data, op);
    bt_visit_pre(n->r, data, op);
}

void
bt_visit_post(bt_node_t *n, void *data, node_op_fn_t op)
{
    if (n == NULL)
	return;

    assert(op != NULL);

    bt_visit_post(n->l, data, op);
    bt_visit_post(n->r, data, op);

    op(data, n);
}

void
bt_visit(bt_node_t *n, void *data, node_op_fn_t op)
{
    if (n == NULL)
	return;

    assert(op != NULL);

    bt_visit_post(n->l, data, op);
    op(data, n);
    bt_visit_post(n->r, data, op);
}

void
bt_print(bt_node_t *n, void *data, node_op_fn_t op)
{
    if (n == NULL)
	return;

    assert(op != NULL);

    printf("( ");
    op(data, n);

    bt_print(n->l, data, op);
    printf(" ");
    bt_print(n->r, data, op);
    printf(" )");
}
/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.4  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.3  96/06/17  14:33:25  eht
 * Added include of <stdio.h> since standard i/o is used here
 * 
 * Revision 1.2  1996/03/25  15:30:20  eht
 * *** empty log message ***
 *
 * Revision 1.1  1996/03/04  15:55:30  eht
 * Initial revision
 *
 *
 */
