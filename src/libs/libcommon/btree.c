/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * Revision 1.4  2004/07/21  18:05:39  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
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
