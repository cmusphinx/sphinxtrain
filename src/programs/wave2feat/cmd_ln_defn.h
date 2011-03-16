/* ====================================================================
 * Copyright (c) 1998-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: cmd_ln_defn.h
 * 
 * Description: 
 *      Command line argument definition
 *
 * Author: 
 *      
 *********************************************************************/

#ifndef ARG_DEFN_H
#define ARG_DEFN_H

const char helpstr[] =
  "Description: \n\
Create cepstra from audio file.\n\
\n\
The main parameters that affect the final output, with typical values, are:\n\
\n\
samprate, typically 8000, 11025, or 16000\n\
lowerf, 130, 200, 130, for the respective sampling rates above\n\
upperf, 3700, 5200, 6800, for the respective sampling rates above\n\
nfilt, 31, 37, 40, for the respective sampling rates above\n\
nfft, 256 or 512\n\
format, raw or nist or mswav\n\
\"";

const char examplestr[] =
  "Example: \n\
This example creates a cepstral file named \"output.mfc\" from an input audio file named \"input.raw\", which is a raw audio file (no header information), which was originally sampled at 16kHz. \n\
\n\
wave2feat -i  input.raw \n\
        -o   output.mfc \n\
        -raw yes \n\
        -input_endian little \n\
        -samprate  16000 \n\
        -lowerf    130 \n\
        -upperf    6800 \n\
        -nfilt     40 \n\
        -nfft      512";

static arg_t defn[] = {
  { "-help",
    ARG_BOOLEAN,
    "no",
    "Shows the usage of the tool"},
  
  { "-example",
    ARG_BOOLEAN,
    "no",
    "Shows example of how to use the tool"},
  
  { "-i",
    ARG_STRING,
    NULL,
    "Single audio input file" },
  
  { "-o",
    ARG_STRING,
    NULL,
    "Single cepstral output file" },
  
  { "-c",
    ARG_STRING,
    NULL,
    "Control file for batch processing" },
  
  { "-nskip",
    ARG_STRING,
    NULL,
    "If a control file was specified, the number of utterances to skip at the head of the file" },
  
  { "-runlen",
    ARG_STRING,
    NULL,
    "If a control file was specified, the number of utterances to process (see -nskip too)" },
  
  { "-di",
    ARG_STRING,
    NULL,
    "Input directory, input file names are relative to this, if defined" },
  
  { "-ei",
    ARG_STRING,
    NULL,
    "Input extension to be applied to all input files" },
  
  { "-do",
    ARG_STRING,
    NULL,
    "Output directory, output files are relative to this" },
  
  { "-eo",
    ARG_STRING,
    NULL,
    "Output extension to be applied to all output files" },
  
  { "-nist",
    ARG_BOOLEAN,
    "no",
    "Defines input format as NIST sphere" },
  
  { "-raw",
    ARG_BOOLEAN,
    "no",
    "Defines input format as raw binary data" },
  
  { "-mswav",
    ARG_BOOLEAN,
    "no",
    "Defines input format as Microsoft Wav (RIFF)" },
  
  { "-input_endian",
    ARG_STRING,
    "little",
    "Endianness of input data, big or little, ignored if NIST or MS Wav" },
  
  { "-nchans",
    ARG_INT32,
    ONE_CHAN,
    "Number of channels of data (interlaced samples assumed)" },
  
  { "-whichchan",
    ARG_INT32,
    ONE_CHAN,
    "Channel to process" },
  
  { "-logspec",
    ARG_BOOLEAN,
    "no",
    "Write out logspectral files instead of cepstra" },
  
  { "-feat",
    ARG_STRING,
    "sphinx",
    "SPHINX format - big endian" },
  
  { "-mach_endian",
    ARG_STRING,
#ifdef WORDS_BIGENDIAN
    "big",
#else
    "little",
#endif
    "Endianness of machine, big or little" },
  
  { "-alpha",
    ARG_FLOAT32,
    DEFAULT_PRE_EMPHASIS_ALPHA,
    "Preemphasis parameter" },
  
  { "-samprate",
    ARG_FLOAT32,
    DEFAULT_SAMPLING_RATE,
    "Sampling rate" },
  
  { "-frate",
    ARG_INT32,
    DEFAULT_FRAME_RATE,
    "Frame rate" },
  
  { "-wlen",
    ARG_FLOAT32,
    DEFAULT_WINDOW_LENGTH,
    "Hamming window length" },
  
  { "-nfft",
    ARG_INT32,
    DEFAULT_FFT_SIZE,
    "Size of FFT" },
  
  { "-nfilt",
    ARG_INT32,
    DEFAULT_NUM_FILTERS,
    "Number of filter banks" },
  
  { "-lowerf",
    ARG_FLOAT32,
    DEFAULT_LOWER_FILT_FREQ,
    "Lower edge of filters" },
  
  { "-upperf",
    ARG_FLOAT32,
    DEFAULT_UPPER_FILT_FREQ,
    "Upper edge of filters" },
  
  { "-ncep",
    ARG_INT32,
    DEFAULT_NUM_CEPSTRA,
    "Number of cep coefficients" },
  
  { "-doublebw",
    ARG_BOOLEAN,
    "no",
    "Use double bandwidth filters (same center freq)" },
  
  { "-warp_type",
    ARG_STRING,
    DEFAULT_WARP_TYPE,
    "Warping function type (or shape)" },

  { "-warp_params",
    ARG_STRING,
    NULL,
    "Parameters defining the warping function" },

  { "-blocksize",
    ARG_INT32,
    DEFAULT_BLOCKSIZE,
    "Block size, used to limit the number of samples used at a time when reading very large audio files" },
  
  { "-dither",
    ARG_BOOLEAN,
    "yes",
    "Add 1/2-bit noise to avoid zero energy frames" },
  
  { "-seed",
    ARG_INT32,
    SEED,
    "Seed for random number generator; if less than zero, pick our own" },

  { "-verbose",
    ARG_BOOLEAN,
    "no",
    "Show input filenames" },
  
};

#endif /* ARG_DEFN_H */ 
