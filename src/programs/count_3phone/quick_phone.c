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
/* The io routines for the HMM program.

   Kai-Fu Lee
 */
#define NOISE_WORD(X)  ((X)[0] == '+' && (X)[1] == '+')
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wrec.h"
#include "count.h"
#define MAXPHONES          500000
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

void add_one_phone (int this,int prev,int next,int type);

extern char wsj1;
extern int Num_Phones, Num_New_Phones, Sil_Index, Min_Freq, Verbose;
extern struct phone *Phone, *New_Phone;
extern char *Sent_Dir[512];
extern int Num_Words, Num_Sent_Dir;
extern struct word *Word;

int Sil_Word;
int occurred[CXT_INDEP][CXT_INDEP][CXT_INDEP][4];

void make_triphone (char *in_ctl_file)
{
  FILE *fp, *sent_fp;
  char file_head[512], *rindex(), *slash_ptr, sent_name[512], sent_file[512];
  int  g, h, this, new=0, len, i, j, wd[MAX_WORDS_PER_SENTENCE], prev, back, no, ii;
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
    sent_fp = 0;
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
}


void find_example (char *in_ctl_file)
{
  FILE *fp, *sent_fp;
  char file_head[512], *rindex(), *slash_ptr, sent_name[512], sent_file[512];
  int  g, h, this, new=0, len, i, j, wd[MAX_WORDS_PER_SENTENCE], prev, back, no, ii;
  int tt;
  char new_phone[512];
  short *this_phones;
  char thisw[512];
  int first_context_phone;
  char *ptr1;

  printf("find examples only\n");
  fflush(stdout);;

  for (first_context_phone=0; first_context_phone < Num_Phones;
	 first_context_phone++)
    if (Phone[first_context_phone].real_phone != first_context_phone)
	 break;
  if (first_context_phone == Num_Phones)
	quit(-1, "no cd phones in the phone file\n");
  if (first_context_phone > CXT_INDEP) quit(1, "occurred[] too samll\n");

  if (!(fp = fopen(in_ctl_file, "r")))
    quit(-1, "Cannot open %s.\n", in_ctl_file);
  if ((Sil_Word = find_word_index ("SIL", Word, Num_Words)) < 0)
    quit(1, "cannot find word SIL\n");
  printf ("Sil_Word = %d\n", Sil_Word);
  fflush (stdout);

  printf ("#. of cxt-indep phones = %d\n", first_context_phone);
  fflush(stdout);
  for (j=0; j < first_context_phone; j++)
    for (g=0; g < first_context_phone; g++)
      for (h=0; h < first_context_phone; h++)
	for (new=0; new < 4; new++)
	  occurred[j][g][h][new] = 0;

  for (i=first_context_phone;  i < Num_Phones; i++)
  {
    for (j=0, ptr1 = Phone[i].name;  *ptr1 != '('; ptr1++, j++)
      thisw[j] = *ptr1;
    thisw[j] = '\0';
    if ( (j=find_word_phone_index(thisw)) < 0)
	quit(-1, "cannot find component 1 in %s\n", Phone[i].name);

    for (g=0, ptr1++;  *ptr1 != ',';  ptr1++, g++)
      thisw[g] = *ptr1;
    thisw[g] = '\0';
    if ( (g=find_word_phone_index(thisw)) < 0)
        quit(-1, "cannot find component 2 in %s\n", Phone[i].name);

    for (h=0, ptr1++;  *ptr1 != ')'; ptr1++, h++)
      thisw[h] = *ptr1;
    thisw[h] = '\0';
    if ( (h=find_word_phone_index(thisw)) < 0)
        quit(-1, "cannot find component 3 in %s\n", Phone[i].name);

    ptr1++;
    if (*ptr1 == '\0') new = WITHIN;
    else if (*ptr1 == 'b') new = BEGINNING;
    else if (*ptr1 == 'e') new = ENDING;
    else if (*ptr1 == 's') new = SINGLE;
    else quit(-1, "unknow type %s\n", Phone[i].name);

    occurred[j][g][h][new] = 1;
  }

  printf("triphones which have different word postions:\n");
  for (j=0; j < first_context_phone; j++)
    for (g=0; g < first_context_phone; g++)
      for (h=0; h < first_context_phone; h++)
      {
	len = 0;
        for (new=0; new < 4; new++)
	  if (occurred[j][g][h][new]) len++;
        if (len > 1) {
	  for (new=0; new < 4; new++)
	    if (occurred[j][g][h][new]) {
	/* WITHIN triphones are easily to be located from the dic.
	   so only look for btw triphone examples */
	        if (new != WITHIN) occurred[j][g][h][new] |= 2;
		sprintf(new_phone, "%s(%s,%s)%s",
		  Phone[j].name, Phone[g].name, Phone[h].name,
			typestr[new]);
		len = find_word_phone_index(new_phone);
	        printf("%s ", Phone[len].name);
	    }
	  printf("\n");
	}
      }
  printf("\n");
  fflush(stdout);

  while (fscanf(fp, "%s", file_head) != EOF)
  { 
    if (!(slash_ptr = rindex(file_head, '/')))
      quit(1, "read_sent(): no slash in %s\n", file_head);
    strcpy (sent_name, slash_ptr + 1);
    len = strlen (sent_name);
    if (sent_name[len - 1] == 'b' && sent_name[len - 2] == '-')
      sent_name[len - 2] = '\0';
    sent_fp = 0;
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
        if ( occurred[new][g][h][SINGLE] & 2 ) 
	  printf("%s: %s(%s,%s)s\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);

        if (prev != Sil_Word)
        {
          g = Word[Sil_Word].phone[Word[Sil_Word].num_phones-1];
          new = this_phones[0];
          h = Word[back].phone[0];
          if ( occurred[new][g][h][SINGLE] & 2)
	    printf("%s: %s(%s,%s)s\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);
	}

        if (back != Sil_Word)
        {
          g = Word[prev].phone[Word[prev].num_phones-1];
          new = this_phones[0];
          h = Word[Sil_Word].phone[0];
          if ( occurred[new][g][h][SINGLE] & 2)
	    printf("%s: %s(%s,%s)s\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);
	}

        if (prev != Sil_Word && back != Sil_Word)
        {
          g = Word[Sil_Word].phone[Word[Sil_Word].num_phones-1];
          new = this_phones[0];
          h = Word[Sil_Word].phone[0];
          if ( occurred[new][g][h][SINGLE] & 2)
	    printf("%s: %s(%s,%s)s\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);
	}
        continue;
      }

    /* "this" consists of at least 2 phones */

      g = Word[prev].phone[Word[prev].num_phones-1];
      new = this_phones[0];
      h = this_phones[1];
      if ( occurred[new][g][h][BEGINNING] & 2)
	    printf("%s: %s(%s,%s)b\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);

      if (prev != Sil_Word)
      {
        g = Word[Sil_Word].phone[Word[Sil_Word].num_phones-1];
        new = this_phones[0];
	h = this_phones[1];
        if ( occurred[new][g][h][BEGINNING] & 2)
	    printf("%s: %s(%s,%s)b\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);
      }

      g = this_phones[Word[this].num_phones - 2];
      new = this_phones[Word[this].num_phones - 1];
      h = Word[back].phone[0];
      if ( occurred[new][g][h][ENDING] & 2)
	    printf("%s: %s(%s,%s)e\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);

      if (back != Sil_Word)
      {
        g = this_phones[Word[this].num_phones - 2];
        new = this_phones[Word[this].num_phones - 1];
        h = Word[Sil_Word].phone[0];
        if ( occurred[new][g][h][ENDING] & 2)
	    printf("%s: %s(%s,%s)e\n", sent_name, Phone[new].name,
		Phone[g].name, Phone[h].name);
      }

/*********
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
********/

    } /* next word */
    fflush(stdout);
  }  /* next sent */
  fclose (fp);
}

/* take care of compound words, too */
void make_bi_triphone (char *bi_file)
{
  int  tt, g, h, new, w1, w2, w3, type;
  short *this_phones;
  float elapsed_user_time();
  FILE *fp;
  char **word_connect;
  char line[512], word[80], str1[80], str2[80];

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

  elapsed_user_time(1);
  /* within-word triphones first */
  for (w1=0; w1 < Num_Words; w1++)
  {
    if (Word[w1].num_phones <= 1) continue;
    this_phones = Word[w1].phone;
    for (tt=1; tt < Word[w1].num_phones-1; tt++)
    {
      new = this_phones[tt];
      if (new == -100 || new == Sil_Index) continue;  /* & */

      type = WITHIN;

      g = this_phones[tt-1];
      if (g == -100){ g = this_phones[tt-2]; type = BEGINNING;}

      h = this_phones[tt+1];
      if (h == -100) { h = this_phones[tt+2]; 
		       if (type == BEGINNING) type = SINGLE;
		       else type = ENDING;
		      }
      if (! occurred[new][g][h][type])
	add_one_phone (new, g, h, type);
    }
  }
  printf("within-word triphone finished in %g secs.\n", 
		elapsed_user_time(1));
  fflush(stdout);

  if ( (Sil_Word = find_word_index ("SIL", Word, Num_Words)) < 0)
	quit(1, "cannot find SIL in dic\n");
  printf("Sil_Word = %d\n", Sil_Word);

  /* btw triphones according to the bigram file */
  if ( (word_connect = (char **) malloc(Num_Words * sizeof(char *))) == NULL)
	quit(-1, "cannot allocate word_connect\n");

  for (h=0; h < Num_Words; h++) {
    if ( (word_connect[h] = (char *) malloc(Num_Words)) == NULL)
	quit(1, "cannot allocate word_connect[%d]\n", h);

    if (h == Sil_Word) type = 1;
    else type = 0;

    for (g=0; g < Num_Words; g++) 
	word_connect[h][g] = type;

    word_connect[h][Sil_Word] = 1;
  }

  if ( (fp=fopen(bi_file, "r")) == NULL)
	quit(1, "cannot open %s\n", bi_file);

  h = 0;
  while (fgets(line, sizeof(line)-1, fp) != NULL)
  {
    sscanf(line, "%s ", word);
    if (strcmp(word, "\\2-grams:") == 0) {h = 1; break;}
  }
  if (! h) quit(1, "cannot find \\2-grams: in %s\n", bi_file);

  h = 0;
  while (fgets(line, sizeof(line)-1, fp) != NULL)
  {
    sscanf(line, "%s %s %s ", word, str1, str2);
    if (strcmp(word, "\\end\\") == 0) {h=1; break;}

    if (strcmp(str1, "<s>") == 0 && strcmp(str2, "</s>") == 0)
	printf("warning: %s\n", line);

    if (strcmp(str1, "<s>") == 0 || strcmp(str2, "</s>") == 0) continue;
    if (strcmp(str1, "</s>") == 0 || strcmp(str2, "<s>") == 0) {
       printf("warning: %s\n", line);
       continue;
    }

    if ( (w1=find_word_index(str1, Word, Num_Words)) < 0)
	quit(1, "cannot find str1 %s in %s\n", str1, line);
    if ( (w2=find_word_index(str2, Word, Num_Words)) < 0)
	quit(1, "cannot find str2 %s in %s\n", str2, line);
    word_connect[w1][w2] = 1;
  }
  if (! h) printf("warning: \\end\\ not found in %s\n", bi_file);
  fclose(fp);
  printf("DARPA bigram read\n");
  fflush(stdout);

 /* btw triphones */
  for (w1=0; w1 < Num_Words; w1++)
  {
    if (NOISE_WORD(Word[w1].word) || w1 == Sil_Word) continue;

    tt = Word[w1].num_phones;
    this_phones = Word[w1].phone;
    if (tt == 1) {
      new = this_phones[0];
      for (w2=0; w2 < Num_Words; w2++)
        if (word_connect[w2][w1]) /* w2 includes SIL word */
        {
	  g = Word[w2].phone[Word[w2].num_phones-1];
          for (w3=0; w3 < Num_Words; w3++)
	    if (word_connect[w1][w3]) /* w3 includes SIL word */
	    {
	      h = Word[w3].phone[0];
	      if (! occurred[new][g][h][SINGLE])
		add_one_phone (new, g, h, SINGLE);
	    }
	}
     continue;
    }

    /* beginning phones */
    new = this_phones[0];
    h = this_phones[1];
    if (h == -100) { type = SINGLE; h = this_phones[2];}
    else type = BEGINNING;

    for (w2=0; w2 < Num_Words; w2++)
      if (word_connect[w2][w1])
      {
	g = Word[w2].phone[Word[w2].num_phones-1];
        if (! occurred[new][g][h][type])
	  add_one_phone (new, g, h, type);
      }

    /* ending phones */
    new = this_phones[tt-1];
    g = this_phones[tt-2];
    if (g == -100) { type = SINGLE; g = this_phones[tt-3];}
    else type = ENDING;

    for (w2=0; w2 < Num_Words; w2++)
      if (word_connect[w1][w2])
      {
	h = Word[w2].phone[0];
        if (! occurred[new][g][h][type])
          add_one_phone (new, g, h, type);
      }
  }
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


void write_phone (char *file)
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
}


/*  this modules take care of compound words */
void all_word_pairs()
{
  int  tt, g, h, new, w1, w2, first_cnt, last_cnt, type;
  short *this_phones;
  float elapsed_user_time();
  char *first_phone, *last_phone;

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

  elapsed_user_time(1);
  /* within-word triphones first */
  for (w1=0; w1 < Num_Words; w1++)
  {
    if (Word[w1].num_phones <= 1) continue;
    this_phones = Word[w1].phone;
    for (tt=1; tt < Word[w1].num_phones-1; tt++)
    {
      new = this_phones[tt];
      if (new == -100 || new == Sil_Index) continue;  /* & */

      type = WITHIN;

      g = this_phones[tt-1];
      if (g == -100){ g = this_phones[tt-2]; type = BEGINNING;}

      h = this_phones[tt+1];
      if (h == -100) { h = this_phones[tt+2]; 
		       if (type == BEGINNING) type = SINGLE;
		       else type = ENDING;
		      }
      if (! occurred[new][g][h][type])
	add_one_phone (new, g, h, type);
    }
  }
  printf("within-word triphone finished in %g secs.\n", 
		elapsed_user_time(1));
  fflush(stdout);

  /* single-phone words */
  first_phone = (char *) malloc(Num_Phones);
  last_phone = (char *) malloc(Num_Phones);
  for (w1=0; w1 < Num_Phones; w1++)
    first_phone[w1] = last_phone[w1] = 0;
  first_phone[Sil_Index] = last_phone[Sil_Index] = 1;
  for (w1=0; w1 < Num_Words; w1++)
  {
    if (Word[w1].num_phones == 0 || NOISE_WORD(Word[w1].word)) continue;
    this_phones = Word[w1].phone;
    first_phone[this_phones[0]] = 1;
    last_phone[this_phones[Word[w1].num_phones-1]] = 1;
  }    
  first_cnt = last_cnt = 0;
  for (g=0; g < Num_Phones; g++)
  {
    if (first_phone[g]) first_cnt++;
    if (last_phone[g]) last_cnt++;
  }
  printf("%d phones can begin a word\n", first_cnt);
  printf("%d phones can end a word\n", last_cnt);
  fflush(stdout);

  w2 = 0;
  for (w1=0; w1 < Num_Words; w1++)
  {
    this_phones = Word[w1].phone;
    if (Word[w1].num_phones != 1 || this_phones[0] == Sil_Index ||
	NOISE_WORD(Word[w1].word)) continue;

    w2++;
    new = this_phones[0];
    for (g=0; g < Num_Phones; g++)
      if (last_phone[g]) /* including SIL */
        for (h=0; h < Num_Phones; h++)
	  if (first_phone[h]) /* including SIL */
          {
	    if (! occurred[new][g][h][SINGLE])
	      add_one_phone (new, g, h,  SINGLE);
	  }
  }
  printf("%d single-phone words\n", w2);
  printf("*(*,*)s finished in %g secs\n",
		elapsed_user_time(1));
  fflush(stdout);

  /* b and e triphones */
  for (w1=0; w1 < Num_Words; w1++)
  {
    if (Word[w1].num_phones <= 1) continue;
    this_phones = Word[w1].phone;

    /* e triphones */
    type = ENDING;
    new = this_phones[Word[w1].num_phones-1];
    h = this_phones[Word[w1].num_phones-2];
    if (h == -100) { h = this_phones[Word[w1].num_phones-3];
		     type = SINGLE;
		   }
    for (g=0; g < Num_Phones; g++)
      if (first_phone[g]) /* including SIL */
      {
	if (!occurred[new][h][g][type])
	  add_one_phone (new, h, g, type);
      }

    /* b triphones */
    type = BEGINNING;
    new = this_phones[0];
    h = this_phones[1];
    if (h == -100) { h = this_phones[2];
		     type = SINGLE;
		   }
    for (g = 0; g < Num_Phones; g++)
      if (last_phone[g]) /* including SIL */
      {
        if (! occurred[new][g][h][type])
  	  add_one_phone (new, g, h, type);
      }
  }
  printf("b/e triphones finished in %g secs\n",
	elapsed_user_time(1));
}


void add_one_phone (int this,int prev,int next,int type)
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
void s_make_triphone (char *in_ctl_file)
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
