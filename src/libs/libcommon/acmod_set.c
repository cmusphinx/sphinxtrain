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
 * File: acmod_set.c
 * 
 * Description: 
 *	This is an implementation of the acmod_set (acoustic model set) module.
 *	It's primary function is to create and maintain the
 *	one-to-one mapping from ASCII acoustic model names to consecutive
 *	integers from 0 through N-1 where N is the total number
 *	of acoustic models known by the system.
 *
 *	Another function is to create/maintain an attribute list
 *	associated w/ each acoustic model and be able to answer attribute
 *	existence questions above a given acoustic model (e.g. Is this acoustic model
 *	a filler model?, etc.).
 *
 * ------------------------------------------------------------------
 *	RESTRICTIONS OF THIS IMPLEMENTATION:
 *
 *	- All base acoustic models must be defined before any triphone.
 *	- No more than 256 CI acoustic models may be defined
 *	- Triphones are the only multiphones supported at present.
 * ------------------------------------------------------------------
 *
 *	A workable calling sequence to this interface is be:
 *
 *		p = acmod_set_new()		- creates a new acmod_set structure
 *		acmod_set_set_n_ci_hint(p, ...)	- gives an optional CI acoustic model
 *					  	  count
 *		acmod_set_set_n_tri_hint(p, ...)- gives an optional triphone model
 *					  	  count.
 * 		for (...) {			
 *			acmod_set_add_ci(p, ...); - add in all the ci
 *							phones
 *		}
 *		for (...) {
 *			acmod_set_add_tri(p, ...);  - add in all the tri phones
 *		}
 *
 *		base = acmod_set_name2id(p, base_name);   - map a base phone name to an
 *					     		    id.
 *		left = acmod_set_name2id(p, left_ctxt_name);
 *					
 *		right = acmod_set_name2id(p, right_ctxt_name);
 *
 *		tri = acmod_set_tri2id(p, base, right, posn);   - get a triphone id
 *							     given base, left, right
 *							     and word position.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs)
 *********************************************************************/


#include <s3/acmod_set.h>

#include <s3/ckd_alloc.h>
#include <s3/s2_param.h>
#include <s3/s3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

/*********************************************************************
 *
 * Function: acmod_set_new()
 * 
 * Description: 
 *	This function will create a new acmod_set structure.
 *	It allocates space for the object and zeros the
 *	allocated space so that all the counts contained in it are
 *	zero initially and all the pointers are NULL.
 * 
 * Function Inputs: 
 *	None
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	A pointer to an allocated acmod_set_t structure.
 * 
 * Global Outputs: 
 *	None
 * 
 * Postconditions:
 *	Return value must be a pointer to allocated space for a
 *	acmod_set_t structure.
 *
 * Errors: 
 *	None at this level.
 * 
 *********************************************************************/

acmod_set_t *
acmod_set_new()
{
    /* ckd_calloc() will exit if the allocation fails */

    return ckd_calloc(1, sizeof(acmod_set_t));
}

/*********************************************************************
 *
 * Function: acmod_set_set_n_ci_hint()
 * 
 * Description: 
 *	For those callers that know in advance how many CI phones
 *	will be created in the acoustic model set, this routine
 *	gives a chance to pre-allocate space for them.
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 *	S3_SUCCESS - if call completed successfully.
 * 
 * Global Outputs: 
 *	None
 * 
 * Pre-Conditions: 
 *	acmod_set argument must be a valid non-zero pointer.
 *
 * Errors: 
 *	None at this level (subroutines might abort)
 * 
 *********************************************************************/

int32
acmod_set_set_n_ci_hint(acmod_set_t *acmod_set,
			uint32 n_ci)
{
    assert(acmod_set->ci == NULL);

    /* ckd_calloc aborts if no mem is avail */
    acmod_set->ci = ckd_calloc(n_ci,
			       sizeof(ci_acmod_t));

    acmod_set->max_n_ci = n_ci;

    assert(acmod_set->next_id == 0);

    return S3_SUCCESS;
}

