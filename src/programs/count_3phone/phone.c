/* ====================================================================
 * Copyright (c) 1990-2000 Carnegie Mellon University.  All rights 
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
/* The io routines for the HMM program.

   Kai-Fu Lee
 */
#define NOISE_WORD(X)  ((X)[0] == '+' && (X)[1] == '+')
#include <stdio.h>
#include <stdlib.h>
#include "wrec.h"
#define MAXPHONES          80000
#define PREV	1
#define MAX_WORDS_PER_SENTENCE 4098
#define SUCC	2
#define CXT_INDEP	80
/* max # of context indep phones */
#define WITHIN	0
#define BEGINNING	1
#define ENDING	2
#define SINGLE	3
static char *typestr[] = {"", "b", "e", "s"};

extern char wsj1;
extern int Num_Phones, Num_New_Phones, Sil_Index, Min_Freq, Verbose;
extern struct phone *Phone, *New_Phone;
extern char *Sent_Dir[512];
extern int Num_Words, Num_Sent_Dir;
extern struct word *Word;
void add_one_phone (int this, int prev, int next,int type);

int Sil_Word;
int occurred[CXT_INDEP][CXT_INDEP][CXT_INDEP][4];

int make_triphone (in_ctl_file)
char *in_ctl_file;
{
  FILE *fp, *sent_fp;
  char file_head[512], *rindex(), *slash_ptr, sent_name[512], sent_file[512];
  int  g, h, this, new, len, i, j, wd[MAX_WORDS_PER_SENTENCE], prev, back, no, ii;
  int tt;
  short *this_phones;
  char thisw[512];


  if (!(fp = fopen(in_ctl_file, "r")))
    quit(-1, "Cannot open %s.\n", in_ctl_file);
  if ((Sil_Word = find_word_index ("SIL", Word, Num_Words)) < 0)
    quit(1, "cannot find word SIL\n");
  printf ("Sil_Word = %d\n", Sil_Word);
  fflush (stdout);
  New_Phone = (struct phone *) malloc (MAXPHONES * sizeof (struct phone));
  if (New_Phone == NULL)
    quit (-1, "cannot allocate New_Phone");
  for (i = 0; i < Num_Phones; i++)
    New_Phone[i] = Phone[i];

  if (i > CXT_INDEP) quit(1, "occurred[] too samll\n");
  Num_New_Phones = i;
  printf ("#. of cxt-indep phones = %d\n", i);
  fflush(stdout);
  for (j = 0; j < i; j++)
    for (g = 0; g < i; g++)
      for (h = 0; h < i; h++)
        for (new = 0; new < 4; new++)
	  occurred[j][g][h][new] = 0;

  while (fscanf(fp, "%s", file_head) != EOF)
  { 
    if (!(slash_ptr = rindex(file_head, '/')))
      quit(1, "read_sent(): no slash in %s\n", file_head);
    if (wsj1) while (*(--slash_ptr) != '/');
    strcpy (sent_name, slash_ptr + 1);
    len = strlen (sent_name);
    if (sent_name[len - 1] == 'b' && sent_name[len - 2] == '-')
      sent_name[len - 2] = '\0';
for (tt=0; tt < Num_Sent_Dir; tt++)
{
    sprintf(sent_file, "%s/%s.sent", Sent_Dir[tt], sent_name);
    if ((sent_fp = fopen(sent_file, "r")) != NULL) break;
}
if (tt >= Num_Sent_Dir)
{
      printf("read_sent(): cannot open %s\n", sent_file);
	continue;
}

    wd[0] = Sil_Word; 
    for (i = 1; fscanf (sent_fp, "%s", thisw) != EOF; i++)
    {
      if ((wd[i] = find_word_index (thisw, Word, Num_Words)) < 0)
      {
        printf("cannot find word %s in sent %s\n", thisw,  sent_file);
	exit(-1); 
      }
      if (NOISE_WORD (thisw))
	wd[i] = Sil_Word;
    } 

    fclose (sent_fp);

    wd[i] = Sil_Word;
    no = i;
    for (ii = 1; ii < no; ii++)
    {
      prev = wd[ii-1];
      this = wd[ii];
      back = wd[ii+1];
      if (this == Sil_Word) continue;
      this_phones = Word[this].phone;

      if (Word[this].num_phones == 1)
      {
        g = Word[prev].phone[Word[prev].num_phones-1];
        new = this_phones[0];
        h = Word[back].phone[0];
        if ( !occurred[new][g][h][SINGLE] )
	  add_one_phone (new, g, h, SINGLE);
	else occurred[new][g][h][SINGLE]++;

        if (prev != Sil_Word)
        {
          g = Word[Sil_Word].phone[Word[Sil_Word].num_phones-1];
          new = this_phones[0];
          h = Word[back].phone[0];
          if ( !occurred[new][g][h][SINGLE] )
	    add_one_phone (new, g, h, SINGLE);
	  else occurred[new][g][h][SINGLE]++;
	}

        if (back != Sil_Word)
        {
          g = Word[prev].phone[Word[prev].num_phones-1];
          new = this_phones[0];
          h = Word[Sil_Word].phone[0];
          if ( !occurred[new][g][h][SINGLE] )
	    add_one_phone (new, g, h, SINGLE);
	  else occurred[new][g][h][SINGLE]++;
	}

        if (prev != Sil_Word && back != Sil_Word)
        {
          g = Word[Sil_Word].phone[Word[Sil_Word].num_phones-1];
          new = this_phones[0];
          h = Word[Sil_Word].phone[0];
          if ( !occurred[new][g][h][SINGLE] )
	    add_one_phone (new, g, h, SINGLE);
	  else occurred[new][g][h][SINGLE]++;
	}
        continue;
      }

    /* "this" consists of at least 2 phones */

      g = Word[prev].phone[Word[prev].num_phones-1];
      new = this_phones[0];
      h = this_phones[1];
      if ( !occurred[new][g][h][BEGINNING] )
	add_one_phone (new, g, h, BEGINNING);
      else occurred[new][g][h][BEGINNING]++;

      if (prev != Sil_Word)
      {
        g = Word[Sil_Word].phone[Word[Sil_Word].num_phones-1];
        new = this_phones[0];
	h = this_phones[1];
        if ( !occurred[new][g][h][BEGINNING] )
	  add_one_phone (new, g, h, BEGINNING);
	else occurred[new][g][h][BEGINNING]++;
      }

      g = this_phones[Word[this].num_phones - 2];
      new = this_phones[Word[this].num_phones - 1];
      h = Word[back].phone[0];
      if ( !occurred[new][g][h][ENDING] )
	add_one_phone (new, g, h, ENDING);
      else occurred[new][g][h][ENDING]++;

      if (back != Sil_Word)
      {
        g = this_phones[Word[this].num_phones - 2];
        new = this_phones[Word[this].num_phones - 1];
        h = Word[Sil_Word].phone[0];
        if ( !occurred[new][g][h][ENDING] )
	  add_one_phone (new, g, h, ENDING);
	else occurred[new][g][h][ENDING]++;
      }


      j = Word[this].num_phones - 1;
      for (i = 1; i < j; i++)
      {
        g = this_phones[i-1];
        new = this_phones[i];
        h = this_phones[i+1];
        if ( !occurred[new][g][h][WITHIN] )
	  add_one_phone (new, g, h, WITHIN);
        else occurred[new][g][h][WITHIN]++;
      } 
    }
  } 
  fclose (fp);
  return 0;
}

