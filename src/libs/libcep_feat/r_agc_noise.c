/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: r_agc_noise.c
 * 
 * Description: 
 * 
 * Author: 
 * 	faa
 *********************************************************************/

/* static char rcsid[] = "@(#)$Id$";*/

#include <s3/r_agc_noise.h>
#include <s3/prim_type.h>	/* defines int32, etc. */
#include <s3/s3.h>	/* defines TRUE and FALSE */

#include <stdio.h>
#include <string.h>

static float agc_thresh = 0.2;

void
real_agc_noise (float *cep,		/* The cepstrum data */
		uint32 fcnt,	/* Number of cepstrum frame availiable */
		uint32 cf_cnt)	/* Number of coeff's per frame */
{
    float *p;			/* Data pointer */
    float min_energy;		/* Minimum log-energy */
    float noise_level;		/* Average noise_level */
    uint32 i;		/* frame index */
    uint32 noise_frames;/* Number of noise frames */

    /* Determine minimum log-energy in utterance */
    min_energy = *cep;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
	if (*p < min_energy)
	    min_energy = *p;
    }

    /* Average all frames between min_energy and min_energy + agc_thresh */
    noise_frames = 0;
    noise_level = 0.0;
    min_energy += agc_thresh;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
	if (*p < min_energy) {
	    noise_level += *p;
	    noise_frames++;
	}
    }
    noise_level /= noise_frames;

    printf ("%6.3f = AGC NOISE\n", noise_level);

    /* Subtract noise_level from all log_energy values */
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt)
	*p -= noise_level;
}

void
agc_set_threshold (float threshold)
{
    agc_thresh = threshold;
}

uint32 find_peak (uint32 *buf,
	   uint32 cnt)
{
    uint32 i, max, maxi, maxcnt;
    uint32 hyst;	/* hysteriesus band */

    maxcnt = 0;
    for (i = 0; i < cnt; i++)
	if (maxcnt < buf[i])
	    maxcnt = buf[i];

    /*
     * Peak must exceed 20% of the max count.
     */
    hyst = 0.20 * maxcnt;
    max = 0;
    maxi = 0;
    for (i = 0; i < cnt; i++) {
  	if (buf[i] > max) {
	    max = buf[i];
	    maxi = i;
	}
	if (buf[i] < (max - hyst))
	    break;
    }

    return (maxi);
}

float histo_noise_level (float *cep,	/* The cepstrum data */
			 int32 fcnt,	/* Number of cepstrum frame availiable */
			 int32 cf_cnt)	/* Number of coeff's per frame */
{
    float *p;			/* Data pointer */
    float min_energy;		/* Minimum log-energy */
    float max_energy;		/* Maximum log-energy */
    float dr;			/* Dynamic range */
    float noise_level;		/* Average noise_level */
    int32 i;		/* frame index */
    uint32 histo[101];
    uint32 idx;

    /* Determine minimum/maximum log-energy in utterance 
     * Plus a histogram of the energy
     */
    for (idx = 0; idx < 101; idx++)
	histo[idx] = 0;

    min_energy = *cep;
    max_energy = *cep;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
	if (*p < min_energy)
	    min_energy = *p;
	else
	    if (*p > max_energy)
		max_energy = *p;
    }

    dr = max_energy - min_energy;

    if (dr == 0.0)
	return (min_energy);

    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
	idx = ((*p - min_energy) / dr) * 100.0;
	histo[idx]++;
    }
    
    idx = 5 + find_peak (histo, 101);
    if (idx > 100)
	idx = 100;
    noise_level = idx;
    noise_level = (noise_level * dr / 100.0) + min_energy;

    printf ("%.3f = Histo noise (%d)\n", noise_level, idx);

    return (noise_level);
}

int
delete_background (float32 *cep,	/* The cepstrum data */
		   int32 fcnt,		/* Number of cepstrum frame availiable */
		   int32 cf_cnt,	/* Number of coeff's per frame */
		   float32 thresh)	/* background threshold */
{
    uint32 i, j;
    float32 *p;
    static char delete[8000];

    if (cf_cnt > 8000) {
	printf ("%s(%d): number frames %d, exceeds max (8000)\n",
		__FILE__, __LINE__, fcnt);
	return fcnt;
    }

    /*
     * Mark the frames that are below the threshold
     */
    for (p = cep, i = 0; i < fcnt; i++, p += cf_cnt) {
	delete[i] = (*p < thresh) ? 1 : 0;
    }

    /*
     * Select the frames to delete
     */
    for (i = 2; i < fcnt-2; i++) {
	if (delete[i-2] &&
	    delete[i-1] &&
	    delete[i]   &&
	    delete[i+1] &&
	    delete[i+2]) {
	    delete[i] = 2;
	}
    }

    /*
     * Delete the frames
     */
    for (j = i = 0; i < fcnt; i++) {
	if (delete[i] != 2) {
	    memcpy (&cep[j*cf_cnt], &cep[i*cf_cnt], cf_cnt * sizeof(float));
	    j++;
	}
    }
    printf ("Deleted %d background frames out of %d frames\n", fcnt-j, fcnt);
    return (j);
}

static float observed_min = 100.0;
static float observed_max = -100.0;
static float observed_dr;
static float min_energy = -20.0;
static float max_energy =  20.0;
static float dynamic_range = 40.0;
static uint32 histo[1001];
static float noise_level = -100.0;       /* Average noise_level */
static int32 noise_frm_cnt = 0;
static int32 noise_frames_discarded = 0;

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )

int
histo_add_c0 (float32 c0)
{
    uint32 idx;

    if (c0 < noise_level)
	noise_frm_cnt++;
    else
	noise_frm_cnt = 0;

    observed_min = MIN(c0,observed_min);
    observed_max = MAX(c0,observed_max);

    c0 = MAX (min_energy, c0);
    c0 = MIN (max_energy, c0);
    c0 -= min_energy;

    idx = (c0 / dynamic_range) * 1000.0;
    histo[idx]++;

    if (noise_frm_cnt < 5)
	return TRUE;
    else {
	noise_frames_discarded++;
	return FALSE;
    }
}

void
compute_noise_level ()
{
    uint32 idx;
    uint32 i;

    idx = find_peak (histo, 1001);
    /*
     * Discard half of the counts
     */
    for (i = 0; i < 1001; i++)
	histo[i] = histo[i] >> 1;
    /*
     * compute the observed dynamic range and contract the observed min and
     * max by 10% each. Reset noise_frm_cnt to 0.
     */
    observed_dr = observed_max - observed_min;
    observed_min += observed_dr / 10.0;
    observed_max -= observed_dr / 10.0;
    noise_frm_cnt = 0;

    /*
     * Compute the noise level as 5% of the dr above the first peak.
     */
    noise_level = idx;
    noise_level = (noise_level * dynamic_range / 1000.0) + min_energy;
    noise_level += (observed_dr * 0.05);

    printf ("%.3f = Histo noise (%d)\n", noise_level, idx);
    printf ("%d Frames discarded\n", noise_frames_discarded);
    noise_frames_discarded = 0;
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/09/24  21:38:31  awb
 * *** empty log message ***
 * 
 * Revision 1.4  1995/10/17  13:05:04  eht
 * Cleaned up code a bit so that it is more ANSI compliant
 *
 * Revision 1.3  1995/10/10  12:36:12  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/09/07  19:55:41  eht
 * include <s3/s3.h> to pick up define for TRUE and FALSE
 * since some machines don't choose to define these in
 * their standard include files
 *
 * Revision 1.1  1995/06/02  20:57:22  eht
 * Initial revision
 *
 *
 */
