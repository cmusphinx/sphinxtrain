/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
/*
 * hash.c -- Hash table module with case insensitive word strings as keys.
 *
 * HISTORY
 * 
 * 08-31-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#include <s3/prim_type.h>
#include <s3/err.h>
#include <s3/ckd_alloc.h>
#include <s3/hash.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct hash_entry_s {
    char *key;			/* Original key, NULL if empty slot */
    void *val;			/* Associated with this key */
    struct hash_entry_s *next;	/* For collision resolution */
} hash_entry_t;

typedef struct {
    hash_entry_t *table;	/* Primary hash table, excluding entries that collide */
    char *name;			/* For identification purposes */
    int32 size;			/* Primary hash table size, (is a prime#) */
} hash_table_t;

static hash_table_t *hash_table;	/* A number of hash tables */
static int32 n_table = 0;		/* Current active number of hash tables */
static int32 n_table_alloc = 0;		/* Current number of hash tables allocated, not
					   all may be actively in use */


#if 0
static void prime_sieve (int32 max)
{
    char *notprime;
    int32 p, pp;
    
    notprime = (char *) ckd_calloc (max+1, 1);
    p = 2;
    for (;;) {
	printf ("%d\n", p);
	for (pp = p+p; pp <= max; pp += p)
	    notprime[pp] = 1;
	for (++p; (p <= max) && notprime[p]; p++);
	if (p > max)
	    break;
    }
}
#endif


/*
 * HACK!!  Max hash tables size restricted by this set of primes.  (Of course, collision
 * resolution by chaining will accommodate more entries, but efficiency will drop.)
 */
static int32 prime[] = {
    101, 211, 307, 401, 503, 601, 701, 809, 907,
    1009, 1201, 1601, 2003, 2411, 3001, 4001, 5003, 6007, 7001, 8009, 9001,
    10007, 12007, 16001, 20011, 24001, 30011, 40009, 50021, 60013, 70001, 80021, 90001,
    100003, 120011, 160001, 200003, 240007, 300007, 400009, 500009, 600011, 700001, 800011, 900001,
    -1
};


static int32 prime_size (int32 size)
{
    int32 i;
    
    for (i = 0; (prime[i] > 0) && (prime[i] < size); i++);
    if (prime[i] <= 0) {
	E_WARN("Very large hash table (%d entries)\n", size);
	--i;
    }
    return (prime[i]);
}


hash_t hash_new (char *name, int32 size)
{
    if (n_table == n_table_alloc) {
	/* Need to grow allocated tables */
	if (n_table_alloc > 0) {
	    n_table_alloc += 8;
	    hash_table = (hash_table_t *) ckd_realloc (hash_table,
						       n_table_alloc*sizeof(hash_table_t));
	} else {
	    n_table_alloc = 7;
	    hash_table = (hash_table_t *) ckd_calloc (n_table_alloc, sizeof(hash_table_t));
	}
    }
    
    hash_table[n_table].name = name;
    hash_table[n_table].size = prime_size (size+(size>>1));
    hash_table[n_table].table = (hash_entry_t *) ckd_calloc (hash_table[n_table].size,
							     sizeof(hash_entry_t));

#if _HASH_TEST_
    E_INFO("hash_new(%s,%d): handle= %d, tablesize= %d\n",
	   name, size, n_table, hash_table[n_table].size);
#endif

    return (n_table++);
}


#define UPPER_CASE(c)	((((c) >= 'a') && ((c) <= 'z')) ? (c-32) : c)

/*
 * Compute hash value for given key string.
 * Somewhat tuned for normal text word strings.
 */
static uint32 key2hash (hash_t h, char *key)
{
    register char *cp, c;
    register int32 s;
    register uint32 hash;
    
    hash = 0;
    s = 0;
    for (cp = key; *cp; cp++) {
	c = *cp;
	c = UPPER_CASE(c);
	hash += c << s;
	s += 5;
	if (s >= 25)
	    s -= 24;
    }

    return (hash % hash_table[h].size);
}


/*
 * Case insensitive key string compare.  Return the usual -1, 0, +1, depending on
 * key1 <, =, > key2 (case insensitive, of course).
 */
