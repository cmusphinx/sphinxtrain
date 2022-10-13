/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * corpus.h -- Corpus-file related misc functions.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.13  2006/02/22 19:49:25  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII:
 * 1, Add structure utt_res_t, this is an utterance-based resouce
 * structure. Add basic operation such as free and report.
 * 2, Modify the structure of the loop in ctl_corpus to make it not so
 * clunky. Tested with make check .
 * 3, Completely removed ctl_process_dyn_lm, it is a product of code
 * duplication (alright, it is written by me......)
 * 4, Fixed doc-dox.
 *
 * Revision 1.12.4.3  2005/07/27 23:19:11  arthchan2003
 * 1, Added utt_res_t structure and its methods. 2, Changed the function pointer prototype. 3, Removed the lm and mllr set process out of ctl_process
 *
 * Revision 1.12.4.2  2005/07/26 03:14:17  arthchan2003
 * Removed ctl_process_dyn_lm. One of my sin.
 *
 * Revision 1.12.4.1  2005/07/05 06:25:40  arthchan2003
 * Fixed dox-doc.
 *
 * Revision 1.12  2005/06/21 20:44:34  arthchan2003
 * 1, Fixed doxygen documentation, 2, Add the $ keyword.
 *
 * Revision 1.4  2005/06/18 20:05:23  archan
 * Sphinx3 to s3.generic: Set lm correctly in dag.c and astar.c.  Same changes should also be applied to decode_anytopo.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 09-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added ctl_process_utt ().
 * 
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Updated ctl_infile() spec to included check for already existing file extension.
 * 
 * 23-Mar-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added a general purpose data argument to ctl_process() and its function
 * 		argument func.
 * 
 * 22-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added an optional validation function argument and an optional
 *		duplicate-resolution function argument to both corpus_load_headid() and
 * 		corpus_load_tailid().
 * 
 * 25-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _S3_CORPUS_H_
#define _S3_CORPUS_H_

#include <stdio.h>

#include <sphinxbase/hash_table.h>
#include <sphinxbase/profile.h>

#include <s3types.h>

/** \file corpus.h
 *  \brief Operations on corpus 
 */
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** \struct utt_res_t
    \brief A structure to store utterance-based resource
    Assume that most resource are string pointers, the string itself
    is pre-allocated somewhere.
*/
typedef struct 
{
    char* uttfile; /**< Utterance file name */
    char* lmname;  /**< LM file name for this utterance */

    char* fsgname;  /**< FSG file name for this utterance. For one
                       utterance, one could only use either LM or fsg */

    char* regmatname; /**< The regression matrix file name for this utterance */

    char* cb2mllrname; /**< The code book to regression matrix file name for this utterance 
                        */
} utt_res_t;

#define utt_res_set_uttfile(ur,name) ur->uttfile=name
#define utt_res_set_lmname(ur,name)  ur->lmname=name
#define utt_res_set_fsgname(ur,name) ur->fsgname=name
#define utt_res_set_regmatname(ur,name) ur->regmatname=name
#define utt_res_set_cb2mllrname(ur,name) ur->cb2mllrname=name

/** This just return a new utter_res_t */
utt_res_t* new_utt_res(void);

/** Free utt_res_t */
void free_utt_res(
    utt_res_t* ur /**< an utt_res_t */
    );

/** Report what's inside utt_res_t */
void report_utt_res(
    utt_res_t *ur /**< an utt_res_t */
    );

/**
 * \struct corpus_t
 * \brief  Structure for a corpus: essentially a set of strings each associated with a
 * unique ID. 
 * Structure for a corpus: essentially a set of strings each associated with a
 * unique ID.  (Such as a reference sentence file, hypothesis file, and various
 * control files.)
 * NOTE: IDs are CASE-SENSITIVE.
 */
typedef struct {
    hash_table_t *ht;	/**< Hash table for IDs; CASE-SENSITIVE */
    int32 n;		/**< \#IDs (and corresponding argument strings) in the corpus */
    char **str;		/**< The argument strings */
} corpus_t;


/**
 * Load a corpus from the given file and return it.
 * Each line is a separate entry in the corpus.  Blank lines are skipped.
 * The ID is the FIRST word in a line.
 * 
 * Validation:
 *
 * validate is an optional, application-supplied function to determine if each input
 * corpus data entry is eligible (valid) for inclusion in the final corpus.  It should
 * return an integer value signifying the following actions:
 *      0: Not valid, skip the entry;
 *     !0: Valid, include the entry.
 * If validate is NULL, every input entry is included in the corpus.
 * 
 * Duplicate resolution:
 *
 * dup_resolve is an optional, application-supplied function to resolve duplicate keys
 * (IDs).  It may be NULL if none is available.  If present, and a duplicate key is
 * encountered, the function is invoked with the original and the duplicate corpus
 * strings as arguments (s1 and s2, respectively).  It should return an integer value
 * signifying the following actions:
 *      0: Retain the original string, discard the new one;
 *     >0: Replace the original string with the new one;
 *     <0: Error (causes a FATAL_ERROR).
 * If dup_resolve is NULL, any duplicate ID causes a FATAL_ERROR.
 * 
 * Return value: Ptr to corpus if successful.
 */
