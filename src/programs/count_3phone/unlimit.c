/* ====================================================================
 * Copyright (c) 1986-2000 Carnegie Mellon University.  All rights 
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
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.6  2005/06/05  22:00:34  arthchan2003
 * Log. Rewriting QUICK_COUNT using SphinxTrain command line functions. Several changes.
 * 1, Removal of -B -t because they were actually not implemented.
 * 2, Add SphinxTrain's style command line, help string and example string.
 * 3, Safe-guarding a, invalid file names, b, user didn't specify SIL in the phone list.
 * 4, Change all quit to E_INFO, also delete obsolete quit.c.  Will change the windows setup respectively.
 * 5, Fix bug 1151880.  That was caused by the use of magic phrase symbol &, the loop will read stuff out of memoery boundary  when & occurs at the end of the word.  This was fixed by specifically checking this particular condition in quick_phone.c.
 * 
 * Windows setup is not yet checked in. Will do right after the linux check-in.
 * 
 * Major revamped by Arthur Chan at 2005 Jun 5
 *
 */

/* Procedure to "unlimit" the memory usage of a program.
   The limit is set to the maximum that the system allows.
   The limit is returned.

   Kai-Fu Lee
   10/86
 */

#if ((! WIN32) && (! _HPUX_SOURCE))
#include <sys/time.h>
#include <sys/resource.h>
#include "count.h"
#endif
#include <stdio.h>

int unlimit ()
{
#if ((! WIN32) && (! _HPUX_SOURCE) && defined(RLIMIT_DATA))
  struct rlimit rl;

  getrlimit(RLIMIT_DATA,&rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_DATA,&rl);
  return (rl.rlim_cur);
#else
  printf("unlimit: setrlimit() of RLIMIT_DATA unsupported on this architecture\n");
  return 0;
#endif
}
