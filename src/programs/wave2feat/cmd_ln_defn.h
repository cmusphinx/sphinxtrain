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
 * 	Command line argument definition
 *
 * Author: 
 *      
 *********************************************************************/

#ifndef CMD_LN_DEFN_H
#define CMD_LN_DEFN_H

#include <s3/cmd_ln.h>
#include "fe.h"
#include "wave2feat.h"

const char helpstr[] =
  "Description: \n\
Create cepstra from audio file.\n		\
									\
The main parameters that affect the final output, with typical values, are:\n \
									\
samprate, typically 8000, 11025, or 16000\n				\
lowerf, 130, 200, 130, for the respective sampling rates above\n	\
upperf, 3700, 5200, 6800, for the respective sampling rates above\n	\
nfilt, 31, 37, 40, for the respective sampling rates above\n		\
nfft, 256 or 512\n							\
format, raw or nist or mswav\n						\
\"";

const char examplestr[] =
  "Example: \n\
This example creates a cepstral file named \"output.mfc\" from an input audio file named \"input.raw\", which is a raw audio file (no header information), which was originally sampled at 16kHz. \n \
\n									\
wave2feat -i  input.raw \n						\
        -o   output.mfc \n						\
        -raw no \n							\
        -input_endian little \n						\
        -samprate  16000 \n						\
        -lowerf    130 \n						\
        -upperf    6800 \n						\
        -nfilt     40 \n						\
        -nfft      512";

static arg_def_t defn[] = {
  { "-help",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Shows the usage of the tool"},
  
  { "-example",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Shows example of how to use the tool"},
  
  { "-i",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Single audio input file" },
  
  { "-o",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Single cepstral output file" },
  
  { "-c",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Control file for batch processing" },
  
  { "-di",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Input directory, input file names are relative to this, if defined" },
  
  { "-ei",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Input extension to be applied to all input files" },
  
  { "-do",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Output directory, output files are relative to this" },
  
  { "-eo",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Output extension to be applied to all output files" },
  
  { "-nist",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Defines input format as NIST sphere" },
  
  { "-raw",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Defines input format as raw binary data" },
  
  { "-mswav",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Defines input format as Microsoft Wav (RIFF)" },
  
  { "-input_endian",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    "little",
    "Endianness of input data, big or little, ignored if NIST or MS Wav" },
  
  { "-nchans",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    ONE_CHAN,
    "Number of channels of data (interlaced samples assumed)" },
  
  { "-whichchan",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    ONE_CHAN,
    "Channel to process" },
  
  { "-logspec",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Write out logspectral files instead of cepstra" },
  
  { "-feat",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    "sphinx",
    "SPHINX format - big endian" },
  
  { "-mach_endian",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
#ifdef WORDS_BIGENDIAN
    "big",
#else
    "little",
#endif
    "Endianness of machine, big or little" },
  
  { "-alpha",
    CMD_LN_FLOAT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_PRE_EMPHASIS_ALPHA,
    "Preemphasis parameter" },
  
  { "-srate",
    CMD_LN_FLOAT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_SAMPLING_RATE,
    "Sampling rate" },
  
  { "-frate",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_FRAME_RATE,
    "Frame rate" },
  
  { "-wlen",
    CMD_LN_FLOAT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_WINDOW_LENGTH,
    "Hamming window length" },
  
  { "-nfft",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_FFT_SIZE,
    "Size of FFT" },
  
  { "-nfilt",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_NUM_FILTERS,
    "Number of filter banks" },
  
  { "-lowerf",
    CMD_LN_FLOAT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_LOWER_FILT_FREQ,
    "Lower edge of filters" },
  
  { "-upperf",
    CMD_LN_FLOAT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_UPPER_FILT_FREQ,
    "Upper edge of filters" },
  
  { "-ncep",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_NUM_CEPSTRA,
    "Number of cep coefficients" },
  
  { "-doublebw",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Use double bandwidth filters (same center freq)" },
  
  { "-blocksize",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    DEFAULT_BLOCKSIZE,
    "Block size, used to limit the number of samples used at a time when reading very large audio files" },
  
  { "-dither",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Add 1/2-bit noise" },
  
  { "-verbose",
    CMD_LN_BOOLEAN,
    CMD_LN_NO_VALIDATION,
    "no",
    "Show input filenames" },
  
  { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
};

    
#define CMD_LN_DEFN_H

#endif /* CMD_LN_DEFN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2004/09/09  17:59:30  egouvea
 * Adding missing files to wave2feat
 * 
 *
 *
 */
