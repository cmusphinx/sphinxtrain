/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: cvt2triphone.c
 * 
 * Description: 
 * 	This file contains routines to convert a sequence of context
 *	independent phones into a sequence of triphones.
 *	
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <s3/cvt2triphone.h>

#include <s3/acmod_set.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

/*********************************************************************
 *
 * Function: btw_posn
 * 
 * Description: 
 *    This function takes a between word marker and a current
 *    word position state and generates the next word position
 *    state (see the description for cvt2triphone() for a
 *    discussion of between word markers).
 *
 * Function Inputs: 
 *    char btw_mark -
 * 	The value of the current between word marker.
 *
 *    word_posn_t posn -
 * 	The current word position state (i.e.
 *	word begin, word end, word internal, single
 *	phone word).
 *
 * Global Inputs: 
 *    None
 * 
 * Return Values: 
 *    The following table gives the return values for all
 *    possible values of the word_posn and btw_mark inputs.
 *
 *			word_posn
 *
 *    		BEGIN	INT	SINGLE	END
 *		------------------------------
 *    btw_mark:	T	END	END	SINGLE	SINGLE
 *	      :	F	INT	INT	BEGIN	BEGIN
 *
 *
 * Global Outputs: 
 *    None
 * 
 * Errors: 
 * 
 *********************************************************************/
word_posn_t btw_posn(char btw_mark,
		     word_posn_t posn)
{
    if (btw_mark) {
	if ((posn == WORD_POSN_INTERNAL) ||
	    (posn == WORD_POSN_BEGIN)) {
	    posn = WORD_POSN_END;
	}
	else if (posn == WORD_POSN_END) {
	    posn = WORD_POSN_SINGLE;
	}
	else if (posn == WORD_POSN_SINGLE)
	    ;	/* Don't change in this case */
	else {
	    E_FATAL("Unhandled word position\n");
	}
    }
    else {
	if (posn == WORD_POSN_BEGIN) {
	    posn = WORD_POSN_INTERNAL;
	} else if ((posn == WORD_POSN_END) ||
		   (posn == WORD_POSN_SINGLE)) {
	    posn = WORD_POSN_BEGIN;
	} else if (posn == WORD_POSN_INTERNAL)
	    ;	/* Don't change posn's value in this case */
	else {
	    E_FATAL("Unhandled word position\n");
	}
    }

    return posn;
}

/*********************************************************************
 *
 * Function: cvt2triphone
 * 
 * Description: 
 *    This routine takes a context indepent phone sequence and
 *    converts it into a triphone sequence.  The between word
 *    marker sequence, given as input, is used to determine the
 *    word position of the triphone (i.e. begin, end, internal or single
 *    phone).
 *
 *    For each context independent phone in the input sequence
 *    there is a between word marker.  It is a boolean value
 *    which is true when there is a word boundary after the
 *    associated phone.  For instance, if the word BAT is
 *    used in the original word string the corresponding
 *    phone string and associated between word marker
 *    sequence are:
 *
 *    phone:	...	B	AA	T	...
 *    marker:	...	FALSE	FALSE	TRUE	...
 * 
 * Function Inputs: 
 *    acmod_set_t *acmod_set -
 *	This is the data structure which allows the acmod_set
 *	module to return triphone id's
 * 
 *    acmod_id_t *phone -
 *	This is the sequence of context independent acoustic
 *	models corresponding to some word string.
 *
 *    char *btw_mark -
 * 	The between word markers for this CI model sequence.
 *
 *    char *multiw_mark -
 * 	Indicate whether the next word is the multiple pronounications of this word 
 *
 *    uint32 n_phone -
 *	The number of CI models in the sequence.
 * 
 * Global Inputs: 
 *    None
 *
 * Return Values: 
 *    None (this probably needs to be changed)
 *
 * Global Outputs: 
 *    None
 * 
 *********************************************************************/

int
cvt2triphone(acmod_set_t *acmod_set,
	     acmod_id_t *phone,
	     char *btw_mark,
	     char *multiw_mark,
	     uint32 n_phone,
	     uint32 multi_prons
	     )

