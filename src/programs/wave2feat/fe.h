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
#ifndef _FE_H_
#define _FE_H_

#include <s3/prim_type.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
     float32 SAMPLING_RATE;
     int32 FRAME_RATE;
     float32 WINDOW_LENGTH;
     int32 FB_TYPE;
     int32 NUM_CEPSTRA;
     int32 NUM_FILTERS;
     int32 FFT_SIZE;
     float32 LOWER_FILT_FREQ;
     float32 UPPER_FILT_FREQ;
     float32 PRE_EMPHASIS_ALPHA;

     char *warp_type;
     char *warp_params;
     float32 MEL_WARP;

     char *wavfile;
     char *cepfile;
     char *ctlfile;
     int32 nskip;
     int32 runlen;
     char *wavdir;
     char *cepdir;
     char *wavext;
     char *cepext;
     int32 input_format;
     int32 is_batch;
     int32 is_single;
     int32 blocksize;
     int32 verbose;
     int32 machine_endian;
     int32 input_endian;
     int32 output_endian;
     int32 dither;
     int32 logspec;
     int32 doublebw;
     int32 nchans;
     int32 whichchan;
  
     int32 splen;
     int32 nframes;
     int16* spdata;
} param_t;


typedef struct{
     float32 sampling_rate;
     int32 num_cepstra;
     int32 num_filters;
     int32 fft_size;
     float32 lower_filt_freq;
     float32 upper_filt_freq;
     float32 **filter_coeffs;
     float32 **mel_cosine;
     float32 *left_apex;
     int32 *width;
     int32 doublewide;
     char *warp_type;
     char *warp_params;
} melfb_t;


typedef struct{
    float32 SAMPLING_RATE;
    int32 FRAME_RATE;
    int32 FRAME_SHIFT;
    float32 WINDOW_LENGTH;
    int32 FRAME_SIZE;
    int32 FFT_SIZE;
    int32 FB_TYPE;
    int32 LOG_SPEC;
    int32 NUM_CEPSTRA;
    int32 FEATURE_DIMENSION;
    float32 PRE_EMPHASIS_ALPHA;
    int16 *OVERFLOW_SAMPS;
    int32 NUM_OVERFLOW_SAMPS;    
    melfb_t *MEL_FB;
    int32 START_FLAG;
    int16 PRIOR;
    float64 *HAMMING_WINDOW;
    int32 FRAME_COUNTER;
} fe_t;

/* Struct to hold the front-end parameters */
typedef struct{
        param_t *P;
        fe_t *FE;
        int16 *fr_data;
        float32 *fr_cep;
} fewrap_t;



#define MEL_SCALE 1
#define LOG_LINEAR 2

#define ON 1
#define OFF 0

/* Default values */
#define DEFAULT_SAMPLING_RATE "16000.0"
#define DEFAULT_FRAME_RATE "100"
#define DEFAULT_FRAME_SHIFT "160"
#define DEFAULT_WINDOW_LENGTH "0.0256" /*0.025625*/
#define DEFAULT_FFT_SIZE "512" /*512*/
#define DEFAULT_FB_TYPE MEL_SCALE
#define DEFAULT_NUM_CEPSTRA "13"
#define DEFAULT_NUM_FILTERS "40"
#define DEFAULT_LOWER_FILT_FREQ "133.33334"
#define DEFAULT_UPPER_FILT_FREQ "6855.4976"
#define DEFAULT_PRE_EMPHASIS_ALPHA "0.97"
#define DEFAULT_START_FLAG 0

#define DEFAULT_WARP_TYPE "inverse_linear"
#define DEFAULT_MEL_WARP "1.0"

#define BB_SAMPLING_RATE 16000
#define DEFAULT_BB_FFT_SIZE 512
#define DEFAULT_BB_FRAME_SHIFT 160
#define DEFAULT_BB_NUM_FILTERS 40
#define DEFAULT_BB_LOWER_FILT_FREQ 133.33334
#define DEFAULT_BB_UPPER_FILT_FREQ 6855.4976

#define NB_SAMPLING_RATE 8000
#define DEFAULT_NB_FFT_SIZE 256 /*512*/
#define DEFAULT_NB_FRAME_SHIFT 80
#define DEFAULT_NB_NUM_FILTERS 31
#define DEFAULT_NB_LOWER_FILT_FREQ 200
#define DEFAULT_NB_UPPER_FILT_FREQ 3500

#define DEFAULT_BLOCKSIZE "200000"
#define DITHER  OFF


