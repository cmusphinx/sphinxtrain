/*
	30 May 1989 David R. Fulmer (drf) updated to do byte order
		conversions when necessary.
 */

#include <s2/byteorder.h>
#include <s3/s2io.h>

#include <sys_compat/file.h>
#include <stdio.h>

int
awritefloat (char *file,
	     float *data,
	     int length)
{
  int             fd;
  int             size;
  int             offset;

  if ((fd = open (file, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
  {
    fprintf (stderr, "awritefloat: %s: can't create\n", file);
    perror("");
    return -1;
  }

  SWAPL(&length);
  if (write (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "awritefloat: %s: can't write length\n", file);
    perror("");
    close (fd);
    return -1;
  }
  SWAPL(&length);

  for(offset = 0; offset < length; offset++)
    SWAPF(data + offset);
  size = length * sizeof (float);
  if (write (fd, (char *) data, size) != size)
  {
    fprintf (stderr, "awritefloat: %s: can't write data\n", file);
    perror("");
    close (fd);
    return (-1);
  }
  for(offset = 0; offset < length; offset++)
    SWAPF(data + offset);

  printf ("Wrote %d floats in %s.\n", length, file);
  close (fd);
  return length;
}