int32
acmod_set_set_n_tri_hint(acmod_set_t *acmod_set,
			 uint32 n_tri)
{
    assert(acmod_set->multi == NULL);
    assert(acmod_set->max_n_multi == 0);
    assert(acmod_set->n_multi == 0);
    
    if (n_tri > 0) {
	acmod_set->multi = ckd_calloc(n_tri,
				      sizeof(acmod_t));

	acmod_set->max_n_multi = n_tri;
    }

    return S3_SUCCESS;
}

const char **new_attrib(const char **in)
{
    char **out;
    uint32 len, i;

    for (len = 0; in[len]; len++);

    out = (char **)ckd_calloc(len+1, sizeof(char *));

    for (i = 0; i < len; i++) {
	out[i] = strdup(in[i]);
    }

    out[i] = NULL;

    return (const char **)out;
}

/*********************************************************************
 *
 * Function: acmod_set_add_ci()
 * 
 * Description: 
 * 	This routine will add a context independent acoustic model to
 *	the set.
 *
 * Function Inputs: 
 * 
 * Global Inputs: 
 *	None
 * 
 * Return Values: 
 * 	The id of the new CI model
 *
 * Global Outputs: 
 *	None
 * 
 * Pre-Conditions: 
 *	- acmod_set argument must be a valid non-zero pointer.
 *	- No prior calls to acmod_set_add_tri() have been made.
 *
 * Errors: 
 * 
 *********************************************************************/

acmod_id_t
acmod_set_add_ci(acmod_set_t *acmod_set,
		 const char *name,
		 const char **attrib)
{
    uint32 id;

    if (acmod_set->n_multi > 0) {
	E_FATAL("Current acmod_set implementation requires all CI phones"
		"to be defined before defining multi phones\n");
    }

    if (acmod_set->n_ci == acmod_set->max_n_ci) {
	E_FATAL("Current acmod_set implementation requires the 'n_ci_hint' to be"
		"exactly the number to be added.\n");
    }

    id = acmod_set->next_id;

    acmod_set->next_id++;
    acmod_set->n_ci++;

    /* copy the name and attribute lists so that the caller
     * does not have restrictions on overwriting the value
     * of the passed arguments */

    acmod_set->ci[id].name   = strdup(name);
    acmod_set->ci[id].attrib = new_attrib(attrib);

    /* the following allows ci_acmod_t structures to be
       passed w/o reference to the acmod_set structure itself */
    acmod_set->ci[id].id = id;

    return id;
}

/*********************************************************************
 *
 * Function: acmod_set_add_tri()
 * 
 * Description: 
 *	This function adds a new triphone to the phone set.
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 	None
 * 
 * Return Values: 
 *	An ID for the new triphone.
 * 
 * Global Outputs: 
 * 	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 *	- acmod_set argument must be a valid non-zero pointer.
 * 
 *********************************************************************/

acmod_id_t
acmod_set_add_tri(acmod_set_t *acmod_set,
		  acmod_id_t base,
		  acmod_id_t left_context,
		  acmod_id_t right_context,
		  word_posn_t posn,
		  const char **attrib)
{
    acmod_id_t new;
    uint32 addr;
    uint32 n_ci;
    uint32 i;
    acmod_t *multi;

    if (acmod_set->n_multi == acmod_set->max_n_multi) {
	E_FATAL("Current acmod_set implementation requires the 'n_tri_hint' to "
		"be not less than the number to be added.\n");
    }
    
    /* get the multi-phone list (e.g. di, tri, quad phones ) */
    multi = acmod_set->multi;

    n_ci = acmod_set->n_ci;

    if (acmod_set->multi_idx == NULL) {
	/* allocate space for multiphone -> acoustic model id
	 * index */

	acmod_set->multi_idx = ckd_calloc(n_ci,
					  sizeof(itree_t *));

	for (i = 0; i < n_ci; i++) {
	    acmod_set->multi_idx[i] = itree_new(n_ci * n_ci * N_WORD_POSN);
	}
    }
    
    assert(base < 256);
    assert(left_context < 256);
    assert(right_context < 256);

    new = acmod_set->next_id;

    /* adds an index cell for this triphone. */
    itree_add_tri(acmod_set->multi_idx[base],
		  left_context, right_context, posn,
		  new);
    
    /* multiphone list does not include CI acoustic models */
    addr = new - n_ci;
    
    /* get ready for next add if any */
    acmod_set->next_id++;

    multi[addr].base = base;
    multi[addr].left_context = left_context;
    multi[addr].right_context = right_context;
    multi[addr].posn = posn;
    multi[addr].attrib = new_attrib(attrib);

    acmod_set->n_multi++;

    return new;
}