static int32 keycmp (char *key1, char *key2)
{
    char c1, c2;
    
    for (;;) {
	c1 = *(key1++);
	c1 = UPPER_CASE(c1);
	c2 = *(key2++);
	c2 = UPPER_CASE(c2);
	if (c1 != c2)
	    return (c1-c2);
	if (c1 == '\0')
	    return 0;
    }
}


/*
 * Lookup chained entries with hash-value hash in table h for given key and return
 * associated value.  If successful return 0, else -1.
 */
static int32 lookup (hash_t h, uint32 hash, char *key, void **val)
{
    hash_entry_t *entry;
    
    entry = &(hash_table[h].table[hash]);
    if (entry->key == NULL)
	return -1;
    
    while (entry && (keycmp(entry->key, key) != 0))
	entry = entry->next;

    if (entry) {
	*val = entry->val;
	return 0;
    } else
	return -1;
}


/*
 * Lookup hash table h for given key and return associated val.  If successful, return
 * 0, else -1.
 */
int32 hash_lookup (hash_t h, char *key, void **val)
{
    uint32 hash;
    int32 res;
    
    assert ((h >= 0) && (h < n_table));		/* Must be a valid, active hash table */
    assert (key != NULL);

    hash = key2hash (h, key);
    res = lookup (h, hash, key, val);

#if _HASH_TEST_
    E_INFO("hash_lookup(%s, %s): hash= %d, lookup= %d, val= %d\n",
	   hash_table[h].name, key, hash, res, *val);
#endif

    return (res);
}


/*
 * Add a new entry with given key and associated value to hash table h.  The key must
 * not already exist in the table.
 */
int32 hash_enter (hash_t h, char *key, void *val)
{
    uint32 hash;
    void *junk;
    hash_entry_t *cur, *new;
    
    assert ((h >= 0) && (h < n_table));		/* Must be a valid, active hash table */
    assert (key != NULL);
    
    hash = key2hash (h, key);

#if _HASH_TEST_
    E_INFO("hash_enter(%s, %s, %p): hash= %d\n", hash_table[h].name, key, val, hash);
#endif

    if (lookup (h, hash, key, &junk) >= 0) {
	E_ERROR("Hash table %s: Duplicate definition: %s\n", hash_table[h].name, key);
	return -1;
    }
    
    cur = &(hash_table[h].table[hash]);
    if (cur->key == NULL) {
	/* Empty slot at hashed location; add this entry */
	cur->key = key;
	cur->val = val;
	assert (cur->next == NULL);
    } else {
	new = (hash_entry_t *) ckd_calloc (1, sizeof(hash_entry_t));
	new->key = key;
	new->val = val;
	new->next = cur->next;
	cur->next = new;
    }

    return 0;
}


#if _HASH_TEST_

static void hash_stat (hash_t h)
{
    int32 i, m, n, sz;
    hash_entry_t *entry;
    
    sz = hash_table[h].size;
    m = n = 0;
    
    for (i = 0; i < sz; i++) {
	entry = &(hash_table[h].table[i]);
	if (entry->key) {
	    m++;
	    n++;
	    for (entry = entry->next; entry; entry = entry->next)
		n++;
	}
    }

    E_INFO("hash_stat(%s): %d keys entered, %d collisions\n",
	   hash_table[h].name, n, n-m);
}


main (int32 argc, char *argv[])
{
    FILE *fp;
    char str[1024];
    char wd[1024], *tmp;
    int32 i, n;
    hash_t h;
    
    if (argc < 2) {
	E_INFO("Usage: %s <dictionary file>\n", argv[0]);
	exit(-1);
    }
    if ((fp = fopen(argv[1], "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", argv[1]);
    
    n = 0;
    while (fgets (str, sizeof(str), fp) != NULL)
	n++;
    h = hash_new (n);
    
    rewind (fp);
    i = 0;
    while (fgets (str, sizeof(str), fp) != NULL) {
	if (sscanf (str, "%s", wd) == 1) {
	    tmp = (char *) ckd_malloc (strlen(wd)+1);
	    strcpy (tmp, wd);
	    if (hash_enter (h, tmp, i) < 0)
		E_FATAL("hash_enter failed\n");
	    i++;
	}
    }

    rewind (fp);
    while (fgets (str, sizeof(str), fp) != NULL) {
	if (sscanf (str, "%s", wd) == 1)
	    hash_lookup (h, wd, &i);
    }

    hash_stat (h);
}

#endif
