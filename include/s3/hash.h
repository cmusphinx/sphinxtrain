/*
 * hash.h -- Hash table module with case insensitive word strings as keys.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
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