/*********************************************************************
 *
 * Function: acmod_set_name2id()
 * 
 * Description: 
 *	This function maps the C string name for a phone into
 *	a phone id.
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 	None
 * 
 * Return Values: 
 *	The ID for the phone - if it exists
 *	NO_ACMOD	     - otherwise.
 * 
 * Global Outputs: 
 * 	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 *	- acmod_set argument must be a valid non-zero pointer.
 * 
 *********************************************************************/

acmod_id_t
acmod_set_name2id(acmod_set_t *acmod_set,
		  const char *name)
{
    uint32 i;
    char tmp_store[1024];
    char *tmp;
    int n_scan;
    int n_assign;
    char base[1024];
    acmod_id_t base_id;

    char left[1024];
    acmod_id_t left_id;

    char right[1024];
    acmod_id_t right_id;

    char *word_posn_map = WORD_POSN_CHAR_MAP;
    char posn[12];
    uint32 posn_id;

    strcpy(tmp_store, name);

    tmp = tmp_store;

    n_assign = sscanf(tmp, "%s%n", base, &n_scan);
    tmp += n_scan;

    if (n_assign != 1) {
	E_ERROR("Error parsing acoustic model name %s\n", name);
	return NO_ACMOD;
    }

    n_assign = sscanf(tmp, "%s%n", left, &n_scan);
    tmp += n_scan;
    
    if (n_assign != 1) {
	for (i = 0; i < acmod_set->n_ci; i++) {
	    if (strcmp(acmod_set->ci[i].name, base) == 0)
		return i;
	}
    
	return NO_ACMOD;
    }
    else {
	n_assign = sscanf(tmp, "%s%n", right, &n_scan);
	tmp += n_scan;
    
	if (n_assign != 1) {
	    E_ERROR("Parse error while getting right context of %s\n", name);

	    return NO_ACMOD;
	}

	n_assign = sscanf(tmp, "%s%n", posn, &n_scan);
	tmp += n_scan;
    
	if (n_assign != 1) {
	    E_ERROR("Parse error while getting word position of %s\n", name);
	    
	    return NO_ACMOD;
	}

	base_id  = acmod_set_name2id(acmod_set, base);
	left_id  = acmod_set_name2id(acmod_set, left);
	right_id = acmod_set_name2id(acmod_set, right);

	for (posn_id = 0; posn_id < strlen(word_posn_map); posn_id++) {
	    if (posn[0] == word_posn_map[(uint32)posn_id])
		break;
	}

	if (posn_id == strlen(word_posn_map)) {
	    E_ERROR("Unknown word position identifier %s\n", posn);

	    return NO_ACMOD;
	}
	
	return acmod_set_tri2id(acmod_set, base_id, left_id, right_id, (word_posn_t)posn_id);
    }
    
    return NO_ACMOD;
}

/*********************************************************************
 *
 * Function: acmod_set_id2s2name()
 * 
 * Description: 
 *	This function maps a phone ID into a SPHINX-II phone name
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 	None
 * 
 * Return Values: 
 *	A C string name for the phone	- if the ID is valid
 *	NULL				- otherwise.
 * 
 * Global Outputs: 
 * 	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 *	- acmod_set argument must be a valid non-zero pointer.
 *********************************************************************/

