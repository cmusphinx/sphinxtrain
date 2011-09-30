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
 * Hashing routines used in mk_mdef_gen
 *
 * Author: 
 *    Rita Singh (rsingh@cs.cmu.edu) 
 *********************************************************************/

#include <s3/s3.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"


unsigned hash(const char *s0, const char *s1, const char *s2, const char *s3)
{
    unsigned   hashval;

    for (hashval = 0; *s0 != '\0'; s0++)
        hashval = *s0 + 31*hashval;
    for (; *s1 != '\0'; s1++)
        hashval = *s1 + 31*hashval;
    for (; *s2 != '\0'; s2++)
        hashval = *s2 + 31*hashval;
    for (; *s3 != '\0'; s3++)
        hashval = *s3 + 31*hashval;

    return hashval % HASHSIZE;
}


hashelement_t *lookup(const char *b, const char *l, const char *r, const char *wp, 
			hashelement_t **hashtable)
{
    hashelement_t  *np;

    for (np = hashtable[hash(b,l,r,wp)]; np != NULL; np = np->next)
        if (strcmp(b, np->basephone) == 0 && 
	    strcmp(l, np->leftcontext) == 0 &&
            strcmp(r, np->rightcontext) == 0 && 
	    strcmp(wp, np->wordposition) == 0)
            return np;

    return NULL;
}



hashelement_t *install(const char *b, const char *l, const char *r, const char *wp, 
			hashelement_t **hashtable)
{
    hashelement_t  *np;
    unsigned hashval;

    if ((np = lookup(b, l, r, wp, hashtable)) == NULL)
    {
        np = (hashelement_t *) calloc (1,sizeof(*np));
        if (np == NULL || 
	    (np->basephone = strdup(b)) == NULL || 
	    (np->leftcontext = strdup(l)) == NULL ||
	    (np->rightcontext = strdup(r)) == NULL ||
	    (np->wordposition = strdup(wp)) == NULL)
            return NULL;

        np->dictcount = 0;
	np->count = 0;
        hashval = hash(b,l,r,wp);
        np->next = hashtable[hashval];
        hashtable[hashval] = np;
    }
    return np;
}


void freehash(hashelement_t **hash)
{
    int32 i;
    hashelement_t *e1, *e2;

    for (i=0; i < HASHSIZE; i++){
        e1 = hash[i];
        while (e1 != NULL){
	   e2 = e1->next;
	   free(e1->basephone);
	   if (e1->leftcontext != NULL) free(e1->leftcontext);
	   if (e1->rightcontext != NULL) free(e1->rightcontext);
	   if (e1->wordposition != NULL) free(e1->wordposition);
           free(e1);
           e1 = e2;
        }
    }
    free(hash);
}



unsigned dicthash(char *s)
{
    unsigned   hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 37*hashval;

    return hashval % DICTHASHSIZE;
}



dicthashelement_t *dictlookup(char *s, dicthashelement_t **hashtable)
{
    dicthashelement_t  *np;

    for (np = hashtable[dicthash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->word) == 0)
            return np;

    return NULL;
}



dicthashelement_t *dictinstall(char *name, dicthashelement_t **hashtable)
{
    dicthashelement_t  *np;
    unsigned hashval;

    if ((np = dictlookup(name, hashtable)) == NULL)
    {
        np = (dicthashelement_t *) calloc (1,sizeof(*np));
        if (np == NULL || (np->word = strdup(name)) == NULL)
            return NULL;

        np->nphns = 0;
        hashval = dicthash(name);
        np->next = hashtable[hashval];
        hashtable[hashval] = np;
    }
    return np;
}


void freedicthash(dicthashelement_t **dicthash)
{
    int32 i;
    dicthashelement_t *e1, *e2;

    for (i=0; i < DICTHASHSIZE; i++){
        e1 = dicthash[i];
        while (e1 != NULL){
	   e2 = e1->next;
	   free(e1->word);
	   for (i=0;i<e1->nphns;i++) free(e1->phones[i]);
	   free(e1->phones);
           free(e1);
           e1 = e2;
        }
    }
    free(dicthash);
}



unsigned phnhash(char *s)
{
    unsigned   hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31*hashval;

    return hashval % PHNHASHSIZE;
}



phnhashelement_t *phnlookup(char *s, phnhashelement_t **hashtable)
{
    phnhashelement_t  *np;

    for (np = hashtable[phnhash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->phone) == 0)
            return np;

    return NULL;
}



phnhashelement_t *phninstall(char *name, phnhashelement_t **hashtable)
{
    phnhashelement_t  *np;
    unsigned hashval;

    if ((np = phnlookup(name, hashtable)) == NULL)
    {
        np = (phnhashelement_t *) calloc (1,sizeof(*np));
        if (np == NULL || (np->phone = strdup(name)) == NULL)
            return NULL;

        hashval = phnhash(name);
        np->next = hashtable[hashval];
        hashtable[hashval] = np;
    }
    return np;
}


void freephnhash(phnhashelement_t **phnhash)
{
    int32 i;
    phnhashelement_t *e1, *e2;

    for (i=0; i < PHNHASHSIZE; i++){
        e1 = phnhash[i];
        while (e1 != NULL){
	   e2 = e1->next;
	   free(e1->phone);
           free(e1);
           e1 = e2;
        }
    }
    free(phnhash);
}
