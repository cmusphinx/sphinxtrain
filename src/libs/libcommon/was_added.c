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
 * File: was_added.c
 * 
 * Description: 
 *	The files in this routine are used to keep track of whether
 *	a given destination tied parameter has already had a source
 *	tied parameter added to it.  This is used to initialize
 *	a destination set of tied parameters from a source set.
 *
 *	The resulting set is not optimal, but further iterations of
 *	reestimation should be able to refine the initial estimate.
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/was_added.h>

#include <sphinxbase/ckd_alloc.h>

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/
pair_t **
init_was_added(uint32 n_dest)
{
    return (pair_t **)ckd_calloc(n_dest, sizeof(pair_t *));
}

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/
int
find_added(pair_t *l, uint32 s)
{
    pair_t *c;

    for (c = l; c && (c->src_id != s); c = c->next);

    if (c == NULL) {
	/* either L is an empty list or S is not on the list */
	return FALSE;
    }
    else {
	return TRUE;
    }
}

/*********************************************************************
 *
 * Function: 
 * 
 * Description: 
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 
 * Return Values: 
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/
int
was_added(pair_t **l,
	  uint32 src)
{
    pair_t *new;
    int ret;

    /* see if given src has been added to the destination yet or not */
    ret = find_added(*l, src);

    if (ret == FALSE) {
	/* it hasn't, so prepend an entry for it on the list */

	new = (pair_t *)ckd_calloc(1, sizeof(pair_t));

	new->src_id = src;
    
	new->next = *l;
	*l = new;
    }

    return ret;
}