const char *
acmod_set_id2s2name(acmod_set_t *acmod_set,
		    acmod_id_t id)
{
    static char str[1024];
    char *pmap = WORD_POSN_CHAR_MAP;
    acmod_id_t base;
    acmod_id_t left;
    acmod_id_t right;
    word_posn_t posn;
    
    if (id < acmod_set->n_ci)
	return acmod_set->ci[id].name;
    
    else if (id < acmod_set->next_id) {

	acmod_set_id2tri(acmod_set, &base, &left, &right, &posn, id);

	sprintf(str, "%s(%s,%s)%c",
		acmod_set_id2name(acmod_set, base),
		acmod_set_id2name(acmod_set, left),
		acmod_set_id2name(acmod_set, right),
		(posn != WORD_POSN_INTERNAL ? pmap[(int)posn] : '\0'));
	
	return str;
    }
    else
	return NULL;
}

/*********************************************************************
 *
 * Function: acmod_set_id2name()
 * 
 * Description: 
 *	This function maps a phone ID into a C string.
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 	None
 * 
 * Return Values: 
 *	A C string name for the phone	- if the ID is valid
 *	NULL				- otherwise.
 * 
 * Global Outputs: 
 * 	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 *	- acmod_set argument must be a valid non-zero pointer.
 *********************************************************************/

const char *
acmod_set_id2name(acmod_set_t *acmod_set,
		  acmod_id_t id)
{
    static char str[1024];
    char *pmap = WORD_POSN_CHAR_MAP;
    acmod_id_t base;
    acmod_id_t left;
    acmod_id_t right;
    word_posn_t posn;

    if (id < acmod_set->n_ci)
	return acmod_set->ci[id].name;

    else if (id < acmod_set->next_id) {

	acmod_set_id2tri(acmod_set, &base, &left, &right, &posn, id);

	sprintf(str, "%s %s %s %c",
		acmod_set_id2name(acmod_set, base),
		acmod_set_id2name(acmod_set, left),
		acmod_set_id2name(acmod_set, right),
		pmap[(int)posn]);
	
	return str;
    }
    else
	return NULL;
}

/*********************************************************************
 *
 * Function: acmod_set_id2fullname()
 * 
 * Description: 
 *	This function maps a phone ID into a C string where context
 *	independent phones are fully expanded as in mdef files.
 * 
 * Function Inputs: 
 * 
 * Global Inputs: 
 * 	None
 * 
 * Return Values: 
 *	A C string name for the phone	- if the ID is valid
 *	NULL				- otherwise.
 * 
 * Global Outputs: 
 * 	None
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 *	- acmod_set argument must be a valid non-zero pointer.
 *********************************************************************/

const char *
acmod_set_id2fullname(acmod_set_t *acmod_set,
		      acmod_id_t id)
{
    static char str[1024];
    char *pmap = WORD_POSN_CHAR_MAP;
    acmod_id_t base;
    acmod_id_t left;
    acmod_id_t right;
    word_posn_t posn;

    if (id < acmod_set->n_ci) {
	sprintf(str, "%s - - -", acmod_set->ci[id].name);

	return str;
    }
    else if (id < acmod_set->next_id) {

	acmod_set_id2tri(acmod_set, &base, &left, &right, &posn, id);

	sprintf(str, "%s %s %s %c",
		acmod_set_id2name(acmod_set, base),
		acmod_set_id2name(acmod_set, left),
		acmod_set_id2name(acmod_set, right),
		pmap[(int)posn]);
	
	return str;
    }
    else
	return NULL;
}

acmod_id_t
acmod_set_enum_init(acmod_set_t *acmod_set,
		    acmod_id_t base)
{
    cell_index_t ret;

    if (acmod_set->n_multi == 0) {
	/* none defined */

	return NO_ACMOD;
    }

    ret = itree_enum_init(acmod_set->multi_idx[base]);

    if (ret != NULL_INDEX) {
	return ret;
    }
    else {
	return NO_ACMOD;
    }
}

