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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "fe.h"
#include "fe_internal.h"


/*
  31 Jan 00 mseltzer - changed rounding of filter edges to -not- use 
                        rint() function. 
   3 Dec 99 mseltzer - corrected inverse DCT-2 
                        period is 1/NumFilts not 1/(2*NumFilts)
			added "beta" factor in summation
	  	     - changed mel filter bank construction so that 
                        left,right,center freqs are rounded to DFT 
                        points before filter is constructed
  
*/
                        

int32 fe_build_melfilters(melfb_t *MEL_FB)
{    
    int32 i, whichfilt, start_pt;
    float32 leftfr, centerfr, rightfr, fwidth, height, *filt_edge;
    float32 melmax, melmin, dmelbw, freq, dfreq, leftslope,rightslope;

    /*estimate filter coefficients*/
    MEL_FB->filter_coeffs = (float32 **)fe_create_2d(MEL_FB->num_filters, MEL_FB->fft_size, sizeof(float32));
    MEL_FB->left_apex = (float32 *) calloc(MEL_FB->num_filters,sizeof(float32));
    MEL_FB->width = (int32 *) calloc(MEL_FB->num_filters,sizeof(int32));
    
    if (MEL_FB->doublewide==ON)
	filt_edge = (float32 *) calloc(MEL_FB->num_filters+4,sizeof(float32));
    else	
	filt_edge = (float32 *) calloc(MEL_FB->num_filters+2,sizeof(float32));

    if (MEL_FB->filter_coeffs==NULL || MEL_FB->left_apex==NULL || MEL_FB->width==NULL || filt_edge==NULL){
	fprintf(stderr,"memory alloc failed in fe_build_mel_filters()\n...exiting\n");
	exit(0);
    }
    
    dfreq = MEL_FB->sampling_rate/(float32)MEL_FB->fft_size;
    
    melmax = fe_mel(MEL_FB->upper_filt_freq);
    melmin = fe_mel(MEL_FB->lower_filt_freq);
    dmelbw = (melmax-melmin)/(MEL_FB->num_filters+1);
    
    if (MEL_FB->doublewide==ON){
        for (i=0;i<=MEL_FB->num_filters+3; ++i){
	    filt_edge[i] = fe_melinv(i*dmelbw + melmin);
        }
    }
    else {
	for (i=0;i<=MEL_FB->num_filters+1; ++i){
	    filt_edge[i] = fe_melinv(i*dmelbw + melmin);   
	}
    }
    
    for (whichfilt=0;whichfilt<MEL_FB->num_filters; ++whichfilt)    {
	/*line triangle edges up with nearest dft points... */
	
	if (MEL_FB->doublewide==ON){
	    leftfr   = (float32)((int32)((filt_edge[whichfilt]/dfreq)+0.5))*dfreq;
	    centerfr = (float32)((int32)((filt_edge[whichfilt+2]/dfreq)+0.5))*dfreq;
	    rightfr  = (float32)((int32)((filt_edge[whichfilt+4]/dfreq)+0.5))*dfreq; 
	}
	else{
	    leftfr   = (float32)((int32)((filt_edge[whichfilt]/dfreq)+0.5))*dfreq;
	    centerfr = (float32)((int32)((filt_edge[whichfilt+1]/dfreq)+0.5))*dfreq;
	    rightfr  = (float32)((int32)((filt_edge[whichfilt+2]/dfreq)+0.5))*dfreq;
	}
	
	MEL_FB->left_apex[whichfilt] = leftfr;
	
	fwidth = rightfr - leftfr;
	
	/* 2/fwidth for triangles of area 1 */
	height = 2/(float32)fwidth;
	leftslope = height/(centerfr-leftfr);
	rightslope = height/(centerfr-rightfr);
	
	start_pt = 1 + (int32)(leftfr/dfreq);
	freq = (float32)start_pt*dfreq;
	i=0;

	while (freq<=centerfr){
	    MEL_FB->filter_coeffs[whichfilt][i] = (freq-leftfr)*leftslope;	    
	    freq += dfreq;
	    i++;
	}
	while (freq<rightfr){
	    MEL_FB->filter_coeffs[whichfilt][i] = (freq-rightfr)*rightslope;
	    freq += dfreq;
	    i++;
	}
	
	MEL_FB->width[whichfilt] = i;
    }
    
    free(filt_edge);
    return(0);
}

