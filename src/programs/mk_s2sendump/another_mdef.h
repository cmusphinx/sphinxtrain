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
/*
 * from s3 mdef.h
 *
 * Yet another definition of a model
 *
 * Extracted by Alan W Black (awb@cs.cmu.edu)
 * from s3.2 stuff.
 */


#ifndef _ANOTHER_MDEF_H_
#define _ANOTHER_MDEF_H_


/* CI phone information */
typedef struct {
    char *name;
    int32 filler;		/* Whether a filler phone; if so, can be substituted by
				   silence phone in left or right context position */
} ciphone_t;

/*
 * Triphone information, including base phones as a subset.  For the latter, lc, rc
 * and wpos are non-existent.
 */
typedef struct {
    s3ssid_t ssid;		/* State sequence (or senone sequence) ID, considering the
				   n_emit_state senone-ids are a unit.  The senone sequences
				   themselves are in a separate table */
    s3tmatid_t tmat;		/* Transition matrix id */
    s3cipid_t ci, lc, rc;	/* Base, left, right context ciphones */
    word_posn_t wpos;		/* Word position */
} phone_t;

/*
 * Structures needed for mapping <ci,lc,rc,wpos> into pid.  (See mdef_t.wpos_ci_lclist
 * below.)  (lc = left context; rc = right context.)
 * NOTE: Both ph_rc_t and ph_lc_t FOR INTERNAL USE ONLY.
 */
typedef struct ph_rc_s {
    s3cipid_t rc;		/* Specific rc for a parent <wpos,ci,lc> */
    s3pid_t pid;		/* Triphone id for above rc instance */
    struct ph_rc_s *next;	/* Next rc entry for same parent <wpos,ci,lc> */
} ph_rc_t;

typedef struct ph_lc_s {
    s3cipid_t lc;		/* Specific lc for a parent <wpos,ci> */
    ph_rc_t *rclist;		/* rc list for above lc instance */
    struct ph_lc_s *next;	/* Next lc entry for same parent <wpos,ci> */
} ph_lc_t;


/* The main model definition structure */
typedef struct {
    int32 n_ciphone;		/* #basephones actually present */
    int32 n_phone;		/* #basephones + #triphones actually present */
    int32 n_emit_state;		/* #emitting states per phone */
    int32 n_ci_sen;		/* #CI senones; these are the first */
    int32 n_sen;		/* #senones (CI+CD) */
    int32 n_tmat;		/* #transition matrices */
    
    hash_table_t *ciphone_ht;	/* Hash table for mapping ciphone strings to ids */
    ciphone_t *ciphone;		/* CI-phone information for all ciphones */
    phone_t *phone;		/* Information for all ciphones and triphones */
    s3senid_t **sseq;		/* Unique state (or senone) sequences in this model, shared
				   among all phones/triphones */
    int32 n_sseq;		/* No. of unique senone sequences in this model */
    
    s3senid_t *cd2cisen;	/* Parent CI-senone id for each senone; the first
				   n_ci_sen are identity mappings; the CD-senones are
				   contiguous for each parent CI-phone */
    s3cipid_t *sen2cimap;	/* Parent CI-phone for each senone (CI or CD) */
    int32 *ciphone2n_cd_sen;	/* #CD-senones for each parent CI-phone */
    
    s3cipid_t sil;		/* SILENCE_CIPHONE id */
    
    ph_lc_t ***wpos_ci_lclist;	/* wpos_ci_lclist[wpos][ci] = list of lc for <wpos,ci>.
				   wpos_ci_lclist[wpos][ci][lc].rclist = list of rc for
				   <wpos,ci,lc>.  Only entries for the known triphones
				   are created to conserve space.
				   (NOTE: FOR INTERNAL USE ONLY.) */
} mdef_t;

