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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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

#include <sys/types.h>
#include <sys/times.h>
#include <sys/timeb.h>	/* For real time */

#define	HZ	60	/* 60 clock ticks per second */


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