int32 fe_compute_melcosine(melfb_t *MEL_FB)
{

    float32 period, freq;
    int32 i,j;
    
    period = (float32)2*MEL_FB->num_filters;

    if ((MEL_FB->mel_cosine = (float32 **) fe_create_2d(MEL_FB->num_cepstra,MEL_FB->num_filters,
					      sizeof(float32)))==NULL){
	fprintf(stderr,"memory alloc failed in fe_compute_melcosine()\n...exiting\n");
	exit(0);
    }
    
    
    for (i=0; i<MEL_FB->num_cepstra; i++) {
	freq = 2*(float32)M_PI*(float32)i/period;
	for (j=0;j< MEL_FB->num_filters;j++)
	    MEL_FB->mel_cosine[i][j] = (float32)cos((float64)(freq*(j+0.5)));	
    }    

    return(0);
	
}


float32 fe_mel(float32 x)
{
    return (float32)(2595.0*log10(1.0+x/700.0));
}

float32 fe_melinv(float32 x)
{
    return (float32)(700.0*(pow(10.0,x/2595.0) - 1.0));
}


void fe_pre_emphasis(int16 const *in, float64 *out, int32 len, float32
		     factor, int16 prior)
{
    int32 i;
  
    out[0] = (float64)in[0]-factor*(float64)prior;
    for (i=1; i<len;i++) {
	out[i] = (float64)in[i] - factor*(float64)in[i-1];
    }
 
}

void fe_short_to_double(int16 const *in, float64 *out, int32 len)
{
    int32 i;
    
    for (i=0;i<len;i++)
	out[i] = (float64)in[i];
}

    
void fe_create_hamming(float64 *in, int32 in_len)
{
    int i;
     
    if (in_len>1){
	for (i=0; i<in_len; i++)
	    in[i] = 0.54 - 0.46*cos(2*M_PI*i/((float64)in_len-1.0));
    }
    return;
    
}


void fe_hamming_window(float64 *in, float64 *window, int32 in_len)
{
    int i;
    
    if (in_len>1){
	for (i=0; i<in_len; i++)
	    in[i] *= window[i];
    }
    return;
    
}


void fe_frame_to_fea(fe_t *FE, float64 *in, float64 *fea)
{
    float64 *spec, *mfspec;
    
    if (FE->FB_TYPE == MEL_SCALE){
	spec = (float64 *)calloc(FE->FFT_SIZE, sizeof(float64));
	mfspec = (float64 *)calloc(FE->MEL_FB->num_filters, sizeof(float64));

	if (spec==NULL || mfspec==NULL){
	    fprintf(stderr,"memory alloc failed in fe_frame_to_fea()\n...exiting\n");
	    exit(0);
	}
	
 	fe_spec_magnitude(in, FE->FRAME_SIZE, spec, FE->FFT_SIZE);
	fe_mel_spec(FE, spec, mfspec);
	fe_mel_cep(FE, mfspec, fea);
	
	free(spec);
	free(mfspec);	
    }
    else {
	fprintf(stderr,"MEL SCALE IS CURRENTLY THE ONLY IMPLEMENTATION!\n");
	exit(0);
    }
    
}



