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
/**********************************************************************
 * Hash functions
 * Hash values are from the following set of "good" hash values 
 * 101, 211, 307, 401, 503, 601, 701, 809, 907, 1009, 1201, 1601, 2003, 
 * 2411, 3001, 4001, 5003, 6007, 7001, 8009, 9001, 10007, 12007, 16001, 
 * 20011, 24001, 30011, 40009, 50021, 60013, 70001, 80021, 90001, 100003, 
 * 120011, 160001, 200003, 240007, 300007, 400009, 500009, 600011, 700001, 
 * 800011, 900001
 *
 * Author: Rita Singh
 *********************************************************************/

#ifndef HASH_H
#define HASH_H

#include <sphinxbase/prim_type.h>

#define EOLN  -1
#define HASHSIZE 	24001
#define DICTHASHSIZE 	10007
#define PHNHASHSIZE 	101

typedef struct hashelement_t
{
    char      *basephone;
    char      *leftcontext;
    char      *rightcontext;
    char      *wordposition;
    int32     dictcount;
    int32     count;
    struct hashelement_t  *next;
} hashelement_t;


typedef struct dicthashelement_t
{
    char      *word;
    char      **phones;
    int32     nphns;
    struct dicthashelement_t  *next;
} dicthashelement_t;


typedef struct phnhashelement_t
{
    char      *phone;
    int32     count;
    struct phnhashelement_t  *next;
} phnhashelement_t;


hashelement_t *lookup(const char *basephone,
		      const char *lctxt,
		      const char *rctxt,
		      const char *wordposn,
		      hashelement_t **tphnhash);

hashelement_t *install(const char *basephone,
		       const char *lctxt,
		       const char *rctxt,
		       const char *wordposn,
		       hashelement_t **tphnhash);

void freehash(hashelement_t **hash);


dicthashelement_t *dictlookup(char *word, dicthashelement_t **dicthash);
dicthashelement_t *dictinstall(char *dictword, dicthashelement_t **dicthash);
void freedicthash(dicthashelement_t **dicthash);

phnhashelement_t *phninstall(char *phone, phnhashelement_t **lhash);
void freephnhash(phnhashelement_t **ephnhash);

#endif
