/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: heap.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef HEAP_H
#define HEAP_H

#include <s3/prim_type.h>

#ifndef NO_KEY
#define NO_KEY (uint32)(0xffffffff)
#endif

/* Functions for heaps of 32-bit float values */
typedef int (*hcmpfn32b_t)(float32 a, float32 b);

int
heap32b_max_comp(float32 a, float32 b);

int
heap32b_min_comp(float32 a, float32 b);

void
heapify32b(float32 *a, uint32 *k, uint32 i, uint32 sz, hcmpfn32b_t fn);

void
heap32b_a2h(float32 *a, uint32 *k, uint32 sz, hcmpfn32b_t fn);

uint32
heap32b_ins(float32 *a,  uint32 *k, uint32 sz,
	    float32 a_v, uint32 k_v, hcmpfn32b_t fn);

uint32
heap32b_extr_top(float32 *out_a, uint32 *out_k,
		 float32 *a, uint32 *k, uint32 sz, hcmpfn32b_t fn);

/* Functions for heaps of 64-bit float values */
typedef int (*hcmpfn64b_t)(float64 a, float64 b);

int
heap64b_max_comp(float64 a, float64 b);

int
heap64b_min_comp(float64 a, float64 b);

void
heapify64b(float64 *a, uint32 *k, uint32 i, uint32 sz, hcmpfn64b_t fn);

void
heap64b_a2h(float64 *a, uint32 *k, uint32 sz, hcmpfn64b_t fn);

uint32
heap64b_ins(float64 *a,  uint32 *k, uint32 sz,
	    float64 a_v, uint32 k_v, hcmpfn64b_t fn);

uint32
heap64b_extr_top(float64 *out_a, uint32 *out_k,
		 float64 *a, uint32 *k, uint32 sz, hcmpfn64b_t fn);

#endif /* HEAP_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.1  97/07/16  11:39:10  eht
 * Initial revision
 * 
 *
 */
