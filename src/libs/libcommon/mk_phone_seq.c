/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: mk_phone_seq.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/


#include <s3/mk_phone_seq.h>

#include <s3/mk_wordlist.h>
#include <s3/mk_phone_list.h>
#include <s3/cvt2triphone.h>
#include <s3/corpus.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>

#include <stdio.h>

int
mk_phone_seq(acmod_id_t **out_phone,
	     uint32 *out_n_phone,
	     char *trans,
	     acmod_set_t *acmod_set,
	     lexicon_t *lex)
{
    char **word;
    uint32 n_word;
    uint32 n_phone;
    acmod_id_t *phone;
    char *btw_mark;

    word = mk_wordlist(trans, &n_word);
    phone = mk_phone_list(&btw_mark, &n_phone, word, n_word, lex);
    if (phone == NULL) {
	E_WARN("Unable to produce CI phones for utt %s.\n", corpus_utt());
    }

    cvt2triphone(acmod_set, phone, btw_mark, n_phone);

    ckd_free(btw_mark);
    ckd_free(word);

    *out_phone = phone;
    *out_n_phone = n_phone;

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.2  2000/09/29  22:35:13  awb
 * *** empty log message ***
 * 
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
