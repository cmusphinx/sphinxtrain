/* ====================================================================
 * Copyright (c) 2000 Carnegie Mellon University.  All rights 
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
 * Heap routines used in mk_mdef_gen
 *
 * Author: 
 *    Rita Singh (rsingh@cs.cmu.edu) 
 *********************************************************************/

#include <s3/s3.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"

int32 compare(heapelement_t *l1, heapelement_t *l2)
{
    int32 diff;

    /* For increasing alphabetical order */
    if ((diff = strcmp(l2->basephone,l1->basephone)) == 0){
	if (l2->leftcontext != NULL && l1->leftcontext != NULL){

	    /* These are triphones */
            if ((diff = strcmp(l2->leftcontext,l1->leftcontext)) == 0){
                if ((diff = strcmp(l2->rightcontext,l1->rightcontext)) == 0){
                    diff = strcmp(l2->wordposition,l1->wordposition);
                }
            }
        }
    }
    return diff;
}


int32 insert(heapelement_t ***heap, int32 heapsize, heapelement_t *newelement)
{
    heapelement_t **lheap;
    int32 id, newheapsize = heapsize + 1;

    if (heapsize == 0) *heap = NULL; /* Needed for realloc to work */
    lheap = (heapelement_t**) realloc(*heap,newheapsize*sizeof(heapelement_t*));

    lheap[heapsize] = newelement;
    newelement->heapid = heapsize;

    id = heapsize;
    while (id > 0 && compare(lheap[id],lheap[PARENTID(id)]) > 0){
	SWAP(lheap[id],lheap[PARENTID(id)]);
	id = PARENTID(id);
    }

    *heap = lheap;
    return(newheapsize);
}


heapelement_t *yanktop(heapelement_t ***heap, int32 heapsize, int32 *newsize)
{
    heapelement_t **lheap,*out;
    int32 left,right,high,id = 0,newheapsize;

    lheap = *heap;
    out = lheap[0];

    lheap[0] = lheap[heapsize-1];
    id = 0;
    while (id < heapsize-1) {
        left = LEFTCHILD(id);
        right = RIGHTCHILD(id);
        if (right > heapsize-2 && left > heapsize-2) break;
        if (left == heapsize-2) high = left;
        else {
            if (compare(lheap[right],lheap[left]) < 0) high = left;
            else high = right; 
        }
        if (compare(lheap[id],lheap[high]) < 0){
            SWAP(lheap[id],lheap[high]);
            id = high;
        }
        else break;
    }

    *newsize = newheapsize = heapsize-1;
    *heap = (heapelement_t**) realloc(lheap,newheapsize*sizeof(heapelement_t*));

    return(out);
}


void free_heap(heapelement_t **heap, int32 heapsize)
{
    int32 i;
    if (heapsize == 0) return;
    for (i=0; i<heapsize; i++) free_heapelement(heap[i]);
    free(heap);
}


void free_heapelement(heapelement_t *heapel)
{
    free(heapel->basephone); 
    if (heapel->leftcontext != NULL) free(heapel->leftcontext); 
    if (heapel->rightcontext != NULL) free(heapel->rightcontext); 
    if (heapel->wordposition != NULL) free(heapel->wordposition); 
    free(heapel);
}
