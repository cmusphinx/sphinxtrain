/* ====================================================================
 * Copyright (c) 1996-2004 Carnegie Mellon University.  All rights
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

#ifndef _FE_INTERNAL_H_
#define _FE_INTERNAL_H_

/** \file fe_internal.h
    \brief Signal processing functions
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef	M_PI
#define M_PI	(3.14159265358979323846)
#endif	/* M_PI */

#define FORWARD_FFT 1
#define INVERSE_FFT -1

  /** \struct complex
      \brief A structure to represent complex numbers
   */
typedef struct { float64 r, i; } complex;

  /** Build mel filters */
  int32 fe_build_melfilters(melfb_t *MEL_FB /**< Input: A mel-frequency banks data structure */
			  );

  /** Compute coefficients for mel cosine transform. The cosine
      transform is the last step in the cepstra computation.
   */
  int32 fe_compute_melcosine(melfb_t *MEL_FB /**< Input: A mel-frequency bank data structure */
			   );

  /** Convert a frequency in linear scale to mel scale */
  float32 fe_mel(float32 x /**< Input: A frequency in linear scale */
		 );

  /** Convert a frequency from mel scale to linear scale */
  float32 fe_melinv(float32 x /**< Input: A frequency in mel-scale */
		    );

  /** Add dither to an audio buffer. Dither is a 1/2 bit random number
      added to prevent zero energy frames, which happens when the
      audio is a sequence of zeros. */
  int32 fe_dither(int16 *buffer, /**< Input: The audio buffer */
		  int32 nsamps /**< Input: the number of samples */
		  );

  /** Perform pre-emphasis. The pre-emphasis filter is:
      y[n] = x[n] - factor * x[n-1]
   */
  void fe_pre_emphasis(int16 const *in, /**< Input: The input vector */
		       float64 *out,  /**< Output: The output vector */
		       int32 len,     /**< Input: The length of the input vector in */
		       float32 factor,  /**< Input: The preemphasis factor */
		       int16 prior      /**< Input: The prior sample, either zero or the last sample from the previous buffer */
		       );

  /** Create the hamming window, defined as:
      in[i] = 0.54  - 0.46 * cos (2 * PI * i / (in_len -1))
 */
  void fe_create_hamming(float64 *in, int32 in_len);

  /** Apply the hamming window to the incoming samples */
  void fe_hamming_window(float64 *in, /**< Input: The input vector */
			 float64 *window,  /**< Input: The precomputed window */
			 int32 in_len /**< Input: the length of the input vector in */
		       );

  /** Compute the magnitude of a spectrum 
   */
  void fe_spec_magnitude(float64 const *data, /**< Input : The input data */
			 int32 data_len,  /**< Input : The data length */
			 float64 *spec, /**< Input : The output spectrum */
			 int32 fftsize /**< Input: The FFT size */
			 );
  
  /** Compute the feature (cepstrum) from frame (wave)
   */
  int32 fe_frame_to_fea(fe_t *FE,  /**< Input: A structure with the front end parameters */
			float64 *in,  /**< Input: The input audio data */
			float64 *fea  /**< Output: The output cepstral feature vector */
			); 

  /** Compute the mel spectrum */
  void fe_mel_spec(fe_t *FE,  /**< Input: A structure with the front end parameters */
		   float64 const *spec,  /**< Input: The spectrum vector */
		   float64 *mfspec  /**< Output: A mel scale spectral vector */
		   );

  /** Compute the mel cepstrum */
  int32 fe_mel_cep(fe_t *FE,  /**< Input: A structure with the front end parameters */
		   float64 *mfspec, /**< Input:  mel scale spectral vector */
		   float64 *mfcep   /**< Output: mel cepstral vector */
		   );

  /** Fast Fourier Transform using complex numbers */
  int32 fe_fft(complex const *in, /**< Input: The complex input vector */
	       complex *out,      /**< Output: The complex output vector */
	       int32 N,           /**< The FFT size */
	       int32 invert       /**< direct FFT if 1 or inverse FFT if -1 */
	       );

  /** Fast Fourier Transform (FFT) using real numbers */
  int32 fe_fft_real(float64 *x, /**< Input/Output: The input vector in real numbers */
		    int n,           /**< Input: The FFT size */
		    int m           /**< Input: The order (log2(size)) of the FFT */
	  );

  /** Convert short to double. Audio is normally quantized as 2-byte
      integers. These are converted to floating point for
      processing. */
  void fe_short_to_double(int16 const *in, /**< Input: audio as 2-byte integer */
			  float64 *out,    /**< Output: audio as floating point */
			  int32 len        /**< Input: Number of samples  */
			  );

  /** DUPLICATION! Front end specific memory allocation routine 
      Create a 2D array. 
      @return: a d1 x d2 array will be returned
   */
  char **fe_create_2d(int32 d1, /** Input: first dimension*/
		      int32 d2, /** Input: second dimension*/
		      int32 elem_size /** Input : the element size */
		      );

  /** DUPLICATION! Front end specific memory delallocation routine */
  void fe_free_2d(void **arr /**Input: a 2d matrix to be freed */
		);

  /** Print the front end parameters used.  */
  void fe_print_current(fe_t const *FE /** Input: the front end parameters used. */
		      );
  
  /**Parse parameters used by an application and copies into an internal structure. */
  void fe_parse_general_params(param_t const *P, /**Input: front end parameters set by the application */
			       fe_t *FE /**Output: front end parameters used internally */
			       );

  /**Parse mel frequency parameters from the structure used by the application. */
  void fe_parse_melfb_params(param_t const *P, /**Input: front end parameters set by the application */
			     melfb_t *MEL /**Output: mel filter parameters used internally */
			     );

#ifdef __cplusplus
}
#endif

#endif

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.14  2006/02/20  23:55:51  egouvea
 * Moved fe_dither() to the "library" side rather than the app side, so
 * the function can be code when using the front end as a library.
 * 
 * Revision 1.13  2006/02/17 00:31:34  egouvea
 * Removed switch -melwarp. Changed the default for window length to
 * 0.025625 from 0.256 (so that a window at 16kHz sampling rate has
 * exactly 410 samples). Cleaned up include's. Replaced some E_FATAL()
 * with E_WARN() and return.
 *
 * Revision 1.12  2006/02/16 00:18:26  egouvea
 * Implemented flexible warping function. The user can specify at run
 * time which of several shapes they want to use. Currently implemented
 * are an affine function (y = ax + b), an inverse linear (y = a/x) and a
 * piecewise linear (y = ax, up to a frequency F, and then it "breaks" so
 * Nyquist frequency matches in both scales.
 *
 * Added two switches, -warp_type and -warp_params. The first specifies
 * the type, which valid values:
 *
 * -inverse or inverse_linear
 * -linear or affine
 * -piecewise or piecewise_linear
 *
 * The inverse_linear is the same as implemented by EHT. The -mel_warp
 * switch was kept for compatibility (maybe remove it in the
 * future?). The code is compatible with EHT's changes: cepstra created
 * from code after his changes should be the same as now. Scripts that
 * worked with his changes should work now without changes. Tested a few
 * cases, same results.
 *
 */
