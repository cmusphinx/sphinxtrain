#ifndef _PREFETCH_H
#define _PREFETCH_H

/*
 * prefetch.c -- a simple interface to set up prefetching of files that
 * a program will be opening soon.
 *
 * By Paul W. Placeway, CMU Speech Group, May 23, 1995.
 * (Thanks to Brian Noble suggesting the open, read one char, close idea.) 
 */


int prefetch_init(void);

int prefetch_hint(char *fn);

int prefetch_finish(void);

#endif /* _PREFETCH_H */