acmod_id_t
acmod_set_enum()
{
    cell_index_t ret;

    ret = itree_enum();

    if (ret != NULL_INDEX) {
	return ret;
    }
    else {
	return NO_ACMOD;
    }
}

acmod_id_t
acmod_set_tri2id(acmod_set_t *acmod_set,
		 acmod_id_t phone,
		 acmod_id_t left_context,
		 acmod_id_t right_context,
		 word_posn_t posn)
{
    itree_t *idx;
    cell_index_t i;

    if (acmod_set->n_multi == 0) {
	/* none defined */

	return NO_ACMOD;
    }

    idx = acmod_set->multi_idx[phone];

    i = itree_find_tri(idx, left_context, right_context, posn);

    if (i != NULL_INDEX)
	return i;
    else
	return NO_ACMOD;
}

int32
acmod_set_id2tri(acmod_set_t *acmod_set,
		 acmod_id_t *phone,
		 acmod_id_t *left_context,
		 acmod_id_t *right_context,
		 word_posn_t *posn,
		 acmod_id_t id)
{
    acmod_t *multi = acmod_set->multi;
    uint32 addr;

    if (id < acmod_set->n_ci) {
	E_ERROR("ID given to acmod_set_id2tri() is a CI acoustic model ID\n");

	return S3_ERROR;
    }

    addr = id - acmod_set->n_ci;

    *phone         = multi[addr].base;
    *left_context  = multi[addr].left_context;
    *right_context = multi[addr].right_context;
    *posn          = multi[addr].posn;

    assert(acmod_set_tri2id(acmod_set,
			    multi[addr].base,
			    multi[addr].left_context,
			    multi[addr].right_context,
			    multi[addr].posn) == id);
			    
    return S3_SUCCESS;
}

uint32
acmod_set_n_acmod(acmod_set_t *acmod_set)
{
    return acmod_set->n_ci + acmod_set->n_multi;
}

uint32
acmod_set_n_ci(acmod_set_t *acmod_set)
{
    return acmod_set->n_ci;
}

uint32
acmod_set_n_multi(acmod_set_t *acmod_set)
{
    return acmod_set->n_multi;
}

acmod_id_t
acmod_set_base_phone(acmod_set_t *acmod_set, acmod_id_t id)
{
    uint32 addr;

    if (id < acmod_set->n_ci)
	return id;
    else if (id < acmod_set->next_id) {
	addr = id - acmod_set->n_ci;

	return acmod_set->multi[addr].base;
    }
    else {
	E_ERROR("acmod_set_base_phone() ID==%u is out-of-range\n", id);
	
	return NO_ACMOD;
    }
}

int
acmod_set_is_triphone(acmod_set_t *acmod_set, acmod_id_t id)

{
    return id >= acmod_set->n_ci;
}

uint32
acmod_set_has_attrib(acmod_set_t *acmod_set,
		     acmod_id_t id,
		     const char *attrib)
{
    const char **attrib_list;
    uint32 addr;
    uint32 i;

    if (attrib == NULL)
	return FALSE;

    if (id < acmod_set->n_ci) {
	attrib_list = acmod_set->ci[id].attrib;
    }
    else if (id < acmod_set->next_id) {
	addr = id - acmod_set->n_ci;

	attrib_list = acmod_set->multi[addr].attrib;
    }
    else {
	E_ERROR("Phone id %u is out of range.\n", id);

	return FALSE;
    }

    assert(attrib_list);

    for (i = 0; attrib_list[i]; i++) {
	if (strcmp(attrib_list[i], attrib) == 0)
	    return TRUE;
    }

    return FALSE;
}

const char **
acmod_set_attrib(acmod_set_t *acmod_set,
		 acmod_id_t id)
{
    const char **attrib_list;
    uint32 addr;

    if (id < acmod_set->n_ci) {
	attrib_list = acmod_set->ci[id].attrib;
    }
    else if (id < acmod_set->next_id) {
	addr = id - acmod_set->n_ci;

	attrib_list = acmod_set->multi[addr].attrib;
    }
    else {
	E_ERROR("Phone id %u is out of range.\n", id);

	return FALSE;
    }

    assert(attrib_list);
    
    return attrib_list;
}

