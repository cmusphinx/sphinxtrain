/*
	30 May 1989 David R. Fulmer (drf) updated to do byte order
		conversions when necessary.
 */

#include <s2/byteorder.h>
#include <s3/s2io.h>

#include <sys_compat/file.h>

#include <stdio.h>
#include <stdlib.h>

int
areadint (char *file,
	  int **data_ref,
	  int *length_ref)
{
  int             fd;
  int             length;
  int             size;
  int             offset;
  char           *data;

  if ((fd = open (file, O_RDONLY, 0644)) < 0)
  {
    fprintf (stderr, "areadint: %s: can't open\n", file);
    return -1;
  }
  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "areadint: %s: can't read length (empty file?)\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&length);
  size = length * sizeof (int);
  if (!(data = malloc ((unsigned) size)))
  {
    fprintf (stderr, "areadint: %s: can't alloc data\n", file);
    close (fd);
    return -1;
  }
  if (read (fd, data, size) != size)
  {
    fprintf (stderr, "areadint: %s: can't read data\n", file);
    close (fd);
    free (data);
    return -1;
  }
  close (fd);
  *data_ref = (int *) data;
  for(offset = 0; offset < length; offset++)
    SWAPL(*data_ref + offset);
  *length_ref = length;
  return length;
}