/* Access macros; not meant for arbitrary use */
#define mdef_is_fillerphone(m,p)	((m)->ciphone[p].filler)
#define mdef_n_ciphone(m)		((m)->n_ciphone)
#define mdef_n_phone(m)			((m)->n_phone)
#define mdef_n_sseq(m)			((m)->n_sseq)
#define mdef_n_emit_state(m)		((m)->n_emit_state)
#define mdef_n_sen(m)			((m)->n_sen)
#define mdef_n_tmat(m)			((m)->n_tmat)
#define mdef_pid2ssid(m,p)		((m)->phone[p].ssid)
#define mdef_pid2tmatid(m,p)		((m)->phone[p].tmat)
#define mdef_silphone(m)		((m)->sil)
#define mdef_sen2cimap(m)		((m)->sen2cimap)


/*
 * Initialize the phone structure from the given model definition file.
 * Return value: pointer to the phone structure created.
 * It should be treated as a READ-ONLY structure.
 */
mdef_t *mdef_init (char *mdeffile);


/* Return value: ciphone id for the given ciphone string name */
s3cipid_t mdef_ciphone_id (mdef_t *m,		/* In: Model structure being queried */
			   char *ciphone);	/* In: ciphone for which id wanted */

/* Return value: READ-ONLY ciphone string name for the given ciphone id */
const char *mdef_ciphone_str (mdef_t *m,	/* In: Model structure being queried */
			      s3cipid_t ci);	/* In: ciphone id for which name wanted */

/* Return 1 if given triphone argument is a ciphone, 0 if not, -1 if error */
int32 mdef_is_ciphone (mdef_t *m,		/* In: Model structure being queried */
		       s3pid_t p);		/* In: triphone id being queried */

/* Return value: phone id for the given constituents if found, else BAD_S3PID */
s3pid_t mdef_phone_id (mdef_t *m,		/* In: Model structure being queried */
		       s3cipid_t b,		/* In: base ciphone id */
		       s3cipid_t l,		/* In: left context ciphone id */
		       s3cipid_t r,		/* In: right context ciphone id */
		       word_posn_t pos);	/* In: Word position */

/*
 * Like phone_id, but backs off to other word positions if exact triphone not found.
 * Also, non-SILENCE_PHONE filler phones back off to SILENCE_PHONE.
 * Ultimately, backs off to base phone id.  Thus, it should never return BAD_S3PID.
 */
s3pid_t mdef_phone_id_nearest (mdef_t *m,	/* In: Model structure being queried */
			       s3cipid_t b,	/* In: base ciphone id */
			       s3cipid_t l,	/* In: left context ciphone id */
			       s3cipid_t r,	/* In: right context ciphone id */
			       word_posn_t pos);	/* In: Word position */

/*
 * Create a phone string for the given phone (base or triphone) id in the given buf.
 * Return value: 0 if successful, -1 if error.
 */
int32 mdef_phone_str (mdef_t *m,		/* In: Model structure being queried */
		      s3pid_t pid,		/* In: phone id being queried */
		      char *buf);		/* Out: On return, buf has the string */

/*
 * Obtain phone components: inverse of mdef_phone_id().
 * Return value: 0 if successful, -1 otherwise.
 */
int32 mdef_phone_components (mdef_t *m,		/* In: Model structure being queried */
			     s3pid_t p,		/* In: triphone id being queried */
			     s3cipid_t *b,	/* Out: base ciphone id */
			     s3cipid_t *l,	/* Out: left context ciphone id */
			     s3cipid_t *r,	/* Out: right context ciphone id */
			     word_posn_t *pos);	/* Out: Word position */

/*
 * Compare the underlying HMMs for two given phones (i.e., compare the two transition
 * matrix IDs and the individual state(senone) IDs).
 * Return value: 0 iff the HMMs are identical, -1 otherwise.
 */
int32 mdef_hmm_cmp (mdef_t *m,			/* In: Model being queried */
		    s3pid_t p1, s3pid_t p2);	/* In: The two triphones being compared */

/*
 * From the given array of active senone-sequence flags, mark the corresponding senones that
 * are active.  Caller responsible for allocating sen[], and for clearing it, if necessary.
 */
void mdef_sseq2sen_active (mdef_t *mdef,
			   int32 *sseq,		/* In: sseq[ss] is != 0 iff senone-sequence ID
						   ss is active */
			   int32 *sen);		/* In/Out: Set sen[s] to non-0 if so indicated
						   by any active senone sequence */

/* For debugging */
void mdef_dump (FILE *fp, mdef_t *m);


#endif