/* The following only use in the application level */

#define ON 1
#define OFF 0
#define NULL_CHAR '\0'
#define MAXCHARS 2048

#define WAV 1
#define RAW 2
#define NIST 3
#define MSWAV 4

#define ONE_CHAN "1"

#define LITTLE 1
#define BIG 2

#define FE_SUCCESS 0
#define FE_OUTPUT_FILE_SUCCESS 0
#define FE_CONTROL_FILE_ERROR 1
#define FE_START_ERROR 2
#define FE_UNKNOWN_SINGLE_OR_BATCH 3
#define FE_INPUT_FILE_OPEN_ERROR 4
#define FE_INPUT_FILE_READ_ERROR 5
#define FE_MEM_ALLOC_ERROR 6
#define FE_OUTPUT_FILE_WRITE_ERROR 7
#define FE_OUTPUT_FILE_OPEN_ERROR 8
#define FE_ZERO_ENERGY_ERROR 9

#define COUNT_PARTIAL 1
#define COUNT_WHOLE 0
#define HEADER_BYTES 1024
/*
  #if defined(ALPHA) || defined(ALPHA_OSF1) || defined(alpha_osf1) || defined(__alpha) || defined(mips) 
*/
/*#define SWAPBYTES*/
#define SWAPW(x)        *(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
#define SWAPL(x)        *(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
                        (0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))
#define SWAPF(x)        SWAPL((int *) x)


/* Some defines for MS Wav Files */
/* The MS Wav file is a RIFF file, and has the following 44 byte header */
typedef struct RIFFHeader{
    char rifftag[4];      /* "RIFF" string */
    int32 TotalLength;      /* Total length */
    char wavefmttag[8];   /* "WAVEfmt " string (note space after 't') */
    int32 RemainingLength;  /* Remaining length */
    int16 data_format;    /* data format tag, 1 = PCM */
    int16 numchannels;    /* Number of channels in file */
    int32 SamplingFreq;     /* Sampling frequency */
    int32 BytesPerSec;      /* Average bytes/sec */
    int16 BlockAlign;     /* Block align */
    int16 BitsPerSample;  /* 8 or 16 bit */
    char datatag[4];      /* "data" string */
    int32 datalength;       /* Raw data length */
} MSWAV_hdr;



/* Functions */

fe_t *fe_init(param_t const *P);

int32 fe_start_utt(fe_t *FE);

int32 fe_end_utt(fe_t *FE, float32 *cepvector, int32 *nframes);

int32 fe_close(fe_t *FE);

int32 fe_process(fe_t *FE, int16 *spch, int32 nsamps, float32 ***cep_block);

int32 fe_process_frame(fe_t *FE, int16 *spch, int32 nsamps,float32 *fr_cep);

int32 fe_process_utt(fe_t *FE, int16 *spch, int32 nsamps,
		     float32 ***cep_block, int32 *nframes);

/* Functions that wrap up the front-end operations on the front-end
   wrapper operations.  */

fewrap_t * few_initialize();
param_t *fe_parse_options();
void fe_init_params(param_t *P);
int32 fe_convert_files(param_t *P);
int16 * fe_convert_files_to_spdata(param_t *P, fe_t *FE, int32 *splenp, int32 *nframesp);

int32 fe_build_filenames(param_t *P, char *fileroot, char **infilename, char **outfilename);
char *fe_copystr(char *dest_str, char *src_str);
int32 fe_count_frames(fe_t *FE, int32 nsamps, int32 count_partial_frames);
int32 fe_readspch(param_t *P, char *infile, int16 **spdata, int32 *splen);
int32 fe_writefeat(fe_t *FE, char *outfile, int32 nframes, float32 **feat);
int32 fe_free_param(param_t *P);
int32 fe_openfiles(param_t *P, fe_t *FE, char *infile, int32 *fp_in, int32 *nsamps, 
		   int32 *nframes, int32 *nblocks, char *outfile, int32 *fp_out);
int32 fe_readblock_spch(param_t *P, int32 fp, int32 nsamps, int16 *buf);
int32 fe_writeblock_feat(param_t *P, fe_t *FE, int32 fp, int32 nframes, float32 **feat);
int32 fe_closefiles(int32 fp_in, int32 fp_out);
int32 fe_dither(int16 *buffer,int32 nsamps);


#ifdef __cplusplus
}
#endif

#endif

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.16  2006/02/16  00:18:26  egouvea
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
