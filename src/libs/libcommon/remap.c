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
 * File: remap.c
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#include <s3/remap.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <string.h>
#include <stdio.h>
#include <math.h>


static int32
next_prime(uint32 n)
{
    uint32 b;
    uint32 f;
    uint32 l;

    l = n;

    while (1) {
	b = ceil(sqrt((double)l));

	for (f = 2; f < b; f++) {
	    if ((l/f)*f == l) {
		++l;
		break;
	    }
	}

	if (f == b)
	    break;
	if (l == n)
	{
	    fprintf(stderr,"next_prime: failed to find next prime for %d\n",
		    n);
	    break;
	}
    }

    return l;
}

map_t *
remap_init(uint32 n_hashed)
{
    map_t *new;
    unsigned int i;
    uint32 size;

    size = next_prime(n_hashed * 3);

    new = ckd_calloc(1, sizeof(map_t));

    new->map = ckd_calloc(size, sizeof(mapping_t));
    new->inv = ckd_calloc(n_hashed, sizeof(uint32));
    new->n_map = size;

    for (i = 0; i < size; i++) {
	new->map[i].x = FREE_MAPPING;
    }

    return new;
}

uint32
remap_assign(map_t *m, uint32 hx, uint32 x)
{
    mapping_t *map = m->map;
    uint32 i = hx;
    uint32 new_rng;

    do {
	if (map[i].x == FREE_MAPPING) {
	    map[i].x = x;
	    new_rng = m->next_rng++;
	    map[i].y = new_rng;
	    m->inv[new_rng] = x;

	    return map[i].y;
	}
	++i;

	m->n_probe++;

	if (i == m->n_map) i = 0;
	
    } while (i != hx);

    return NO_MAP;
}

uint32
remap(map_t *m, uint32 x)
{
    uint32 hx;

    hx = x % m->n_map;
    
    if (m->map[hx].x == x)
	return m->map[hx].y;
    else {
	return remap_assign(m, hx, x);
    }
}

uint32 *
remap_inverse(map_t *m, uint32 *n_map)
{
    uint32 *inv;

    inv = ckd_calloc(m->next_rng, sizeof(uint32));

    memcpy(inv, m->inv, sizeof(uint32) * m->next_rng); 

    *n_map = m->next_rng;

    return inv;
}

void
remap_status(map_t *m)
{
    E_INFO("nxt_rng= %d, n_probe= %d\n", m->next_rng, m->n_probe);
}

uint32
remap_rngsize(map_t *m)
{
    return m->next_rng;
}

int
remap_free(map_t *m)
{
/*    remap_status(m); */

    ckd_free(m->map);
    ckd_free(m->inv);

    ckd_free(m);

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.6  2005/08/26  20:34:15  egouvea
 * Removed unused variables from the scripts
 * 
 * Revision 1.5  2004/07/21 18:05:40  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.4  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.3  2000/11/17 12:42:53  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.6  1995/10/10  18:43:22  eht
 * Got rid of bcopy() in favor of memcpy() since bcopy is non-ANSI
 *
 * Revision 1.5  1995/10/10  18:39:32  eht
 * got rid of an unused local variable
 *
 * Revision 1.4  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.3  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.2  1995/08/24  20:05:52  eht
 * Changed a sqrt(n) where n is int to sqrt((double)n) for
 * the case of non-ANSI or hosed /usr/include/math.h files.
 *
 * Revision 1.1  1995/06/02  14:52:54  eht
 * Initial revision
 *
 *
 */

