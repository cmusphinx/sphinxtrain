/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: heap.c
 * 
 * Description: 
 * 	Maintain a list of floating point values and a corresponding
 *	list of keys in a heap.
 *
 * Author: 
 * 	Pirated from Algorithms book.
 *********************************************************************/

#include <sphinxbase/err.h>

#include <s3/heap.h>
#include <s3/s3.h>

#define LEFT(i)   ((i<<1) + 1)
#define RIGHT(i)  ((i<<1) + 2)
#define PARENT(i) ((i-1) >> 1)

/*
 * float32 heaps
 */
int
heap32b_max_comp(float32 a, float32 b)
{
    return (a > b);
}

int
heap32b_min_comp(float32 a, float32 b)
{
    return (a < b);
}

void
heapify32b(float32 *a, uint32 *k,
	    uint32 i,
	    uint32 sz,
	    hcmpfn32b_t cmp)
{
    uint32 l, r, b, t_k;
    float32 t_a;

    l = LEFT(i);
    r = RIGHT(i);

    /* Determine the "best" node of i, l or r */
    if ((l < sz) && cmp(a[l], a[i]))
	b = l;
    else
	b = i;

    if ((r < sz) && cmp(a[r], a[b]))
	b = r;

    /* If node i is not the first in the ordering,
     * then switch i with the left or right child that is first and
     * heapify the the tree rooted at the new node i position
     */
    if (b != i) {
	/* exchange the values... */
	t_a = a[i];
	a[i] = a[b];
	a[b] = t_a;

	/* ... and the keys */
	t_k = k[i];
	k[i] = k[b];
	k[b] = t_k;

	heapify32b(a, k, b, sz, cmp);
    }
}

void
heap32b_a2h(float32 *a, uint32 *k, uint32 sz, hcmpfn32b_t cmp)
{
    int32 i;

    for (i = (sz>>1)-1; i >= 0; i--)
	heapify32b(a, k, i, sz, cmp);
}

uint32
heap32b_ins(float32 *a,  uint32 *k, uint32 sz,
	    float32 a_v, uint32 k_v,
	    hcmpfn32b_t cmp)
{
    uint32 i, p;

    ++sz;

    i = sz-1;
    p = PARENT(i);
    /* if the parent does not satisfy the ordering,
     * then switch with i */
    while ((i > 0) && !cmp(a[p], a_v)) {
	a[i] = a[p];
	k[i] = k[p];
	i = p;
	p = PARENT(p);
    }
    a[i] = a_v;
    k[i] = k_v;

    return sz;
}

uint32
heap32b_extr_top(float32 *out_a,
		 uint32 *out_k,
		 float32 *a, uint32 *k, uint32 sz,
		 hcmpfn32b_t cmp)
{
    if (sz < 1) {
	E_ERROR("Heap underflow\n");
	return NO_KEY;
    }

    *out_a = a[0];
    *out_k = k[0];

    a[0] = a[sz-1];
    k[0] = k[sz-1];

    sz--;

    heapify32b(a, k, 0, sz, cmp);

    return sz;
}

/*
 * float64 heaps
 */
int
heap64b_max_comp(float64 a, float64 b)
{
    return (a > b);
}

int
heap64b_min_comp(float64 a, float64 b)
{
    return (a < b);
}

void
heapify64b(float64 *a, uint32 *k,
	    uint32 i,
	    uint32 sz,
	    hcmpfn64b_t cmp)
{
    uint32 l, r, b, t_k;
    float64 t_a;

    l = LEFT(i);
    r = RIGHT(i);

    /* Determine "best" node
     * given node i and its left and right
     * children
     */
    if ((l < sz) && cmp(a[l], a[i]))
	b = l;
    else
	b = i;

    if ((r < sz) && cmp(a[r], a[b]))
	b = r;

    /* If node i is not the first in the ordering,
     * then switch i with the left or right child that is first and
     * heapify the the tree rooted at the new node i position
     */
    if (b != i) {
	/* exchange the values... */
	t_a = a[i];
	a[i] = a[b];
	a[b] = t_a;

	/* ... and the keys */
	t_k = k[i];
	k[i] = k[b];
	k[b] = t_k;

	heapify64b(a, k, b, sz, cmp);
    }
}

void
heap64b_a2h(float64 *a, uint32 *k, uint32 sz, hcmpfn64b_t cmp)
{
    int32 i;

    for (i = (sz>>1)-1; i >= 0; i--)
	heapify64b(a, k, i, sz, cmp);
}

uint32
heap64b_ins(float64 *a,  uint32 *k, uint32 sz,
	    float64 a_v, uint32 k_v,
	    hcmpfn64b_t cmp)
{
    uint32 i, p;

    ++sz;

    i = sz-1;
    p = PARENT(i);
    /* if the parent does not satisfy the ordering,
     * then switch with i */
    while ((i > 0) && !cmp(a[p], a_v)) {
	a[i] = a[p];
	k[i] = k[p];
	i = p;
	p = PARENT(p);
    }
    a[i] = a_v;
    k[i] = k_v;

    return sz;
}

uint32
heap64b_extr_top(float64 *out_a,
		 uint32 *out_k,
		 float64 *a, uint32 *k, uint32 sz,
		 hcmpfn64b_t cmp)
{
    if (sz < 1) {
	E_ERROR("Heap underflow\n");
	return NO_KEY;
    }

    *out_a = a[0];
    *out_k = k[0];

    /* grab some item from the heap and
       stick it in the hole */

    a[0] = a[sz-1];
    k[0] = k[sz-1];

    sz--;

    heapify64b(a, k, 0, sz, cmp);

    return sz;
}
		  
#ifdef TEST
#include <stdio.h>
main()
{
    float64 a[] = {4,1,3,2,16,9,10,14,8,7};
    float64 h[] = {4,1,3,2,16,9,10,14,8,7};
    float64 t_a;
    uint32  k[] = {0,1,2,3, 4,5, 6, 7,8,9};
    uint32 t_k;
    uint32 i, sz;

    printf("a[] ==");
    for (i = 0; i < 10; i++) {
	printf(" %3.1f", a[i]);
    }
    printf("\n");
    printf("k[] ==");
    for (i = 0; i < 10; i++) {
	printf(" %3u", k[i]);
    }
    printf("\n");

    for (i = 0, sz = 0; i < 10; i++) {
	sz = heap64b_ins(h, k, sz, a[i], i, &heap64b_min_comp);
    }

    for (i = 0; i < 10; i++) {
	sz = heap64b_extr_top(&t_a, &t_k, h, k, sz, &heap64b_min_comp);
	printf("%u %3.1f\n", t_k, t_a);
    }
}
#endif

