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
 * File: itree.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#include <s3/itree.h>

#include <sphinxbase/ckd_alloc.h>

#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

itree_t *
itree_new(uint32 n_cell_hint)
{
    itree_t *new;
    uint32 i;

    new = ckd_calloc(1, sizeof(itree_t));
    new->cell = ckd_calloc(n_cell_hint, sizeof(cell_t));
    for (i = 0; i < n_cell_hint; i++) {
	new->cell[i].id = NO_ID;
	new->cell[i].sib = NULL_INDEX;
	new->cell[i].child = NULL_INDEX;
    }
    new->max_n_cell = n_cell_hint;

    return new;
}

cell_index_t itree_new_cell(itree_t *t)
{
    assert(t->n_cell < t->max_n_cell);

    return t->n_cell++;
}

cell_index_t itree_find(itree_t *t,
			cell_index_t *end,
			cell_index_t start,
			cell_id_t id)
{
    cell_t *cell;
    cell_index_t i, l = NO_ID, o;
    
    cell = t->cell;

    if (cell[start].id != NO_ID) {
	for (i = start; (i != NO_ID) && (cell[i].id != id); i = cell[i].sib)
	    l = i;

	if (i != NO_ID)
	    o = i;
	else 
	    o = NULL_INDEX;
    }
    else {
	o = NULL_INDEX;
	l = start;
    }

    *end = l;

    return o;
}

cell_index_t itree_add_sib(itree_t *t,
			   cell_index_t end,
			   cell_id_t id)
{
    cell_index_t new;
    cell_t *cell;

    if (t->n_cell == t->max_n_cell) {
	E_FATAL("index cells exhausted (exceeded the allocated size %d)\n",t->max_n_cell);
    }

    new = itree_new_cell(t);

    cell = t->cell;
	
    if (new != 0)
	/* this is not the first cell, so link it w/ prior */
	cell[end].sib = new;
    
    cell[new].id = id;
    cell[new].child = NULL_INDEX;
    cell[new].sib = NULL_INDEX;

    return new;
}

cell_index_t itree_add_child(itree_t *t,
			     cell_index_t parent,
			     cell_id_t id)
{
    cell_index_t end;
    cell_index_t child;
    cell_index_t new;
    cell_t *cell;

    cell = t->cell;

    if (cell[parent].child == NO_ID) {
	new = itree_new_cell(t);

	cell[parent].child = new;

	cell[new].id = id;
	cell[new].sib = NULL_INDEX;
	cell[new].child = NULL_INDEX;

	child = new;
    }
    else {
	child = itree_find(t, &end, cell[parent].child, id);
	if (child == NULL_INDEX) {
	    child = itree_add_sib(t, end, id);
	}
    }

    return child;
}

cell_index_t
itree_find_tri(itree_t *t,
	       cell_id_t left_context,
	       cell_id_t right_context,
	       cell_id_t posn)
{
    cell_index_t end;
    cell_index_t parent;

    parent = itree_find(t, &end, 0, left_context);
    if (parent == NULL_INDEX) {
	return NULL_INDEX;
    }

    parent = itree_find(t, &end, t->cell[parent].child, right_context);
    if (parent == NULL_INDEX) {
	return NULL_INDEX;
    }
    
    parent = itree_find(t, &end, t->cell[parent].child, posn);
    if (parent == NULL_INDEX) {
	return NULL_INDEX;
    }

    return t->cell[parent].child;
}

cell_id_t
itree_add_tri(itree_t *t,
	      cell_id_t left_context,
	      cell_id_t right_context,
	      cell_id_t posn,
	      cell_id_t tri_id)
{
    cell_index_t end;
    cell_index_t parent;

    parent = itree_find(t, &end, 0, left_context);
    if (parent == NULL_INDEX) {
	parent = itree_add_sib(t, end, left_context);
	parent = itree_add_child(t, parent, right_context);
	parent = itree_add_child(t, parent, posn);

	t->cell[parent].child = tri_id;

	return parent;
    }

    parent = itree_find(t, &end, t->cell[parent].child, right_context);
    if (parent == NULL_INDEX) {
	parent = itree_add_sib(t, end, right_context);
	parent = itree_add_child(t, parent, posn);

	t->cell[parent].child = tri_id;

	return parent;
    }
    
    parent = itree_find(t, &end, t->cell[parent].child, posn);
    if (parent == NULL_INDEX) {
	parent = itree_add_sib(t, end, posn);
	
	t->cell[parent].child = tri_id;

	return parent;
    }

    return parent;
}

cell_index_t
itree_child(itree_t *t,
	    cell_index_t parent)
{
    return t->cell[parent].child;
}

static itree_t *base = NULL;
static cell_index_t left = NULL_INDEX;
static cell_index_t right = NULL_INDEX;
static cell_index_t posn = NULL_INDEX;

cell_id_t
itree_enum_init(itree_t *root)
{
    base = root;

    left  = 0;				/* first left context */
    right = base->cell[left].child;	/* first right context, given left */
    posn  = base->cell[right].child;	/* first word posn, given left and right */

    return base->cell[posn].child;	/* id of first (base left right posn) */
}

cell_id_t
itree_enum()
{
    cell_index_t nxt;

    nxt = base->cell[posn].sib;		/* get next posn (given left and right) */
    if (nxt != NULL_INDEX) {
	/* exists, so set the posn leaf node to the next one */
	posn = nxt;
    }
    else {
	/* no next posn (given left and right) */
	/* therefore get the next right context if any */

	nxt = base->cell[right].sib;
	if (nxt != NULL_INDEX) {
	    /* found a next right context, so save it */
	    right = nxt;

	    /* set the posn to the first position given the new right */
	    posn = base->cell[right].child;
	}
	else {
	    /* no next right context exists */
	    /* therefore get a new left context if any */
	    nxt = base->cell[left].sib;
	    if (nxt != NULL_INDEX) {
		/* a next left context exists so save it */
		left = nxt;
		/* get the first right context given the left */
		right = base->cell[left].child;

		/* get the first posn given the left and right */
		posn = base->cell[right].child;
	    }
	    else {
		return NULL_INDEX;	/* no more left contexts, so the
					   whole tree for this base phone
					   has been enumerated */
	    }
	}
    }

    /* if we arrive here, posn will reference the cell with the
       next triphone id */

    return base->cell[posn].child;
}