int make_bi_triphone (bi_file)
char *bi_file;
{
printf("not implemented yet\n");
exit(0);
}


/* Sort phones by:
	real_phones first (already should be)
	context-dep. phones should be sorted by their real phone index,
 */

int phn_cmp (ph1, ph2)
struct phone *ph1, *ph2;
{
  if (ph1 -> real_phone != ph2 -> real_phone)
    return (ph1 -> real_phone - ph2 -> real_phone);
  else
    return (strcmp (ph1 -> name, ph2 -> name));
}


int write_phone (file)
char *file;
{
  int i, this, prev, next, type;
  FILE *fp;

  fp = fopen (file, "w");
  if (fp == NULL)
    quit (-1, "Cannot open file <%s> to write.\n", file);

  qsort (&New_Phone[Num_Phones], Num_New_Phones - Num_Phones, 
	 sizeof (struct phone), phn_cmp);

  for (i = Num_Phones; i < Num_New_Phones; i++)
  {
    this = New_Phone[i].real_phone;
    prev = New_Phone[i].word_index;
    next = New_Phone[i].pinw;
    type = New_Phone[i].output_phone;

fprintf(fp, "%-20s	%d\n", New_Phone[i].name, 
	occurred[this][prev][next][type]);
  }
  fclose (fp);
  return 0;
}


