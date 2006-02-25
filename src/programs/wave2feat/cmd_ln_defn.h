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

#ifndef CMD_LN_DEFN_H
#define CMD_LN_DEFN_H

#include <s3/cmd_ln.h>
#include "fe.h"
#include "wave2feat.h"

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
  
  { "-nskip",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "If a control file was specified, the number of utterances to skip at the head of the file" },
  
  { "-runlen",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "If a control file was specified, the number of utterances to process (see -nskip too)" },
  
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
  
  { "-warp_type",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    DEFAULT_WARP_TYPE,
    "Warping function type (or shape)" },

  { "-warp_params",
    CMD_LN_STRING,
    CMD_LN_NO_VALIDATION,
    CMD_LN_NO_DEFAULT,
    "Parameters defining the warping function" },

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
  
  { "-seed",
    CMD_LN_INT32,
    CMD_LN_NO_VALIDATION,
    SEED,
    "Seed for random number generator; if less than zero, pick our own" },

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
 * Revision 1.7  2006/02/25  00:53:48  egouvea
 * Added the flag "-seed". If dither is being used and the seed is less
 * than zero, the random number generator is initialized with time(). If
 * it is at least zero, it's initialized with the provided seed. This way
 * we have the benefit of having dither, and the benefit of being
 * repeatable.
 * 
 * This is consistent with what sphinx3 does. Well, almost. The random
 * number generator is still what the compiler provides.
 * 
 * Also, moved fe_init_params to fe_interface.c, so one can initialize a
 * variable of type param_t with meaningful values.
 * 
 * Revision 1.6  2006/02/17 00:31:34  egouvea
 * Removed switch -melwarp. Changed the default for window length to
 * 0.025625 from 0.256 (so that a window at 16kHz sampling rate has
 * exactly 410 samples). Cleaned up include's. Replaced some E_FATAL()
 * with E_WARN() and return.
 *
 * Revision 1.5  2006/02/16 00:18:26  egouvea
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
 * Revision 1.4  2006/02/14 20:56:54  eht
 * Implement an argument -melwarp that changes the standard mel-scale
 * equation from:
 *      M(f) = 2595 * log10( 1 + f/700 )
 * to:
 *      M(f,w) = 2595 * log10( 1 + f/(700*w))
 *
 * So, 1.0 means no warp,  w > 1.0 means linear compression w < 1.0 means
 * linear expansion.
 *
 * Implement argument -nskip and -runlen arguments so that a subset of the
 * utterances in the control file can be executed.  Allows a simple
 * distribution of wave2feat processing over N processors.
 *
 * Revision 1.3  2005/05/19 21:21:55  egouvea
 * Bug #1176394: example bug
 *
 * Revision 1.2  2004/11/23 04:14:06  egouvea
 * Fixed bug in cmd_ln.c in which a wrong boolean argument led into an
 * infinite loop, and fixed the help and example strings, getting rid of
 * spaces, so that the appearance is better.
 *
 * Revision 1.1  2004/09/09 17:59:30  egouvea
 * Adding missing files to wave2feat
 *
 *
 *
 */
