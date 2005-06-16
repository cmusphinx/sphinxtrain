/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1994-2005 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: parse_cmd_ln.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Sam-Joo Doh <sjdoh@cs.cmu.edu>
 *	David Huggins-Daines <dhuggins@cs.cmu.edu>
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/cmd_ln.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <sys_compat/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>


/* defines, parses and (partially) validates the arguments
   given on the command line */

int
parse_cmd_ln(int argc, char *argv[])
{
#include "cmd_ln_defn.h"

    cmd_ln_define(defn);

    if (argc == 1) {
	cmd_ln_print_definitions();
	exit(1);
    }

    cmd_ln_parse(argc, argv);

    if(cmd_ln_int32("-help")){
      printf("%s\n\n",helpstr);
      exit(1);
    }

    if(cmd_ln_int32("-example")){
      printf("%s\n\n",examplestr);
      exit(1);
    }

    cmd_ln_print_configuration();

    return 0;
}


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
 */

