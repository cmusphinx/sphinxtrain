/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996-2005 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: parse_cmd_ln.h
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef PARSE_CMD_LN_H
#define PARSE_CMD_LN_H

int
parse_cmd_ln(int argc, char *argv[]);


#endif /* PARSE_CMD_LN_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2005/06/16  04:31:28  dhdfu
 * Replace this program with my own "map_update" code.  This implements
 * the MAP update equations from Chin-Hui Lee and Jean-Juc Gauvain's
 * papers in addition to the (actually superior) simple interpolation
 * from Sam-Joo's code (which is still the default).  There is no longer
 * any need to run norm to generate an ML estimate, we do that
 * internally.  Also we can now adapt mixture weights, which may or may
 * not improve accuracy slightly versus only updating the means.
 * 
 * Currently this is BROKEN for semi-continuous models (the old map_adapt
 * worked fine for them but didn't do a whole lot since it couldn't
 * update mixture weights).  But it shouldn't be hard to fix it.  Also,
 * variance updating doesn't work, and transition matrix updating isn't
 * implemented.  These might require some changes to bw.
 * 
 *
 */