void all_word_pairs()
{
  int  tt, g, h, new, w1, w2, w3;
  short *this_phones;

  if ((Sil_Word = find_word_index ("SIL", Word, Num_Words)) < 0)
    quit(1, "cannot find word SIL\n");
  printf ("Sil_Word = %d\n", Sil_Word);
  fflush (stdout);
  New_Phone = (struct phone *) malloc (MAXPHONES * sizeof (struct phone));
  if (New_Phone == NULL)
    quit (-1, "cannot allocate New_Phone");
  for (w1 = 0; w1 < Num_Phones; w1++)
    New_Phone[w1] = Phone[w1];

  if (w1 > CXT_INDEP) quit(1, "occurred[] too samll\n");
  Num_New_Phones = w1;
  printf ("#. of cxt-indep phones = %d\n", w1);
  fflush(stdout);
  for (new = 0; new < w1; new++)
    for (g = 0; g < w1; g++)
      for (h = 0; h < w1; h++)
        for (w2 = 0; w2 < 4; w2++)
	  occurred[new][g][h][w2] = 0;

  for (w1=0; w1 < Num_Words; w1++)
  {
    if (Word[w1].num_phones == 0) continue;
    if (w1 == Sil_Word || NOISE_WORD(Word[w1].word)) continue;
    this_phones = Word[w1].phone;
    if (Word[w1].num_phones == 1 && this_phones[0] == Sil_Index) continue;

    /* within-word triphones */
    for (tt=1; tt < Word[w1].num_phones-1; tt++)
    {
      g = this_phones[tt-1];
      new = this_phones[tt];
      h = this_phones[tt+1];
      if (! occurred[new][g][h][WITHIN])
	add_one_phone (new, g, h, WITHIN);
    }

    /* single-phone words */
    if (Word[w1].num_phones == 1) 
    {
      new = this_phones[0];
      if (! occurred[new][Sil_Index][Sil_Index][SINGLE])
	add_one_phone (new, Sil_Index, Sil_Index, SINGLE);

      for (w2 = 0; w2 < Num_Words; w2++)
      {
	if (Word[w2].num_phones == 0) continue;
	if (w2 == Sil_Word || NOISE_WORD(Word[w2].word)) continue;
	/* w1 --> w2 */
	g = Word[w2].phone[0];
	if (! occurred[new][Sil_Index][g][SINGLE])
	  add_one_phone (new,  Sil_Index,  g, SINGLE);

	/* w2  -> w1 */
	g = Word[w2].phone[Word[w2].num_phones-1];
	if (! occurred[new][g][Sil_Index][SINGLE])
	  add_one_phone (new, g, Sil_Index, SINGLE);

	/* w2 -> w1 -> w3 */
	for (w3 = 0; w3 < Num_Words; w3++)
	{
	  if (Word[w3].num_phones == 0) continue;
	  if (w3 == Sil_Word || NOISE_WORD (Word[w3].word)) continue;
	  h = Word[w3].phone[0];
	  if (! occurred[new][g][h][SINGLE])
	    add_one_phone (new, g, h, SINGLE);
	}
      }
      continue;  /* next w1 */
    }

    /* more than one phone */
    /* beginning phone */
    new = this_phones[0];
    h = this_phones[1];
    if (! occurred[new][Sil_Index][h][BEGINNING])
      add_one_phone (new, Sil_Index, h, BEGINNING);
    for (w2 = 0;  w2 < Num_Words; w2++)
    {
      if (Word[w2].num_phones == 0) continue;
      if (w2 == Sil_Word || NOISE_WORD(Word[w2].word)) continue;
      g = Word[w2].phone[Word[w2].num_phones-1];
      if (! occurred[new][g][h][BEGINNING])
	add_one_phone (new, g, h, BEGINNING);
    }
    /* ending phone */
    new = this_phones[Word[w1].num_phones-1];
    g = this_phones[Word[w1].num_phones-2];
    if (! occurred[new][g][Sil_Index][ENDING])
      add_one_phone (new, g, Sil_Index, ENDING);
    for (w2 = 0; w2 < Num_Words; w2++)
    {
      if (Word[w2].num_phones == 0) continue;
      if (w2 == Sil_Word || NOISE_WORD(Word[w2].word)) continue;
      h = Word[w2].phone[0];
      if (! occurred[new][g][h][ENDING])
	add_one_phone (new, g, h, ENDING);
    }
  }
}


void add_one_phone (int this, int prev, int next,int type)
{
  char new_phone[512];
  int g;

  occurred[this][prev][next][type] = 1;
  sprintf(new_phone, "%s(%s,%s)%s", Phone[this].name,
    Phone[prev].name, Phone[next].name, typestr[type]);
  g = Num_New_Phones++;
  strcpy (New_Phone[g].name, new_phone);
  New_Phone[g].word_index = prev;
  New_Phone[g].pinw = next;
  New_Phone[g].real_phone = this;
  New_Phone[g].output_phone = type;
  if (g >= MAXPHONES)
    quit(1, "make_triphone(): Num_New_Phones = %d is too big",
                     Num_New_Phones);
  if (Verbose && (Num_New_Phones % 1000 == 0))
  {
    printf("%d phones generated...\n", Num_New_Phones);
    fflush(stdout);
  }
}