void fe_spec_magnitude(float64 const *data, int32 data_len, float64 *spec, int32 fftsize)
{
    int32  j,wrap;
    complex  *FFT, *IN;
    
    /*fftsize defined at top of file*/
    FFT = (complex *) calloc(fftsize,sizeof(complex));
    IN = (complex *) calloc(fftsize,sizeof(complex));
    
    if (FFT==NULL || IN==NULL){
	fprintf(stderr,"memory alloc failed in fe_spec_magnitude()\n...exiting\n");
	exit(0);
    }
	
    if (data_len > fftsize)  /*aliasing */
    {
	
	for (j=0; j<fftsize;j++) {
	    IN[j].r = data[j];
	    IN[j].i = 0.0;
	}
	for (wrap=0; j<data_len; wrap++,j++) {
	    IN[wrap].r += data[j];
	    IN[wrap].i += 0.0;
	}
    }
    else
    {
	for (j=0; j < data_len; j++){
		IN[j].r = data[j];
		IN[j].i = 0.0;
	}
        for ( ;j<fftsize;j++) {  /*pad zeros if necessary */
		IN[j].r = 0.0;
		IN[j].i = 0.0;
	}
    }
    

    fe_fft(IN,FFT,fftsize,FORWARD_FFT);
    
    for (j=0; j <= fftsize/2; j++)
    {	
	spec[j] = FFT[j].r*FFT[j].r + FFT[j].i*FFT[j].i;
    }

    free(FFT);
    free(IN);
    return;
}

void fe_mel_spec(fe_t *FE, float64 const *spec, float64 *mfspec)
{
    int32 whichfilt, start, i;
    float32 dfreq;
    
    dfreq = FE->SAMPLING_RATE/(float32)FE->FFT_SIZE;
    
    for (whichfilt = 0; whichfilt<FE->MEL_FB->num_filters; whichfilt++){
	start = (int32)(FE->MEL_FB->left_apex[whichfilt]/dfreq) + 1;
	mfspec[whichfilt] = 0;
	for (i=0; i< FE->MEL_FB->width[whichfilt]; i++)
	    mfspec[whichfilt] +=
		FE->MEL_FB->filter_coeffs[whichfilt][i]*spec[start+i];	
    }
    
}




void fe_mel_cep(fe_t *FE, float64 *mfspec, float64 *mfcep)
{
    int32 i,j;
    int32 period;
    float32 beta;

    period = FE->MEL_FB->num_filters;

    for (i=0;i<FE->MEL_FB->num_filters; ++i)
    {
	if (mfspec[i]>0)
	    mfspec[i] = log(mfspec[i]);
	else
	    mfspec[i] = -1.0e+5;

    }
    
    for (i=0; i< FE->NUM_CEPSTRA; ++i){
	mfcep[i] = 0;
	for (j=0;j<FE->MEL_FB->num_filters; j++){
	    if (j==0)
		beta = 0.5;
	    else
		beta = 1.0;
	    mfcep[i] += beta*mfspec[j]*FE->MEL_FB->mel_cosine[i][j];
	}
		mfcep[i] /= (float32)period;
    }
    return;
}

int32 fe_fft(complex const *in, complex *out, int32 N, int32 invert)
{
  static int32
    s, k,			/* as above				*/
    lgN;			/* log2(N)				*/
  complex
    *f1, *f2,			/* pointers into from array		*/
    *t1, *t2,			/* pointers into to array		*/
    *ww;			/* pointer into w array			*/
  static complex
    *w, *from, *to,		/* as above				*/
    wwf2,			/* temporary for ww*f2			*/
    *buffer,			/* from and to flipflop btw out and buffer */
    *exch,			/* temporary for exchanging from and to	*/
    *wEnd;			/* to keep ww from going off end	*/
  static float64
    div,			/* amount to divide result by: N or 1	*/
    x;				/* misc.				*/

  
  /* check N, compute lgN						*/
  for (k = N, lgN = 0; k > 1; k /= 2, lgN++)
  {
    if (k%2 != 0 || N < 0)
    {
      fprintf(stderr, "fft: N must be a power of 2 (is %d)\n", N);
      return(-1);
    }
  }

  /* check invert, compute div						*/
  if (invert == 1)
    div = 1.0;
  else if (invert == -1)
    div = N;
  else
  {
    fprintf(stderr, "fft: invert must be either +1 or -1 (is %d)\n", invert);
    return(-1);
  }

  /* get the to, from buffers right, and init				*/
  buffer = (complex *)calloc(N, sizeof(complex));
  if (lgN%2 == 0)
  {
    from = out;
    to = buffer;
  }
  else
  {
    to = out;
    from = buffer;
  }

  
  for (s = 0; s<N; s++)
  {
      from[s].r = in[s].r/div;
      from[s].i = in[s].i/div;

  }

  /* w = exp(-2*PI*i/N), w[k] = w^k					*/
  w = (complex *) calloc(N/2, sizeof(complex));
  for (k = 0; k < N/2; k++)
  {
    x = -6.28318530717958647*invert*k/N;
    w[k].r = cos(x);
    w[k].i = sin(x);
  }
  wEnd = &w[N/2];
  
  /* go for it!								*/
  for (k = N/2; k > 0; k /= 2)
  {
    for (s = 0; s < k; s++)
    {
      /* initialize pointers						*/
      f1 = &from[s]; f2 = &from[s+k];
      t1 = &to[s]; t2 = &to[s+N/2];
      ww = &w[0];
      /* compute <s,k>							*/
      while (ww < wEnd)
      {
        /* wwf2 = ww*f2							*/
        wwf2.r = f2->r*ww->r - f2->i*ww->i;
        wwf2.i = f2->r*ww->i + f2->i*ww->r;
        /* t1 = f1+wwf2							*/
        t1->r = f1->r + wwf2.r;
        t1->i = f1->i + wwf2.i;
        /* t2 = f1-wwf2							*/
        t2->r = f1->r - wwf2.r;
        t2->i = f1->i - wwf2.i;
        /* increment							*/
        f1 += 2*k; f2 += 2*k;
        t1 += k; t2 += k;
        ww += k;
      }
    }
    exch = from; from = to; to = exch;
  }
  free(buffer);
  free(w);
  return(0);
}



