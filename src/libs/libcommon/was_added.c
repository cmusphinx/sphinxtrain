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

#include <s3/ckd_alloc.h>

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


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.1  1995/12/15  18:37:07  eht
 * Initial revision
 *
 *
 */
