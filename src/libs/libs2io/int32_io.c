/*
 * 32-bit int I/O routines
 * 
 */

#include <s3/int32_io.h>

#include <stdio.h>

int32
read_int32 (FILE *stream)
{
  int             c;
  int32            word;

  c = getc (stream);
  if (c == EOF)
    return -1;
  word = c;
  c = getc (stream);
  if (c == EOF)
    return -1;
  word = word << 8 | c;
  c = getc (stream);
  if (c == EOF)
    return -1;
  word = word << 8 | c;
  c = getc (stream);
  if (c == EOF)
    return -1;
  return word << 8 | c;
}


int
write_int32 (FILE *stream,
	     int32 word)
{
  if (putc (word >> 24, stream) == EOF)
    return -1;
  if (putc (word >> 16, stream) == EOF)
    return -1;
  if (putc (word >> 8, stream) == EOF)
    return -1;
  if (putc (word, stream) == EOF)
    return -1;
  return 0;
}


int
read_int32_array (FILE *stream,
		  int32 *base,
		  int length)
{
#ifdef sun
  return fread ((char *) base, length * 4, 1, stream) != 1 ? -1 : 0;
#else
  int             counter;
  int32           *ptr;

  counter = length;
  ptr = base;
  while (--counter >= 0)
  {
    int             c;
    int32            word;

    c = getc (stream);
    if (c == EOF)
      return -1;
    word = c;
    c = getc (stream);
    if (c == EOF)
      return -1;
    word = word << 8 | c;
    c = getc (stream);
    if (c == EOF)
      return -1;
    word = word << 8 | c;
    c = getc (stream);
    if (c == EOF)
      return -1;
    *ptr++ = word << 8 | c;
  }
  return 0;
#endif
}


int
write_int32_array (FILE *stream,
		   int32 *base,
		   int length)
{
#ifdef sun
  return fwrite((char *) base, length * 4, 1, stream) != 1 ? -1 : 0;
#else
  int             counter;
  int32           *ptr;

  counter = length;
  ptr = base;
  while (--counter >= 0)
  {
    int32            word;

    word = *ptr++;
    if (putc (word >> 24, stream) == EOF)
      return -1;
    if (putc (word >> 16, stream) == EOF)
      return -1;
    if (putc (word >> 8, stream) == EOF)
      return -1;
    if (putc (word, stream) == EOF)
      return -1;
  }
  return 0;
#endif
}