corpus_t *corpus_load_headid (const char *file,	/**< Input file name, the file must be seekable and rewindable */
			      int32 (*validate)(char *str),
			      int32 (*dup_resolve)(char *s1, char *s2));

/**
 * Similar to corpus_load_headid, but the ID is at the END of each line, in parentheses.
 */
corpus_t *corpus_load_tailid (const char *file,	/**< Input file name, the file must be seekable and rewindable */
			      int32 (*validate)(char *str),
			      int32 (*dup_resolve)(char *s1, char *s2));

/**
 * Lookup the given corpus for the given ID and return the associated string.
 * Return NULL if ID not found.
 */
char *corpus_lookup (corpus_t *corp, const char *id);


/**
 * Read another entry from a S3 format "control file" and parse its various fields.
 * Blank lines and lines beginning with a hash-character (#) are omitted.
 * Control file entry format:
 *     uttfile(usually cepstrum file) [startframe endframe [uttid]]
 * Any error in control file entry format is FATAL.
 * Return value: 0 if successful, -1 if no more entries left.
 */

int32 ctl_read_entry (FILE *fp,         /**< In: an input file pointer */
		      char *uttfile,	/**< Out: (Cep)file containing utterance data */
		      int32 *sf,	/**< Out: Start frame in uttfile; 0 if omitted */
		      int32 *ef,	/**< Out: End frame in uttfile; -1 (signifying
					   until EOF) if omitted */
		      char *uttid	/**< Out: Utterance ID (generated from uttfile/sf/ef
					   if omitted) */
    );


/**
 * Process the given control file (or stdin if NULL): Skip the first
 * nskip entries, and process the next count entries by calling the
 * given function (*func) for each entry.  Any error in reading the
 * control file is FATAL.  ctllmfile and ctlmllrfile can be specified
 * optionally. If they are not specified, then NULL could be used.
 *
 * Return value: ptmr_t structure containing cpu/elapsed time stats for the run.
 */
S3DECODER_EXPORT
ptmr_t ctl_process (const char *ctlfile,	/**< In: Control file to read; use stdin if NULL */
		    const char *ctllmfile,     /**< In: Control file that specify the lm used for the corresponding utterance */
		    const char *ctlmllrfile,   /**< In: Contorl file that specify the mllr used for the corresponding utterance */
		    int32 nskip,	/**< In: No. of entries to skip at the head */
		    int32 count,	/**< In: No. of entries to process after nskip */
		    void (*func) (void *kb, utt_res_t *ur, int32 sf, int32 ef, char *uttid),
		    /**< In: Function to be invoked for each of the
		       count entries processed. */
		    void *kb		/**< In: A catch-all data pointer to be passed as
					   the first argument to func above */
    );


/**
 * Like ctl_process, but process the single filename given (uttfile), count times.  After each
 * processing, wait for the time of modification on the given file to change.  In this mode,
 * the decoder can be used to process a dynamically generated sequence of utterances.  To avoid
 * race conditions, each new instance of the file should be created "in an instant": by creating
 * it under a temporary name and finally renaming it to the given filename atomically.
 * @return: ptmr_t structure containing cpu/elapsed time stats for the run.
 */
S3DECODER_EXPORT
ptmr_t ctl_process_utt (const char *uttfile,	/**< In: Filename to be process (in its entirety) */
			int32 count,	/**< In: No. of iterations to process uttfile */
			void (*func) (void *kb, utt_res_t *ur, int32 sf, int32 ef, char *uttid),/**< A function pointer that do the actual processing */

			void *kb);

/**
 * Build a complete input filename from the given uttname, directory and file-extension:
 *   If utt begins with a / ignore dir, otherwise prefix dir/ to utt;
 *   If a non-empty file extension is provided, and utt doesn't already have that extension,
 * 	append .ext to filename.
 */
void ctl_infile (char *file,	/**< Out: Generated filename (allocated by caller) */
		 const char *dir,	/**< In: Optional directory spec if relative utt specified */
		 const char *ext,	/**< In: File extension to be appended to utt to generate
					   complete filename */
		 const char *utt	/**< In: Utterance file pathname, absolute or relative,
					   with or without file extension.  This is usually the
					   first field in a control file */
    );

/**
 * Build a complete output filename from the given components as follows:
 *     if dir ends with ,CTL and utt does not begin with /, use dir/utt
 *     if dir ends with ,CTL and utt DOES begin with /, filename is utt
 *     if dir does not end with ,CTL, filename is dir/uttid.
 * If a non-empty ext specified append .ext to generated filename.
 */
void ctl_outfile (char *file,	/**< Out: Generated filename (allocated by caller) */
		  const char *dir,	/**< In: Directory for the generated filename; see comment
					   for special handling of ,CTL suffix */
		  const char *ext,	/**< In: File-extension applied to the generated filename */
		  const char *utt,	/**< In: Utterance file pathname, absolute or relative,
					   with or without extension.  This is usually the first
					   field in a control file. */
		  const char *uttid,	/**< In: Utterance ID (derived from the control file */
                  int build_dirs        /**< In: Create parent directories if true */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