char **fe_create_2d(int32 d1, int32 d2, int32 elem_size)
{
    char *store;
    char **out;
    int32 i, j;
    store = calloc(d1 * d2, elem_size);

    if (store == NULL) {
	fprintf(stderr,"fe_create_2d failed\n");
	return(NULL);
    }
    
    out = calloc(d1, sizeof(void *));

    if (out == NULL) {
	fprintf(stderr,"fe_create_2d failed\n");
	free(store);
	return(NULL);
    }
    
    for (i = 0, j = 0; i < d1; i++, j += d2) {
	out[i] = &((char *)store)[j*elem_size];
    }

    return out;
}

void fe_free_2d(void **arr)
{
    if (arr!=NULL){
	free(arr[0]);
	free(arr);
    }
    
}

void fe_parse_general_params(param_t const *P, fe_t *FE)
{

    if (P->SAMPLING_RATE != 0) 
      FE->SAMPLING_RATE = P->SAMPLING_RATE;
    else
      FE->SAMPLING_RATE = (float32)atof(DEFAULT_SAMPLING_RATE);

    if (P->FRAME_RATE != 0) 
      FE->FRAME_RATE = P->FRAME_RATE;
    else 
      FE->FRAME_RATE = (int32)atof(DEFAULT_FRAME_RATE);
    
    if (P->WINDOW_LENGTH != 0) 
	FE->WINDOW_LENGTH = P->WINDOW_LENGTH;
    else 
      FE->WINDOW_LENGTH = (float32)atof(DEFAULT_WINDOW_LENGTH);
    
    if (P->FB_TYPE != 0) 
      FE->FB_TYPE = P->FB_TYPE;
    else 
      FE->FB_TYPE = DEFAULT_FB_TYPE;
 
    if (P->PRE_EMPHASIS_ALPHA != 0) 
      FE->PRE_EMPHASIS_ALPHA = P->PRE_EMPHASIS_ALPHA;
    else 
      FE->PRE_EMPHASIS_ALPHA = (float32)atof(DEFAULT_PRE_EMPHASIS_ALPHA);
 
    if (P->NUM_CEPSTRA != 0) 
      FE->NUM_CEPSTRA = P->NUM_CEPSTRA;
    else 
      FE->NUM_CEPSTRA = atoi(DEFAULT_NUM_CEPSTRA);

    if (P->FFT_SIZE != 0) 
      FE->FFT_SIZE = P->FFT_SIZE;
    else 
      FE->FFT_SIZE = atoi(DEFAULT_FFT_SIZE);
 
}

