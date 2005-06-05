/* ====================================================================
 * Copyright (c) 1987-2000 Carnegie Mellon University.  All rights 
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
 * Revision 1.7  2005/06/05  22:00:34  arthchan2003
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

/* Functions for IO & access of phones and words.
   The following functions are used by SPHINX for training/recognition
   	(other functions are experimental):
	read_base () - read a baseform dictionary file.
	find_word_index () - find the index of a word in a dictionary.
	read_phone () - read a phone file.
	find_word_phone_index () - find the index of a phone.
    Kai-Fu Lee 10/87 created    Joe Keane (?) 8/88 added sorting.
    Kai-Fu Lee 8/88 added some documentation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "count.h"
#include <s3/err.h>


int new_read_base (char *base_file, struct word **words_ref, int ignore_error);
int find_word_phone_index (char *phone);

#define MAX_LINE 50000

struct sorted_phone
{
  int phone_index;
  char *name;
};
struct sorted_phone *Sorted_Phone;

#if (defined(__alpha) || defined(WIN32))
void *malloc();
#else
char *malloc();
#endif

extern int      Num_Phones, Num_Words;
extern struct phone *Phone;


int read_base (char *base_file, struct word **words_ref)
{
  return new_read_base (base_file, words_ref, FALSE);
}


int read_base2 (base_file, words_ref)
  char           *base_file;
  struct word   **words_ref;
{
  return new_read_base (base_file, words_ref, TRUE);
}


int new_read_base (char *base_file, struct word **words_ref, int ignore_error)
{
  FILE           *fp;
  int             num_words;
  struct word    *words;
  int             word_num;
  char            buff[MAX_LINE];

  if (!(fp = fopen (base_file, "r")))
    E_FATAL ("read_base: can't open dictionary file: %s\n", base_file);
  num_words = 0;
  while (fgets (buff, MAX_LINE - 1, fp))
    num_words++;
  if (!(words = (struct word *)
	malloc (num_words * (unsigned) sizeof (struct word))))
    E_FATAL ( "read_base: %s: can't alloc words\n", base_file);
  Num_Words = num_words;
  printf ("Total no. of words = %d\n", Num_Words);
  fflush(stdout);
  rewind (fp);
  for (word_num = 0; fgets (buff, MAX_LINE - 1, fp) != NULL; word_num++)
  {
    struct word    *word = &words[word_num];
    int             i,
                    last,
                    end,
                    num_phones;
    short           phones[MAX_PHONES_PER_WORD];

    for (i = 0; !isspace ((int)buff[i]); i++)
      ;
    if (!(word->word = (char *) malloc ((i + 1) * (unsigned) sizeof (char))))
      E_FATAL ( "read_base: %s: can't alloc words[%d]\n", base_file, word_num);
    strncpy (word->word, buff, i);
    word->word[i] = '\0';
    while (isspace ((int)buff[i]))
      i++;
    last = i;
    num_phones = 0;
    end = strlen (buff);
    for (; i <= end; i++)
      if (isspace ((int)buff[i]))
      {
	char            phone_name[MAX_PHONE_STRING];

	strncpy (phone_name, &buff[last], i - last);
	phone_name[i - last] = '\0';
     if (strcmp(phone_name, "&") == 0) {
	printf("Phrase %s\n", buff);
	fflush(stdout);
	phones[num_phones] = MAGIC_PHASE_SYMBOL;
     } else {
	phones[num_phones] = find_word_phone_index (phone_name);
	if (phones[num_phones] < 0)
	{
	  if (ignore_error)
	  {
	    int j;

	    for (j = i - last; j > 0; j--)
	      if (phone_name[j] == '(') break;
	    if (phone_name[j] == '(')
	    {
	      phone_name[j] = '\0';
	      phones[num_phones] = find_word_phone_index (phone_name);
	      if (phones[num_phones] < 0)
	        E_FATAL ( "read_base: %s: can't even find ci-phone `%s' in word `%s'(%d)\n",
		base_file, phone_name, word->word, word_num);
	    }
	    else
	      E_FATAL ( "read_base: %s: can't find phone `%s' in word `%s'(%d)\n",
		base_file, phone_name, word->word, word_num);
	  }
	  else
	    E_FATAL ( "read_base: %s: can't find phone `%s' in word `%s'(%d)\n",
		base_file, phone_name, word->word, word_num);
	}
     }

	while (isspace ((int)buff[i]))
	  i++;
	last = i;
	num_phones++;
      }
    if (num_phones && !(word->phone = (short *) malloc (num_phones *
					  (unsigned) sizeof (short))))
      E_FATAL ( "read_base: %s: can't alloc word->phone\n", base_file);
    word->num_phones = num_phones;
    for (i = 0; i < word->num_phones; i++)
      word->phone[i] = phones[i];
  }
  fclose (fp);
  *words_ref = words;
  return num_words;
}


/* Maps a word in sentence to one in lexicon.  Returns -1 if the word
   is not found. */

int find_word_index (char *word,struct word *dictionary,int num_words)
{
  int             lower = 0,
                  upper = num_words;

  while (lower < upper)
  {
    int             test = (lower + upper) / 2,
                    cmp = strcmp (dictionary[test].word, word);

    if (cmp < 0)
      lower = test + 1;
    else if (cmp > 0)
      upper = test;
    else
      return test;
  }
  return -1;
}

int spc (ph1, ph2)
struct sorted_phone *ph1, *ph2;
{
  return (strcmp (ph1 -> name, ph2 -> name));
}


