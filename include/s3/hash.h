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
/*
 * hash.h -- Hash table module with case insensitive word strings as keys.
 *
 * HISTORY
 * 
 * 08-31-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */

/*
 * Hash table: associates strings (keys) with (void *) values.  The strings are treated
 * in a case-INsensitive manner.
 */

#ifndef _LIBUTIL_HASH_H_
#define _LIBUTIL_HASH_H_

#include "prim_type.h"

typedef int32 hash_t;	/* Type of handle to hash tables */

/*
 * Allocate hash table
 * Return value: handle to allocated hash table.
 */
hash_t hash_new (char *name,	/* In: For identifying the hash table to report errors */
		 int32 size);	/* In: Expected no. of entries in the hash table */

/*
 * Add new entry to hash table, associating the given value with the given key.
 * Return value: 0 if successful, -1 if error (eg, key conflict).
 */
int32 hash_enter (hash_t h,	/* In: Handle of hash table to which addition being made */
		  char *key,	/* In: Key identifying the new entry */
		  void *val);	/* In: Value associated with above key */

/*
 * Extract value associated with given key in a hash table.
 * Return 0 if successful, else -1 (eg, no entry with give key).
 */
int32 hash_lookup (hash_t h,	/* In: Handle of hash table being searched */
		   char *key,	/* In: Key for which value being sought */
		   void **val);	/* Out: *val = value associated with key */

#endif
