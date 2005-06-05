/* ====================================================================
 * Copyright (c) 1988-2000 Carnegie Mellon University.  All rights 
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
 * Revision 1.5  2005/06/05  22:00:33  arthchan2003
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

/* Make word-dependent triphones and dictinary
   from word-independent triphones and dictinary

   Mei-Yuh Hwang    Oct. 1988
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "count.h"
#include "wrec.h"
#include "parse_cmd_ln.h"
#include <s3/prim_type.h>
#include <s3/cmd_ln.h>
#include <s3/err.h>
int Num_New_Phones,  Sil_Index, Num_Phones;
char *Sent_Dir[512];
int Num_Sent_Dir=1;
struct phone *Phone, *New_Phone;

int Num_Words;

struct word *Word;
int Delete = 0, nSquare=0, Verbose=0;
int Single_Path = 0;
int Find_Example = 0;
char wsj1 = 0;

int main (int argc, char* argv[])
{
  /*  char sw;*/
  char* base_file;
  char* phone_file;
  char* in_ctl_file;
  char* log_file;
  char** sentdirs;

  base_file = phone_file = in_ctl_file = log_file = NULL;

  float elapsed_user_time(), time;

  parse_cmd_ln(argc,argv);

  Num_Sent_Dir = 1;
  
  Verbose= *(uint32 *) cmd_ln_access("-v");
  nSquare= *(uint32 *) cmd_ln_access("-q");
  base_file = (char *) cmd_ln_access("-b");
  log_file = (char *) cmd_ln_access("-o");
  sentdirs = (char **) cmd_ln_access("-s");


  if(cmd_ln_access("-p")&&cmd_ln_access("-P"))
    E_FATAL("Please specify either -p or -P for the phone list\n");
  if(cmd_ln_access("-p"))
    phone_file=(char*) cmd_ln_access("-p");
  if(cmd_ln_access("-P"))
    phone_file=(char*) cmd_ln_access("-P");


  if(cmd_ln_access("-i")&&cmd_ln_access("-I"))
    E_FATAL("Please specify either -i or -I for the input control file\n");

  if(cmd_ln_access("-i"))
    in_ctl_file=(char*) cmd_ln_access("-i");
  if(cmd_ln_access("-I"))
    in_ctl_file=(char*) cmd_ln_access("-I");

  Find_Example = *(uint32 *) cmd_ln_access("-f");
  Single_Path = *(uint32 *) cmd_ln_access("-S");

  if(phone_file==NULL)
    E_FATAL("Must specify a phone list file -p \n");

  if(base_file==NULL)
    E_FATAL("Must specify a dictionary file -b \n");

  if(log_file==NULL)
    E_FATAL("Must specify a log file -o \n");

  if(!nSquare){
    E_FATAL("Currently -q is mandatory\n");
  }

  unlimit ();

  read_phone (phone_file);	/* Read in cxt-indep phones */
  Num_Words = read_base (base_file, &Word);
  printf ("Num_Phones = %d, Num_Words = %d\n", Num_Phones, Num_Words);

  if((Sil_Index = find_word_phone_index ("SIL"))<0)
    E_FATAL("Fail, could not find phone SIL in the phone list (specified by -p of QUICK_COUNT) \n");

  printf ("Sil_Index = %d\n", Sil_Index);
  elapsed_user_time (1);
  if (Find_Example) find_example(in_ctl_file);
  else {
    if (nSquare) all_word_pairs();
    else {
      if (Single_Path)
	s_make_triphone (in_ctl_file);
      else make_triphone (in_ctl_file);
    }
    write_phone (log_file);
  }
  time = elapsed_user_time (1);
  printf("used %10.2f sec\n", time);
  return 0;
}


