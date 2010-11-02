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
/* ------------------ HEAP.H ---------------------*/
#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <sphinxbase/prim_type.h>

#define MAXSWAPS 1000 	/* Dont expect more than a 1000 swaps */

typedef struct heapelement_t {
    char *basephone;
    char *leftcontext;
    char *rightcontext;
    char *wordposition;
    int32 count;
    int32 heapid;
} heapelement_t;

#define PARENTID(X)     ((int32)((X)-1)/2)
#define LEFTCHILD(X)    ((X)*2+1)
#define RIGHTCHILD(X)   ((X)*2+2)
#define SWAP(A,B)  {heapelement_t *tmp; int32 x,y; \
                    x = (A)->heapid; y = (B)->heapid; \
                    tmp = (A); A = B; B = tmp; \
                    (A)->heapid = x; (B)->heapid = y;}
int32 insert (heapelement_t ***heap, int32 heapsize, heapelement_t *newelement);
heapelement_t *yanktop (heapelement_t ***heap, int32 heapsize, int32 *newsize);
void free_heap (heapelement_t **heap, int32 heapsize); 
void free_heapelement (heapelement_t *heapel);

#endif
