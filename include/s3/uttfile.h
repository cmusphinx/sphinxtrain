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
 * File: uttfile.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef UTTFILE_H
#define UTTFILE_H

#include <s3/prim_type.h>

#include <stdio.h>

typedef struct {
    FILE  *fp;
    uint32 off;
    uint32 len;
} uttfile_t;

uttfile_t *
uttfile_open(const char *fn);

void
uttfile_close(uttfile_t *uf);

int
uttfile_data_at(uttfile_t *uf, uint32 off, char *buf, uint32 sz);

int
uttfile_data_next(uttfile_t *uf, char *buf, uint32 sz);

int
uttfile_data_for(uttfile_t *uf, const char *id, char *buf, uint32 sz);

#endif /* UTTFILE_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:30  awb
 * *** empty log message ***
 * 
 *
 */
