/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
/**********************************************************************
 *
 * File: acmod_set.h
 * 
 * Description: 
 *	This header defines the interface to the acmod_set (acoustic
 *	model set) module.  See acmod_set.c for the detailed
 *	descriptions of each function below.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef ACMOD_SET_H
#define ACMOD_SET_H


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <sphinxbase/prim_type.h>

typedef unsigned char ci_acmod_id_t;

#define NO_CI_ACMOD	(0xff)
#define MAX_CI_ACMOD	(0xfe)

typedef uint32 acmod_id_t;

#define NO_ACMOD	(0xffffffff)	/* The ID representing no acoustic
					 * model. */
#define MAX_ACMOD	(0xfffffffe)	/* The max ID possible */

typedef enum {
    WORD_POSN_BEGIN = 0,	/* beginning phone of word */
    WORD_POSN_END = 1,		/* ending phone of word */
    WORD_POSN_SINGLE = 2,	/* single phone word (i.e. begin & end) */
    WORD_POSN_INTERNAL = 3,	/* internal phone of word */
    WORD_POSN_UNDEFINED = 4	/* undefined value, used for initial conditions, etc */
} word_posn_t;

#define N_WORD_POSN	4	/* total # of word position indicators (excluding undefined) */

/* the following map must be consistent w/ the types above.  Unpredictable
 * behaviour w/ result if not.  */

#define WORD_POSN_CHAR_MAP	"besiu"	/* b == begin, e == end, s == single-phone
					   i == word-internal, u == undefined */

/* acoustic model record (describes triphones, diphones, etc.) */
typedef struct acmod_s {
    ci_acmod_id_t base;		/* The base phone associated w/ this phone */

    ci_acmod_id_t left_context;	/* The left context associated w/ this phone,
				 * if there is no specified left context, this
				 * is set to NO_ACMOD */

    ci_acmod_id_t right_context; /* The right context associated w/ this phone,
				  * This can be any CI phone id or NO_ACMOD.
				  * NO_ACMOD indicates to specified right context */

    word_posn_t posn;		 /* Word position indicator (see above) */

    /* The name is generatable from the information above */

    char **attrib;	/* A NULL terminated list of C strings which
			 * represent various attributes of the acoustic model.
			 * For instance, "ci", "non_speech", "triphone"
			 * model. */

} acmod_t;

/* ci_acmod record (represents base phones and filler words) */
typedef struct ci_acmod_s {
    char *name;		/* A C string representation for the acoustic model */

    char **attrib;	/* A NULL terminated list of C strings which
			 * represent various attributes of the acoustic model.
			 * For instance, "ci", "non_speech", "triphone"
			 * model. */
    
    ci_acmod_id_t id;		/* The ID of this acoustic model */
} ci_acmod_t;

#include <s3/itree.h>

typedef struct acmod_set_s {
    ci_acmod_t *ci;		/* base phone and filler model list.
				 * The base phone set are used to compose
				 * triphones, diphones, etc.  They are also
				 * acoustic models themselves that represent
				 * phones trained with no regard to their
				 * left and right contexts.
				 *
				 * filler models are used to represent non-speech
				 * signals such as silence (actually stationary
				 * background noise), coughs, door slams, etc. */
    uint32 n_ci;	/* # of CI acoustic models */
    uint32 max_n_ci;

    acmod_t *multi;		/* The remaining acoustic models.  Includes only
				 * triphones at the moment. */
    uint32 n_multi;
    uint32 max_n_multi;

    uint32 next_id;	/* The ID which would be assigned to the next
				 * new acoustic model */
    
    itree_t **multi_idx;	/* A quick index for mapping a multiphone to
				 * an ID. */

    char **attrib;		/* A NULL terminated list of C strings which
				 * represent all possible attributes of the phones in
				 * a phone set.
				 * For instance, "base", "non_speech", "triphone" */
    
    uint32 *n_with;	/* The number of phones having each attribute. */
} acmod_set_t;

acmod_set_t *
acmod_set_new(void);

int32
acmod_set_set_n_ci_hint(acmod_set_t *acmod_set,
			uint32 n_ci_hint);

int32
acmod_set_set_n_tri_hint(acmod_set_t *acmod_set,
			 uint32 n_tri_hint);

acmod_id_t
acmod_set_add_ci(acmod_set_t *acmod_set,
		 const char *name,
		 const char **attrib);

acmod_id_t
acmod_set_add_tri(acmod_set_t *acmod_set,
		  acmod_id_t base,
		  acmod_id_t left_context,
		  acmod_id_t right_context,
		  word_posn_t posn,
		  const char **attrib);

acmod_id_t
acmod_set_name2id(acmod_set_t *acmod_set,
		  const char *name);

const char *
acmod_set_id2name(acmod_set_t *acmod_set,
		  acmod_id_t id);

const char *
acmod_set_id2fullname(acmod_set_t *acmod_set,
		      acmod_id_t id);

const char *
acmod_set_id2s2name(acmod_set_t *acmod_set,
		    acmod_id_t id);

acmod_id_t
acmod_set_enum_init(acmod_set_t *acmod_set,
		    acmod_id_t base);

acmod_id_t
acmod_set_enum(void);

acmod_id_t
acmod_set_tri2id(acmod_set_t *acmod_set,
		 acmod_id_t base,
		 acmod_id_t left_context,
		 acmod_id_t right_context,
		 word_posn_t posn);

int32
acmod_set_id2tri(acmod_set_t *acmod_set,
		 acmod_id_t *base,
		 acmod_id_t *left_context,
		 acmod_id_t *right_context,
		 word_posn_t *posn,
		 acmod_id_t id);

const char *
acmod_set_s2_id2name(acmod_set_t *acmod_set,
		     acmod_id_t id);

uint32
acmod_set_n_acmod(acmod_set_t *acmod_set);

uint32
acmod_set_n_base_acmod(acmod_set_t *acmod_set);

uint32
acmod_set_n_multi(acmod_set_t *acmod_set);

uint32
acmod_set_n_ci(acmod_set_t *acmod_set);

acmod_id_t
acmod_set_base_phone(acmod_set_t *acmod_set,
		     acmod_id_t id);

uint32
acmod_set_has_attrib(acmod_set_t *acmod_set,
		     acmod_id_t id,
		     const char *attrib);

const char **
acmod_set_attrib(acmod_set_t *acmod_set,
		 acmod_id_t id);

int
acmod_set_s2_parse_triphone(acmod_set_t *acmod_set,
			    acmod_id_t *base,
			    acmod_id_t *left,
			    acmod_id_t *right,
			    word_posn_t *posn,
			    char *str);
#ifdef __cplusplus
}
#endif

#endif /* ACMOD_SET_H */ 
