/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * File: read_seno_dtree.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

static char rcsid[] = "@(#)$Id$";

#include <s3/read_seno_dtree.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <string.h>
#include <assert.h>

static int
read_header_line(uint32 *n_base,
		 char *id,
		 uint32 *n_cd,
		 FILE *fp)
{
    if (fscanf(fp, "%u:%s %u",
	       n_base, id, n_cd) == 3) {
	return S3_SUCCESS;
    }
    else {
	return S3_ERROR;
    }
}

static int
read_node_line(float32 *out_entropy,
	       uint32 *out_key,
	       uint32 *out_left_key,
	       uint32 *out_right_key,
	       char *q_str,
	       FILE *fp)
{
    int ret;

    ret = fscanf(fp, "%f %u %u %u",
		 out_entropy, out_key, out_left_key, out_right_key);

    if (ret != 4) {
	if (ret < 0)
	    return -1;
	else
	    return 0;
    }

    fgets(q_str, 1024, fp);
    q_str[strlen(q_str)-1] = '\0';

    return 1;
}

int
read_seno_dtree_file(dtree_t **out_dt,
		     const char *file_name)
{
    uint32 n_base;
    uint32 n_cd;
    char tree_id[64];
    float32 ent;
    uint32 key, l_key, r_key, n_key = 0, *k2q = NULL;
    char q_str[1024], *rem_q_str;
    FILE *fp;
    int i, j, n_quest;
    dtree_t *dt;
    bt_node_t *node;
    bt_t *tree;
    comp_quest_t *q;
    float32 *q2ent;

    *out_dt = dt = (dtree_t *)ckd_calloc(1, sizeof(dtree_t));

    dt->tree = tree = bt_new();

    fp = fopen(file_name, "r");
    if (fp == NULL) {
	E_WARN_SYSTEM("Unable to open %s for reading", file_name);

	return S3_ERROR;
    }

    for (n_quest = 0; fgets(q_str, 1024, fp) != NULL; n_quest++);

    --n_quest;	/* account for header line */

    dt->n_quest = n_quest;

    rewind(fp);

    read_header_line(&n_base, tree_id, &n_cd, fp);
    for (i = 0; read_node_line(&ent, &key, &l_key, &r_key, q_str, fp) > 0; i++) {
	if (n_key < l_key)
	    n_key = l_key;
	if (n_key < r_key)
	    n_key = r_key;
    }
    ++n_key;
    dt->n_key = n_key;

    rewind(fp);

    read_header_line(&n_base, tree_id, &n_cd, fp);

    E_INFO("Reading tree %s (%u base phones, %u CD phones, %u quest)\n",
	   tree_id, n_base, n_cd, n_quest);

    dt->quest = q = (comp_quest_t *)ckd_calloc(n_quest,
						    sizeof(comp_quest_t));
    dt->k2q   = k2q   = (uint32 *)ckd_calloc(n_key, sizeof(uint32));
    dt->q2ent = q2ent = (float32 *)ckd_calloc(n_quest, sizeof(float32));

    for (i = 0; i < n_key; i++) {
	k2q[i] = NO_MAP;
    }

    for (i = 0; read_node_line(&ent, &key, &l_key, &r_key, q_str, fp) > 0; i++) {
	if (tree->root) {
	    E_INFO("%u\n", key);

	    node = bt_find_node(tree, key);
	    if (node) {
		/* grow left and right children */
		bt_add_left(node, l_key);
		bt_add_right(node, r_key);
	    }
	    else {
		E_FATAL("Find node w/ key %u failed\n", key);
	    }
	}
	else {
	    E_INFO("root %u\n", key);
	    tree->root = bt_new_node(key);
	    bt_add_left(tree->root, l_key);
	    bt_add_right(tree->root, r_key);
	}

	k2q[key] = i;
	q2ent[i] = ent;
	parse_compound_q(&q[i], q_str);
    }

    assert(i == n_quest);

    return S3_SUCCESS;
}
/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.1  1996/03/26  15:16:26  eht
 * Initial revision
 *
 *
 */
