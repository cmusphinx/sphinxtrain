/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: remap.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef REMAP_H
#define REMAP_H

#include <s3/prim_type.h>

typedef struct {
    uint32 x;
    uint32 y;
} mapping_t;

typedef struct {
    mapping_t *map;
    uint32 *inv;
    uint32 n_map;
    uint32 n_probe;
    uint32 next_rng;
} map_t;

#define FREE_MAPPING	0xffffffff
#define NO_MAP		0xffffffff

uint32
remap(map_t *m, uint32 x);

map_t *
remap_init(uint32 size);

void
remap_status(map_t *m);

uint32
remap_rngsize(map_t *m);

uint32 *
remap_inverse(map_t *m, uint32 *n_map);

int
remap_free(map_t *m);

#endif /* REMAP_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 * Revision 1.2  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
