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
 * 4. Redistributions of any form whatsoever must retain the following
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
/* Include file used by word HMM programs */
#ifndef WREC_H
#define WREC_H

#include "phrec.h"
#include <stdarg.h>

int quit (int status, char *fmt, ...);

#define FIND_INDEX		find_word_phone_index
#define CEP_EXT			"cep"
#define FEAT_EXT		"feat"
#define LOLA_EXT		"ptlola"
#define WORD_LOLA_EXT		"newlola"
#define CODE_EXT		"code"
#define WINDOW_WIDTH		20
#define ANALYSIS_STEP		10
#define LOLA_FRAME	 	3
#define MAX_CODEBOOKS		256
#define MAX_FRAMES		256
#define MIN_DOUBLE		1.0e-300
#define PHONE_FILE		"/usr/kfl/phrec/ctl/word.phone"
#define MAX_CLASSES		5
#define MAX_CONF		5
#define MAX_PHONES_PER_WORD	30
#define MAX_PHONE_STRING	30



struct phone_segment
{
  short identity, length;
  unsigned char *code;
};

struct word
{
  char *word;			/* The spelling of the word */
  int num_phones;		/* Number of phones in this word */
  short *phone;		/* Indices of each phone in this word */
};

#endif