int
acmod_set_s2_parse_triphone(acmod_set_t *acmod_set,
			    acmod_id_t *base,
			    acmod_id_t *left,
			    acmod_id_t *right,
			    word_posn_t *posn,
			    char *str)
{
    char *tok;
    char *save;
    uint32 i;
    char *pmap = WORD_POSN_CHAR_MAP;

    save = strdup(str);

    tok = strtok(save, "(,)");
    *base = acmod_set_name2id(acmod_set, tok);
    if (*base == NO_ACMOD)
	goto error;

    tok = strtok(NULL, "(,)");
    *left = acmod_set_name2id(acmod_set, tok);
    if (*left == NO_ACMOD)
	goto error;

    tok = strtok(NULL, "(,)");
    *right = acmod_set_name2id(acmod_set, tok);
    if (*right == NO_ACMOD)
	goto error;

    tok = strtok(NULL, "(,)");
    if (tok) {
	for (i = 0; i < strlen(pmap); i++) {
	    if (tok[0] == pmap[i])
		break;
	}

	if (i < strlen(pmap))
	    *posn = (word_posn_t)i;

	/* hack for parsing seno mapping files */
	if (tok[0] == '<')
	    *posn = WORD_POSN_INTERNAL;
    }
    else {
	*posn = WORD_POSN_INTERNAL;
    }
    
    free(save);
    return S3_SUCCESS;

error:
    free(save);
    return S3_ERROR;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.5  2006/02/16  00:18:23  egouvea
 * Implemented flexible warping function. The user can specify at run
 * time which of several shapes they want to use. Currently implemented
 * are an affine function (y = ax + b), an inverse linear (y = a/x) and a
 * piecewise linear (y = ax, up to a frequency F, and then it "breaks" so
 * Nyquist frequency matches in both scales.
 * 
 * Added two switches, -warp_type and -warp_params. The first specifies
 * the type, which valid values:
 * 
 * -inverse or inverse_linear
 * -linear or affine
 * -piecewise or piecewise_linear
 * 
 * The inverse_linear is the same as implemented by EHT. The -mel_warp
 * switch was kept for compatibility (maybe remove it in the
 * future?). The code is compatible with EHT's changes: cepstra created
 * from code after his changes should be the same as now. Scripts that
 * worked with his changes should work now without changes. Tested a few
 * cases, same results.
 * 
 * Revision 1.4  2004/07/21 18:05:39  egouvea
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
 * Revision 1.11  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.10  96/03/25  15:29:13  eht
 * Added id2s2name functions so that SPHINX-II triphone
 * names could be outputted if necessary.
 * 
 * Revision 1.9  1996/03/04  15:53:36  eht
 * Added ability to walk the triphone trees
 *
 * Revision 1.8  1995/10/12  15:07:03  eht
 * Fixed bug when acmod_set_name2id() is given a triphone
 * but the acoustic model set does not have any triphones.
 *
 * Revision 1.7  1995/10/10  15:09:38  eht
 * Cleaned up some signed/unsigned stuff.
 *
 * Revision 1.6  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.5  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.4  1995/10/05  12:56:42  eht
 * Fix problem w/ attribute lists on HPUX.
 * Make acmod_set interface consistent w/ header file.
 * Extend acmod_set_name2id() to include triphone names.
 *
 * Revision 1.3  1995/09/13  12:27:09  eht
 * Do some sanity checking of the indexing structures.
 *
 * Revision 1.2  1995/09/08  19:11:14  eht
 * Updated to use new acmod_set module.  Prior to testing
 * on TI digits.
 *
 * Revision 1.1  1995/09/08  13:49:52  eht
 * Initial revision
 *
 * Revision 1.1  95/06/02  14:52:54  14:52:54  eht (Eric Thayer)
 * Initial revision
 * 
 *
 */