{
    uint32 i;
    ci_acmod_id_t b = (ci_acmod_id_t)NO_ACMOD;
    ci_acmod_id_t l = (ci_acmod_id_t)NO_ACMOD;
    ci_acmod_id_t r = (ci_acmod_id_t)NO_ACMOD;
    acmod_id_t tri_id;
    word_posn_t posn;
    static int spoke_my_peace = FALSE;
    acmod_id_t sil = acmod_set_name2id(acmod_set, "SIL");
    char *word_posn_map = WORD_POSN_CHAR_MAP;
    ci_acmod_id_t last_l = (ci_acmod_id_t)NO_ACMOD;
    ci_acmod_id_t true_r = (ci_acmod_id_t)NO_ACMOD;
    
    uint32 k;

    if (acmod_set_n_multi(acmod_set) == 0) {
	/* nothing to do */

	if (!spoke_my_peace) {
	    fflush(stdout);
	    E_INFO("no multiphones defined, no conversion done\n");
	    spoke_my_peace = TRUE;
	    fflush(stderr);
	}
	return S3_SUCCESS;
    }

    if(!multi_prons){  /*Single pronounciation code */
      /* 20040706 ARCHAN: I separate the code of multiple pronounciations out.  Because that confuse the code 
	 by using strange finite state machines code. 
      */
      for (i = 0, l = r = sil, posn = WORD_POSN_END; i < n_phone-1; i++) {
	/* get new right context */
	r = phone[i+1];
	if (acmod_set_has_attrib(acmod_set, r, "filler"))
	  r = sil;
	
	b = phone[i];
	
	/* determine between word position {begin|start|single} */
	posn = btw_posn(btw_mark[i], posn);

#if 0
	E_INFO("Position %d, btw_mark[i] %d, multiw_mark[i] %d\n",posn, btw_mark[i],multiw_mark[i]);
#endif
	
	if (!acmod_set_has_attrib(acmod_set, b, "filler")) {
	  tri_id = acmod_set_tri2id(acmod_set,
				    b, l, r, posn);
	  if (tri_id != NO_ACMOD) {
	    /* got good triphone, so replace CI w/ tri */
	    phone[i] = tri_id;
	  }
	  else {
#if 0
	    E_WARN("Missing triphone, (%s %s %s %c), left as CI phone",
		       acmod_set_id2name(acmod_set, b),
		   acmod_set_id2name(acmod_set, l),
		   acmod_set_id2name(acmod_set, r),
		   word_posn_map[(int)posn]);
#endif
	  }
	}
	else {
	  /* phone[i] is a filler phone, so just leave it as is */
	  /* Change b to SIL for triphone context purposes */
	  b = sil;
	}
	
	/* Set next left context is the current base phone (where filler phones
	 * are mapped to SIL) */
	l = b;
      }

    }else{ /*Multiple pronounciation code */
      /* ARCHAN 20040702 : Hack!, This assumes in the transcripts, there are always
	 silences between words */
      /* This hacks enable us to make Viterbi training works after this change */

      last_l = phone[0];
      for (i = 0, l = r = sil, posn = WORD_POSN_END; i < n_phone-1; i++) {
	/* get new right context */
	r = phone[i+1];
	if (acmod_set_has_attrib(acmod_set, r, "filler"))
	  r = sil;
	b = phone[i];

	/* determine between word position {begin|start|single} */
	posn = btw_posn(btw_mark[i], posn);

	if(btw_mark[i]){
	  if(multiw_mark[i]){
	  }else{
	    last_l = b; 
	  }
	}
	

	if (!acmod_set_has_attrib(acmod_set, b, "filler")) {
	  if(btw_mark[i]){
	    /* If before the word boundary */
	    /* Hardwire the next word to be sil */
	    if(multiw_mark[i])
	      tri_id = acmod_set_tri2id(acmod_set, b, l, sil, posn);
	    else
	      tri_id = acmod_set_tri2id(acmod_set, b, l, r, posn);
	    
	  }else if(i>0 && btw_mark[i-1]){
	    /* If after the word boundary */
	    if(multiw_mark[i-1])
	      tri_id = acmod_set_tri2id(acmod_set, b, last_l, r, posn);
	    else
	      tri_id = acmod_set_tri2id(acmod_set, b, l, r, posn);
	  }else{
	    tri_id = acmod_set_tri2id(acmod_set, b, l, r, posn);
	  }
	  
	  if (tri_id != NO_ACMOD) 
	    phone[i] = tri_id;
	}
	else {
	  /* phone[i] is a filler phone, so just leave it as is */
	  /* Change b to SIL for triphone context purposes */
	  b = sil;
	}


#if 0
	E_INFO("tri_id %d b %s, l %s, r %s, last_l %s\n",
	       tri_id,
	       acmod_set_id2name(acmod_set, b),
	       acmod_set_id2name(acmod_set, l),
	       acmod_set_id2name(acmod_set, r),
	       acmod_set_id2name(acmod_set, last_l));
#endif

	/* Set next left context is the current base phone (where filler phones
	 * are mapped to SIL) */
	l = b;
      }

    }
    
    b = phone[i];
    r = sil;
      
    /* At this point, b is the right context of
       * the next to last phone.  Typically, this
       * is sil or some other filler phone. */
    
    if (!acmod_set_has_attrib(acmod_set, b, "filler")) {
	if (i > 0) {
	  E_WARN("utt does not end with filler phone\n");
	  
	  r = sil;
	}
	
	assert( btw_mark[i] );
	
	posn = btw_posn(btw_mark[i], posn);
	
	tri_id = acmod_set_tri2id(acmod_set,
				  b, l, r, posn);
	if (tri_id != NO_ACMOD)
	  phone[i] = tri_id;
	else {
	  E_WARN("Missing triphone, (%s %s %s %c), left as CI phone",
		 acmod_set_id2name(acmod_set, b),
		 acmod_set_id2name(acmod_set, l),
		 acmod_set_id2name(acmod_set, r),
		 word_posn_map[(int)posn]);
	}
    }
    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/13  06:31:19  arthchan2003
 * code checked in for multiple pronounciations
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.5  97/07/16  11:36:22  eht
 * *** empty log message ***
 * 
 * Revision 1.4  96/06/17  14:33:57  eht
 * Added typecasts
 * 
 * Revision 1.3  1996/03/25  15:30:39  eht
 * Removed unreferenced local var
 * 
 *
 * Revision 1.2  1995/12/01  20:47:04  eht
 * Include function prototypes for this .c file
 *
 * Revision 1.1  1995/12/01  20:41:47  eht
 * Initial revision
 *
 */
