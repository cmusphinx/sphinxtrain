/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
/*
 * feat.h -- Cepstral features computation.
 *
 * HISTORY
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added sf, ef parameters to s2mfc_read().
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added feat_cepsize().
 * 
 * 09-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Moved constant declarations to feat.c.
 * 
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFEAT_FEAT_H_
#define _LIBFEAT_FEAT_H_

/*
 * Structure for describing a speech feature type (no. of streams and stream widths),
 * as well as the computation for converting the input speech (e.g., Sphinx-II format
 * MFC cepstra) into this type of feature vectors.
 */
typedef struct feat_s {
    char *name;		/* Printable name for this feature type */
    int32 cepsize;	/* Size of input speech vector (typically, a cepstrum vector) */
    int32 cepsize_used;	/* No. of cepstrum vector dimensions actually used (0 onwards) */
    int32 n_stream;	/* #Feature streams; e.g., 4 in Sphinx-II */
    int32 *stream_len;	/* Vector length of each feature stream */
    int32 window_size;	/* #Extra frames around given input frame needed to compute
			   corresponding output feature (so total = window_size*2 + 1) */
    void (*compute_feat)(struct feat_s *fcb, float32 **input, float32 **feat);
			/* Function for converting window of input speech vector
			   (input[-window_size..window_size]) to output feature vector
			   (feat[stream][]).  If NULL, no conversion available, the
			   speech input must be feature vector itself.
			   Return value: 0 if successful, -ve otherwise. */
} feat_t;

/* Access macros */
#define feat_name(f)		((f)->name)
#define feat_cepsize(f)		((f)->cepsize)
#define feat_cepsize_used(f)	((f)->cepsize_used)
#define feat_n_stream(f)	((f)->n_stream)
#define feat_stream_len(f,i)	((f)->stream_len[i])
#define feat_window_size(f)	((f)->window_size)


/*
 * Read feature vectors from the given file.  Feature file format:
 *   Line containing the single word: s3
 *   File header including any argument value pairs/line and other text (e.g.,
 * 	'chksum0 yes', 'version 1.0', as in other S3 format binary files)
 *   Header ended by line containing the single word: endhdr
 *   (int32) Byte-order magic number (0x11223344)
 *   (int32) No. of frames in file (N)
 *   (int32) No. of feature streams (S)
 *   (int32 x S) Width or dimensionality of each feature stream (sum = L)
 *   (float32) Feature vector data (NxL float32 items).
 *   (uint32) Checksum (if present).
 * (Note that this routine does NOT verify the checksum.)
 * Return value: # frames read if successful, -1 if error.
 */
int32 feat_readfile (feat_t *fcb,	/* In: Control block from feat_init() */
		     char *file,	/* In: File to read */
		     int32 sf,		/* In: Start/end frames (range) to be read from
					   file; use 0, 0x7ffffff0 to read entire file */
		     int32 ef,
		     float32 ***feat,	/* Out: Data structure to be filled with read
					   data; allocate using feat_array_alloc() */
		     int32 maxfr);	/* In: #Frames allocated for feat above; error if
					   attempt to read more than this amount. */

/*
 * Counterpart to feat_readfile.  Feature data is assumed to be in a contiguous block
 * starting from feat[0][0][0].  (NOTE: No checksum is written.)
 * Return value: # frames read, including padding, if successful, -1 if error.
 */
int32 feat_writefile (feat_t *fcb,	/* In: Control block from feat_init() */
		     char *file,	/* In: File to write */
		     float32 ***feat,	/* In: Feature data to be written */
		     int32 nfr);	/* In: #Frames to be written */

/*
 * Read Sphinx-II format mfc file (s2mfc = Sphinx-II format MFC data).
 * Return value: #frames read, including padding, if successful, -1 if error (e.g.,
 * mfc array too small).
 */
int32 s2mfc_read (char *file,		/* In: Sphinx-II format MFC file to be read */
		  int32 sf, int32 ef,	/* In: Start/end frames (range) to be read from
					   file.  If ef is -ve, read until EOF. */
		  int32 featpad,	/* In: #Frames of extra MFC data needed to be
					   padded at either end, to compute features */
		  float32 **mfc,	/* Out: 2-D array to be filled with MFC data,
					   including padding (featpad, above). */
		  int32 maxfr);		/* In: #Frames of mfc array allocated; error if
					   attempt to read more than this amount. */

/*
 * Allocate an array to hold several frames worth of feature vectors.  The returned value
 * is the float32 ***data array, organized as follows:
 *   data[0][0] = frame 0 stream 0 vector, data[0][1] = frame 0 stream 1 vector, ...
 *   data[1][0] = frame 1 stream 0 vector, data[0][1] = frame 1 stream 1 vector, ...
 *   data[2][0] = frame 2 stream 0 vector, data[0][1] = frame 2 stream 1 vector, ...
 *   ...
 * NOTE: For I/O convenience, the entire data area is allocated as one contiguous block.
 * Return value: Pointer to the allocated space if successful, NULL if any error.
 */
float32 ***feat_array_alloc (feat_t *fcb,	/* In: Descriptor from feat_init(), used
						   to obtain #streams and stream sizes */
			     int32 nfr);	/* In: #Frames for which to allocate */

/*
 * Like feat_array_alloc except that only a single frame is allocated.  Hence, one
 * dimension less.
 */
float32 **feat_vector_alloc (feat_t *fcb);	/* In: Descriptor from feat_init(), used
						   to obtain #streams and stream sizes */

/*
 * Initialize feature module to use the selected type of feature stream.  One-time only
 * initialization at the beginning of the program.  Input type is a string defining the
 * kind of input->feature conversion desired:
 *   "s2_4x":   s2mfc->Sphinx-II 4-feature stream,
 *   "s3_1x39": s2mfc->Sphinx-3 single feature stream,
 *   "n1,n2,n3,...": Explicit feature vector layout spec. with comma-separated feature
 * 	stream lengths.  In this case, the input data is already in the feature format
 * 	and there is no conversion necessary.
 * Return value: (feat_t *) descriptor if successful, NULL if error.  Caller must not
 * directly modify the contents of the returned value.
 */
feat_t *feat_init (char *type);		/* In: type of feature stream */


#endif
