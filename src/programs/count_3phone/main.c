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
/* Make word-dependent triphones and dictinary
   from word-independent triphones and dictinary

   Mei-Yuh Hwang    Oct. 1988
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "count.h"
#include "wrec.h"
int Num_New_Phones,  Sil_Index, Num_Phones;
char *Sent_Dir[512];
int Num_Sent_Dir=1;
struct phone *Phone, *New_Phone;

int Num_Words;

struct word *Word;
int Min_Freq=1;
int Delete = 0, nSquare=0, Verbose=0;
int Single_Path = 0;
int Find_Example = 0;
char wsj1 = 0;


int main (argc, argv)
int argc;
char **argv;
{
  int i;
  char sw, phone_file[512], base_file[512], 
	*log_file=0,
	 in_ctl_file[512], bi_file[512], BI_FLAG;
  float elapsed_user_time(), time;

  Num_Sent_Dir = 1;
  Sent_Dir[0] = "/wordg/usr2/kfl/data/new_sent";
  strcpy (phone_file, "/usr/hmm/network/phone/base.phone");
  strcpy (base_file, "/usr/hmm/network/dic/base-dic");
  strcpy (bi_file, "");
  BI_FLAG = FALSE;
  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] != '-')
     quit (-1, "Unexpected argv <%s>\n", argv[i]);
    sw = argv[i][1];
    switch (sw)
    {
      case 'v':
	Verbose = 1;
	break;
      case 'q':
	nSquare = 1;
	break;
      case 't':
	Min_Freq = atoi (argv[++i]);
	break;
      case 'b':
	{
	  strcpy (base_file, argv[++i]);
	  break;
	}
      case 'B':
	{
	  strcpy (bi_file, argv[++i]); 
	  BI_FLAG = TRUE;
	  break;
	}      
      case 'p':		/* Phone file */
      case 'P': 
	{
	  strcpy (phone_file, argv[++i]);
	  break;
	}
      case 'o':
	log_file = argv[++i];
	break;
      case 'i':
      case 'I':
        {  
          strcpy (in_ctl_file, argv[++i]);
          break;
	}
      case 'f':
	Find_Example = 1;
	break;
      case 'S':
	Single_Path = 1;
	break;
      case 's':
        {
	  if (argv[i][2] == 's') wsj1 = 1;
	  Num_Sent_Dir = 0;
	  while ( (++i < argc) && argv[i][0] != '-')
            Sent_Dir[Num_Sent_Dir++] = argv[i];
	  --i;
          break;
	}
      default: 
        quit (-1, "Unknown flag %c.\n", sw);
    }
  }

  unlimit ();

  read_phone (phone_file);	/* Read in cxt-indep phones */
  Num_Words = read_base (base_file, &Word);
  printf ("Num_Phones = %d, Num_Words = %d\n", Num_Phones, Num_Words);
  Sil_Index = find_word_phone_index ("SIL");
  printf ("Sil_Index = %d\n", Sil_Index);
  elapsed_user_time (1);
  if (Find_Example) find_example(in_ctl_file);
  else {
    if (nSquare) all_word_pairs();
    else if (BI_FLAG)
    {
      printf ("Making triphones by bigram\n");
      make_bi_triphone (bi_file);    
    }
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


