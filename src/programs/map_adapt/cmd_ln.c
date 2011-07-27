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

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>

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

    cmd_ln_parse(defn, argc, argv, 1);

    if(cmd_ln_int32("-help")){
      printf("%s\n\n",helpstr);
      exit(0);
    }

    if(cmd_ln_int32("-example")){
      printf("%s\n\n",examplestr);
      exit(0);
    }

    return 0;
}