/* single_path */
void s_make_triphone (in_ctl_file)
char *in_ctl_file;
{
  FILE *fp, *sent_fp=0;
  char file_head[512], *rindex(), *slash_ptr, sent_name[512], sent_file[512];
  int  g, h, this, new, len, i, j, wd[MAX_WORDS_PER_SENTENCE], prev, back, no, ii;
  int tt;
  short *this_phones;
  char thisw[512];


  if (!(fp = fopen(in_ctl_file, "r")))
    quit(-1, "Cannot open %s.\n", in_ctl_file);
  if ((Sil_Word = find_word_index ("SIL", Word, Num_Words)) < 0)
    quit(1, "cannot find word SIL\n");
  printf ("Sil_Word = %d\n", Sil_Word);
  fflush (stdout);
  New_Phone = (struct phone *) malloc (MAXPHONES * sizeof (struct phone));
  if (New_Phone == NULL)
    quit (-1, "cannot allocate New_Phone");
  for (i = 0; i < Num_Phones; i++)
    New_Phone[i] = Phone[i];

  if (i > CXT_INDEP) quit(1, "occurred[] too samll\n");
  Num_New_Phones = i;
  printf ("#. of cxt-indep phones = %d\n", i);
  fflush(stdout);
  for (j = 0; j < i; j++)
    for (g = 0; g < i; g++)
      for (h = 0; h < i; h++)
        for (new = 0; new < 4; new++)
	  occurred[j][g][h][new] = 0;

  while (fscanf(fp, "%s", file_head) != EOF)
  { 
    if (!(slash_ptr = rindex(file_head, '/')))
      quit(1, "read_sent(): no slash in %s\n", file_head);
    strcpy (sent_name, slash_ptr + 1);
    len = strlen (sent_name);
    if (sent_name[len - 1] == 'b' && sent_name[len - 2] == '-')
      sent_name[len - 2] = '\0';
for (tt=0; tt < Num_Sent_Dir; tt++)
{
    sprintf(sent_file, "%s/%s.sent", Sent_Dir[tt], sent_name);
    if ((sent_fp = fopen(sent_file, "r")) != NULL) break;
}
if (tt >= Num_Sent_Dir)
      quit(1, "read_sent(): cannot open %s\n", sent_file);

    for (i = 0; fscanf (sent_fp, "%s", thisw) != EOF; i++)
    {
      if ((wd[i] = find_word_index (thisw, Word, Num_Words)) < 0)
        quit (-1, "cannot find word %s in sent %s", thisw,  sent_file);
      if (NOISE_WORD (thisw))
	wd[i] = Sil_Word;
    } 
    fclose (sent_fp);
    if (wd[0] != Sil_Word)
      quit(-1, "%s not begin with SIL\n", file_head);
    if (wd[i-1] != Sil_Word)
      quit(-1, "%s not end with SIL\n", file_head);

    no = i;
    for (ii = 1; ii < no; ii++)
    {
      this = wd[ii];
      if (this == Sil_Word) continue;

      prev = wd[ii-1];
      back = wd[ii+1];
      this_phones = Word[this].phone;

      if (Word[this].num_phones == 1)
      {
        g = Word[prev].phone[Word[prev].num_phones-1];
        new = this_phones[0];
        h = Word[back].phone[0];
        if ( !occurred[new][g][h][SINGLE] )
	  add_one_phone (new, g, h, SINGLE);
	else occurred[new][g][h][SINGLE]++;
        continue;
      }

    /* "this" consists of at least 2 phones */

      g = Word[prev].phone[Word[prev].num_phones-1];
      new = this_phones[0];
      h = this_phones[1];
      if ( !occurred[new][g][h][BEGINNING] )
	add_one_phone (new, g, h, BEGINNING);
      else occurred[new][g][h][BEGINNING]++;

      g = this_phones[Word[this].num_phones - 2];
      new = this_phones[Word[this].num_phones - 1];
      h = Word[back].phone[0];
      if ( !occurred[new][g][h][ENDING] )
	add_one_phone (new, g, h, ENDING);
      else occurred[new][g][h][ENDING]++;

      j = Word[this].num_phones - 1;
      for (i = 1; i < j; i++)
      {
        g = this_phones[i-1];
        new = this_phones[i];
        h = this_phones[i+1];
        if ( !occurred[new][g][h][WITHIN] )
	  add_one_phone (new, g, h, WITHIN);
        else occurred[new][g][h][WITHIN]++;
      } 
    }
  } 
  fclose (fp);
}