void read_phone (char *file)
{
  FILE           *fp;
  int             i,
                  items;
  char            buff[512];

  if (!(fp = fopen (file, "r")))
    E_FATAL ( "read_phone: can't open phone list file %s\n", file);

  for (i = 0; fgets (buff, 79, fp) != NULL; i++);
  Num_Phones = i;
  if (!(Phone = (struct phone *) malloc (Num_Phones *
			 (unsigned) sizeof (struct phone))))
    E_FATAL ( "read_phone: %s: can't alloc %d phones\n", file, Num_Phones);
  if (!(Sorted_Phone = (struct sorted_phone *) malloc (Num_Phones *
			 (unsigned) sizeof (struct sorted_phone))))
    E_FATAL( "read_phone: %s: can't alloc %d sorted_phones\n", 
		file, Num_Phones);
  rewind (fp);
  for (i = 0;
       (items = fscanf (fp, "%s%hd%hd%hd%hd\n", Phone[i].name,
			&Phone[i].word_index, &Phone[i].pinw,
			&Phone[i].real_phone, &Phone[i].output_phone)) != EOF;
       i++)
  {
    if (items != 5)
      E_FATAL ("read_phone: %s: read %d items, should be 5\n", (char*)file, items);
    Sorted_Phone[i].phone_index = i;
    Sorted_Phone[i].name = Phone[i].name;
  }
  qsort (Sorted_Phone, Num_Phones, sizeof (struct sorted_phone), spc);
  fclose (fp);
}


int find_word_phone_index (char *phone)
{
  int             lower = 0,
                  upper = Num_Phones;

  while (lower < upper)
  {
    int             test = (lower + upper) / 2,
                    cmp = strcmp (Sorted_Phone[test].name, phone);

    if (cmp < 0)
      lower = test + 1;
    else if (cmp > 0)
      upper = test;
    else
      return Sorted_Phone[test].phone_index;
  }
  return -1;
}


#if 0
/* OLD STUFF

find_cont_phone_index (phone)
  char           *phone;
{
  int             i,
                  len = strlen (phone);
  char           *ptr;

  if (phone[len - 2] == '-')
    phone[len - 2] = '\0';
  if (phone[len - 1] == '?')
    phone[len - 1] = '\0';

  if ((strcmp (phone, "bcl") == 0) || (strcmp (phone, "dcl") == 0) ||
      (strcmp (phone, "gcl") == 0))
    strcpy (phone, "VOI");
  else if ((strcmp (phone, "pcl") == 0) || (strcmp (phone, "tcl") == 0) ||
	   (strcmp (phone, "kcl") == 0) || (strcmp (phone, "qcl") == 0))
    strcpy (phone, "-");
  else if ((strcmp (phone, "h#") == 0) || (strcmp (phone, "#h") == 0) ||
	   (strcmp (phone, "pau") == 0))
    strcpy (phone, "SIL");
  else if (strcmp (phone, "j") == 0)
    strcpy (phone, "jh");
  else if (strcmp (phone, "axr") == 0)
    strcpy (phone, "er");
  else if (strcmp (phone, "eng") == 0)
    strcpy (phone, "ng");
  else if (strcmp (phone, "hv") == 0)
    strcpy (phone, "hh");
  else if (strcmp (phone, "zh") == 0)
    strcpy (phone, "sh");
  else if (strcmp (phone, "nx") == 0)
    strcpy (phone, "n");
  else if (strcmp (phone, "ux") == 0)
    strcpy (phone, "uw");
  else if (strcmp (phone, "em") == 0)
    strcpy (phone, "m");

  for (ptr = phone; *ptr != '\0'; ptr++)
    if (islower (*ptr))
      *ptr = toupper (*ptr);
  for (i = 0; i < Num_Phones; i++)
    if (strcmp (Phone[i].name, phone) == 0)
      return i;
  return -1;
}

find_bob_phone_index (phone)
  char           *phone;
{
  int             i,
                  len;
  char           *ptr;

  len = strlen (phone);

  if (phone[len - 2] == '-')
    phone[len - 2] = '\0';
  if (phone[len - 1] == '?')
    phone[len - 1] = '\0';

  if ((strcmp (phone, "bcl") == 0) || (strcmp (phone, "dcl") == 0) ||
      (strcmp (phone, "gcl") == 0))
    strcpy (phone, "VOI");
  else if ((strcmp (phone, "pcl") == 0) || (strcmp (phone, "tcl") == 0) ||
	   (strcmp (phone, "kcl") == 0))
    strcpy (phone, "-");
  else if ((strcmp (phone, "h#") == 0) || (strcmp (phone, "#h") == 0) ||
	   (strcmp (phone, "pau") == 0))
    strcpy (phone, "SIL");
  else if (strcmp (phone, "axr") == 0)
    strcpy (phone, "er");
  else if (strcmp (phone, "eng") == 0)
    strcpy (phone, "ng");
  else if (strcmp (phone, "hv") == 0)
    strcpy (phone, "hh");
  else if (strcmp (phone, "zh") == 0)
    strcpy (phone, "sh");
  else if (strcmp (phone, "nx") == 0)
    strcpy (phone, "n");
  else if (strcmp (phone, "em") == 0)
    strcpy (phone, "m");

  for (ptr = phone; *ptr != '\0'; ptr++)
    if (islower (*ptr))
      *ptr = toupper (*ptr);
  for (i = 0; i < Num_Phones; i++)
    if (strcmp (Phone[i].name, phone) == 0)
      return i;
  return -1;
}

*/
#endif