void fe_parse_melfb_params(param_t const *P, melfb_t *MEL)
{
    if (P->SAMPLING_RATE != 0) 
      MEL->sampling_rate = P->SAMPLING_RATE;
    else 
      MEL->sampling_rate = (float32)atof(DEFAULT_SAMPLING_RATE);

    if (P->FFT_SIZE != 0) 
      MEL->fft_size = P->FFT_SIZE;
    else {
      if (MEL->sampling_rate == BB_SAMPLING_RATE)
	MEL->fft_size = DEFAULT_BB_FFT_SIZE;
      if (MEL->sampling_rate == NB_SAMPLING_RATE)
	MEL->fft_size = DEFAULT_NB_FFT_SIZE;
      else 
	MEL->fft_size = atoi(DEFAULT_FFT_SIZE);
    }
 
    if (P->NUM_CEPSTRA != 0) 
      MEL->num_cepstra = P->NUM_CEPSTRA;
    else 
      MEL->num_cepstra = atoi(DEFAULT_NUM_CEPSTRA);
 
    if (P->NUM_FILTERS != 0)	
	MEL->num_filters = P->NUM_FILTERS;
    else {
      if (MEL->sampling_rate == BB_SAMPLING_RATE)
	MEL->num_filters = DEFAULT_BB_NUM_FILTERS;
      else if (MEL->sampling_rate == NB_SAMPLING_RATE)
	MEL->num_filters = DEFAULT_NB_NUM_FILTERS;
      else {
	fprintf(stderr,"Please define the number of MEL filters needed\n");
	fprintf(stderr,"Modify include/new_fe.h and new_fe_sp.c\n");
	fflush(stderr); exit(0);
      }
    }

    if (P->UPPER_FILT_FREQ != 0)	
      MEL->upper_filt_freq = P->UPPER_FILT_FREQ;
    else{
      if (MEL->sampling_rate == BB_SAMPLING_RATE)
	MEL->upper_filt_freq = (float32) DEFAULT_BB_UPPER_FILT_FREQ;
      else if (MEL->sampling_rate == NB_SAMPLING_RATE)
	MEL->upper_filt_freq = (float32) DEFAULT_NB_UPPER_FILT_FREQ;
      else {
	fprintf(stderr,"Please define the upper filt frequency needed\n");
	fprintf(stderr,"Modify include/new_fe.h and new_fe_sp.c\n");
	fflush(stderr); exit(0);
      }
    } 

    if (P->LOWER_FILT_FREQ != 0)	
      MEL->lower_filt_freq = P->LOWER_FILT_FREQ;
    else {
      if (MEL->sampling_rate == BB_SAMPLING_RATE)
	MEL->lower_filt_freq = (float32) DEFAULT_BB_LOWER_FILT_FREQ;
      else if (MEL->sampling_rate == NB_SAMPLING_RATE)
	MEL->lower_filt_freq = (float32) DEFAULT_NB_LOWER_FILT_FREQ;
      else {
	fprintf(stderr,"Please define the lower filt frequency needed\n");
	fprintf(stderr,"Modify include/new_fe.h and new_fe_sp.c\n");
	fflush(stderr); exit(0);
      }
    } 

    if (P->doublebw == ON)
	MEL->doublewide = ON;
    else
	MEL->doublewide = OFF;

}

void fe_print_current(fe_t *FE)
{
    fprintf(stderr,"Current FE Parameters:\n");
    fprintf(stderr,"\tSampling Rate:             %f\n",FE->SAMPLING_RATE);
    fprintf(stderr,"\tFrame Size:                %d\n",FE->FRAME_SIZE);
    fprintf(stderr,"\tFrame Shift:               %d\n",FE->FRAME_SHIFT);
    fprintf(stderr,"\tFFT Size:                  %d\n",FE->FFT_SIZE);
    fprintf(stderr,"\tNumber of Overflow Samps:  %d\n",FE->NUM_OVERFLOW_SAMPS);
    fprintf(stderr,"\tStart Utt Status:          %d\n",FE->START_FLAG);
}

