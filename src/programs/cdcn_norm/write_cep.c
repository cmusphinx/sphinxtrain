/* ====================================================================
 * Copyright (c) 1989-2005 Carnegie Mellon University.  All rights 
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/file.h>
#endif

/* Just in case O_BINARY isn't defined. */
#if !defined(O_BINARY)
#define O_BINARY 0
#endif

/* Macro to byteswap an int variable.  x = ptr to variable */
#define SWAP_INT(x)   *(x) = ((0x000000ff & (*(x))>>24) | \
                                (0x0000ff00 & (*(x))>>8) | \
                                (0x00ff0000 & (*(x))<<8) | \
                                (0xff000000 & (*(x))<<24))
/* Macro to byteswap a float variable.  x = ptr to variable */
#define SWAP_FLOAT(x) SWAP_INT((int *) x)


int write_cep(char *cepfilename, float **cep, int nframes, int numcep)
{
	int fcep, i;
	int length, wrbytes;
	float *X;

	X = cep[0];

	if ((fcep =
	     open(cepfilename, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
		  0644)) < 0) {
		printf("Unable to open %s for writing\n", cepfilename);
		return (-1);
	}

	length = nframes * numcep;
	SWAP_INT(&length);
	if (write(fcep, &length, 4) != 4) {
		printf("Data write error on %s\n", cepfilename);
		close(fcep);
		return (-1);
	}
	SWAP_INT(&length);

	for (i = 0; i < length; ++i)
		SWAP_FLOAT(cep[0] + i);
	wrbytes = length * sizeof(float);
	if (write(fcep, cep[0], wrbytes) != wrbytes) {
		printf("Data write error on %s\n", cepfilename);
		close(fcep);
		return (-1);
	}
	for (i = 0; i < length; ++i)
		SWAP_FLOAT(cep[0] + i);
	close(fcep);
	printf("Wrote %d cepstral vectors of length %d in %s\n",
	       nframes, numcep, cepfilename);
	return (length);
}
