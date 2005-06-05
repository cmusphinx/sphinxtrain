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
 * Revision 1.5  2005/06/05  22:00:34  arthchan2003
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

/* TIME
 *  Functions to compute CPU (user & sys) and REAL time elapsed.
 *    Elapsed_???_time returns the number of milliseconds elapsed since
 *    last (update = 1).
 *    If update = 0, just peek and return how much time has elapsed.
 *    If update = 1, actually change the from time for next call.   
 *
 * Kai-Fu Lee
 * 6/86 Created
 */

#if !defined(WIN32)
#include <sys/types.h>
#include <sys/times.h>
#include <sys/timeb.h>	/* For real time */
#else
#include <time.h>
#endif

#define	HZ	60	/* 60 clock ticks per second */


#if !defined(WIN32)
float elapsed_user_time (update)
int update;
{
  float  ret;
  static struct tms ubefore, uafter;

  times (&uafter);
  ret = (double) (uafter.tms_utime - ubefore.tms_utime) / HZ;
  if (update)
    times (&ubefore);
  return (ret);
}

float elapsed_wall_time (update)
int update;
{
  float  ret;
  static struct timeb ubefore, uafter;

/*  ftime (&uafter); */
  ret = (float) (uafter.time + uafter.millitm/1000.0 -
		 ubefore.time - ubefore.millitm/1000.0);
/*
  if (update)
    ftime (&ubefore);
*/
  return (ret);
}

float elapsed_sys_time (update)
int update;
{
  float  ret;
  static struct tms sbefore, safter;

  times (&safter);
  ret = (double) (safter.tms_stime - sbefore.tms_stime) / HZ;
  if (update)
    times (&sbefore);
  return (ret);
}

int elapsed_real_time (update)
int   update;
{
  int ret;
  static struct timeb rbefore,
                      rafter;

/*  ftime (&rafter); */
  ret = 1000 * (rafter.time - rbefore.time) +
	(rafter.millitm - rbefore.millitm);
/*
  if (update)
    ftime (&rbefore);
*/
  return (ret);
}

#else

long elapsed_user_time (int update)
{
  time_t ret;
  static time_t ubefore, uafter;

  time (&uafter);
  ret = (uafter - ubefore);
  if (update)
    time (&ubefore);
  return ((long)ret);
}

#endif
