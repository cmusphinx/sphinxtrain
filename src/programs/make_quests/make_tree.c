/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * $Log$
 * Revision 1.6  2005/07/09  03:13:03  arthchan2003
 * Fix keyword expansion probelm
 * 
 *  Revision 1.5  2005/07/09 02:31:47  arthchan2003
 * 1, When user forgot to specify -type, make_quest.c failed to check whether type is valid, when passed as an argument to strcpy, strcpy will cause seg core.  Resolved it by explicitly adding a checking and prompting user to specify it correctly.  2, Also added  keyword for all .c files.
 *
 */

#include <math.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <s3/s3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MINVAR 1e-08
int32  findclosestpair(float32***,float32***,float32**,int32,int32,
			int32,int32*,int32*);
float32 likelhddec(float32*,float32*,float32*,float32*,float32,float32,int32);


int make_tree(float32 **mixw,	  // Mixture weights for [phone][state]
	      float32 ***means,  // Means for [phone][state][dim]
	      float32 ***vars,   // Variances for [phone][state][dim]
	      int32   nphones,	  // Total no. of phones
	      char    **phones,   // Identities of the phones
	      int32   nstates,	  // No. of states per phone
              int32   stt,    	  // State we are building tree for
	      int32   ndim	  // Dimensionality of feature set
	      )
{
    float32  ***oldmeans, ***oldvars, ***newmeans, ***newvars;
    float32  **oldmixw, **newmixw, ***tmp3d, **tmp2d;
    char     **phoneid, **newphoneid, **tmpstr;
    int32    i,j,l,a,b,set,nsets;

    oldmixw = (float32 **) ckd_calloc_2d(nphones,nstates,sizeof(float32));
    oldmeans = (float32***) ckd_calloc_3d(nphones,nstates,ndim,sizeof(float32));
    oldvars = (float32 ***) ckd_calloc_3d(nphones,nstates,ndim,sizeof(float32));
    phoneid = (char **)ckd_calloc_2d(nphones,2048,sizeof(char));
    newmixw = (float32 **) ckd_calloc_2d(nphones,nstates,sizeof(float32));
    newmeans = (float32***) ckd_calloc_3d(nphones,nstates,ndim,sizeof(float32));
    newvars = (float32 ***) ckd_calloc_3d(nphones,nstates,ndim,sizeof(float32));
    newphoneid = (char **)ckd_calloc_2d(nphones,2048,sizeof(char));

    for (i=0;i<nphones;i++){
        sprintf(phoneid[i],"%s",phones[i]); //Phone ids
        for (j=0;j<nstates;j++){
            oldmixw[i][j] = mixw[i][j];
            for (l=0;l<ndim;l++){
                oldmeans[i][j][l] = means[i][j][l];
                oldvars[i][j][l] = vars[i][j][l];
            }
        }
    }

    for (nsets = nphones; nsets > 2; nsets--) {
        // Find the closest distributions
        findclosestpair(oldmeans,oldvars,oldmixw,nsets,stt,ndim,&a,&b);
	printf("Merging %s %s\n",phoneid[a],phoneid[b]); fflush(stdout);

        // Copy and Merge distributions...
        // Copy unmerged distributions first
        for (i=0,set=0;i<nsets;i++){
            if (i != a && i != b){
                sprintf(newphoneid[set],"%s",phoneid[i]); //Phone ids
                newmixw[set][stt] = oldmixw[i][stt];
                for (l=0;l<ndim;l++){
                    newmeans[set][stt][l] = oldmeans[i][stt][l];
                    newvars[set][stt][l] = oldvars[i][stt][l];
                }
                set++;
            }
        }
        // Merge a and b
        sprintf(newphoneid[set],"%s_%s",phoneid[a],phoneid[b]);
        {
	    float32 *nm = newmeans[set][stt];
	    float32 *nv = newvars[set][stt];
	    float32 *oma = oldmeans[a][stt];
	    float32 *ova = oldvars[a][stt];
	    float32 *omb = oldmeans[b][stt];
	    float32 *ovb = oldvars[b][stt];
            float32 cnta, cntb;

            cnta = oldmixw[a][stt]; cntb = oldmixw[b][stt];
            newmixw[set][stt] = cnta + cntb;
            for (l=0;l<ndim;l++){
                nm[l] = (cnta*oma[l] + cntb*omb[l]) / (cnta + cntb);
                nv[l] = cnta*(ova[l]+oma[l]*oma[l])+cntb*(ovb[l]+omb[l]*omb[l]);
                nv[l] = nv[l]/(cnta+cntb) - nm[l]*nm[l];
                if (nv[l] < MINVAR) nv[l] = MINVAR;
            }
        }

        // Switch old and new variables
        tmp3d = oldmeans; oldmeans = newmeans; newmeans = tmp3d;
        tmp3d = oldvars;  oldvars = newvars;   newvars = tmp3d;
        tmp2d = oldmixw;  oldmixw = newmixw;   newmixw = tmp2d;
        tmpstr = phoneid; phoneid = newphoneid; newphoneid = tmpstr;
    }
    ckd_free_3d((void ***)oldmeans); ckd_free_3d((void ***)oldvars);
    ckd_free_3d((void ***)newmeans); ckd_free_3d((void ***)newvars);
    ckd_free_2d((void **)oldmixw); ckd_free_2d((void **)newmixw);
    ckd_free_2d((void **)newphoneid); ckd_free_2d((void **)phoneid);
    
    return 0;
}


/* Find the two closest distributions. We assume 1 gaussian/state */

int32  findclosestpair(float32 ***pmeans, float32 ***pvars,
                       float32 **pmixw, int32 nsets, int32 stt, int32 dim,
                       int *a, int32 *b)
{
    float32    reduction, minreduction;
    int32      i, j, la=0, lb=0;

    minreduction = 1.0e+32;
    for (i=0; i<nsets; i++){
        for (j=i+1;j<nsets;j++){
            if (i != j){
                reduction = likelhddec(pmeans[i][stt],pvars[i][stt],
                                       pmeans[j][stt],pvars[j][stt],
                                       pmixw[i][stt],pmixw[j][stt],
                                       dim);
                if (reduction < minreduction){
                    minreduction = reduction;
                    la = i; lb = j;
                }
            }
        }
    }
    *a = la; *b = lb;
    return 0;
}

float32    likelhddec(float32 *meana, float32 *vara,
                      float32 *meanb, float32 *varb,
                      float32 cnta, float32 cntb,
                      int32 dim)
{
    int32   i;
    float32 cntc, la, lb, lc, nm, nv, lkdec;

    cntc = cnta + cntb;
    for (i=0, lc=0, lb=0, la=0;i<dim;i++){
        nm = (cnta*meana[i] + cntb*meanb[i])/(cnta+cntb);
        nv = cnta*(vara[i]+meana[i]*meana[i])+cntb*(varb[i]+meanb[i]*meanb[i]);
        nv = nv/(cnta+cntb) - nm*nm;
        if (nv < MINVAR) nv = MINVAR;
        lc += (float32)log(nv);
        lb += (float32)log(varb[i]);
        la += (float32)log(vara[i]);
    }
    lkdec = 0.5*(cntc*lc - cntb*lb - cnta*la);

    return(lkdec);
}
        
