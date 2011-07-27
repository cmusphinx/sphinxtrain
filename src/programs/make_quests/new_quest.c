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
 * 
 * Revision 1.5  2005/07/09 02:31:47  arthchan2003
 *
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


int32  findclosestpair(float32**,float32**,float32*,int32,
			int32,int32*,int32*);
float32 likelhddec(float32*,float32*,float32*,float32*,float32,float32,int32);
int compare();

typedef struct node_str {int32  nphones; 
                     int32  *phoneids; 
		     struct node_str *left; 
		     struct node_str *right;
		    } node;


int
make_tree (float32 **means, float32 **vars, float32 *mixw, 
           int32 *nodephoneids, int32 nphones, int32 ndim,
	   node *root, int32 npermute)
{
    float32  **oldmeans, **oldvars, **newmeans, **newvars;
    float32  *oldmixw, *newmixw, **tmp2d, *tmp1d;
    float32  *meana, *vara, *meanb, *varb; 
    float32  cnt, counta, countb, bestdec, reduction;
    int32    **phoneid, **newphoneid, *numphones, *newnumphones, **it2d, *it1d;
    int32    i,j,k,l,a,b,set,nsets,ncombinations,bestclust=0;
    char     **identifier, *tmpid;
    node     *left, *right;

    oldmixw = (float32 *) ckd_calloc(nphones,sizeof(float32));
    oldmeans = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
    oldvars = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
    phoneid = (int32 **)ckd_calloc_2d(nphones,nphones,sizeof(int32));
    numphones = (int32 *) ckd_calloc(nphones,sizeof(int32));

    newmixw = (float32 *) ckd_calloc(nphones,sizeof(float32));
    newmeans = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
    newvars = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
    newphoneid = (int32 **)ckd_calloc_2d(nphones,nphones,sizeof(int32));
    newnumphones = (int32 *) ckd_calloc(nphones,sizeof(int32));

    for (i=0;i<nphones;i++){
        numphones[i] = 1;
        phoneid[i][0] =  nodephoneids[i]; //Phone ids
        oldmixw[i] = mixw[nodephoneids[i]];
        for (l=0;l<ndim;l++){
            oldmeans[i][l] = means[nodephoneids[i]][l];
            oldvars[i][l] = vars[nodephoneids[i]][l];
        }
    }
    if (nphones > npermute){

        for (nsets = nphones; nsets > npermute; nsets--) {
            // Find the closest distributions
            findclosestpair(oldmeans,oldvars,oldmixw,nsets,ndim,&a,&b);
	    // printf("Merging %s %s\n",phoneid[a],phoneid[b]); fflush(stdout);

            // Copy and Merge distributions...
            // Copy unmerged distributions first
            for (i=0,set=0;i<nsets;i++){
                if (i != a && i != b){
                    newnumphones[set] = numphones[i];
                    for (l=0;l<numphones[i];l++)
                        newphoneid[set][l] = phoneid[i][l];
                    newmixw[set] = oldmixw[i];
                    for (l=0;l<ndim;l++){
                        newmeans[set][l] = oldmeans[i][l];
                        newvars[set][l] = oldvars[i][l];
                    }
                    set++;
                }
            }
            // Merge a and b
            newnumphones[set] =  numphones[a]+numphones[b];
            for (i=0;i<numphones[a];i++)
                newphoneid[set][i] = phoneid[a][i];
            for (l=0;l<numphones[b];l++,i++)
                newphoneid[set][i] = phoneid[b][l];
            {
	        float32 *nm = newmeans[set];
	        float32 *nv = newvars[set];
	        float32 *oma = oldmeans[a];
	        float32 *ova = oldvars[a];
	        float32 *omb = oldmeans[b];
	        float32 *ovb = oldvars[b];
                float32 cnta, cntb;

                cnta = oldmixw[a]; cntb = oldmixw[b];
                newmixw[set] = cnta + cntb;
                for (l=0;l<ndim;l++){
                    nm[l] = (cnta*oma[l] + cntb*omb[l]) / (cnta + cntb);
                    nv[l] = cnta*(ova[l]+oma[l]*oma[l]) +
                            cntb*(ovb[l]+omb[l]*omb[l]);
                    nv[l] = nv[l]/(cnta+cntb) - nm[l]*nm[l];
                    if (nv[l] < MINVAR) nv[l] = MINVAR;
                }
            }

            // Switch old and new variables
            tmp2d = oldmeans; oldmeans = newmeans; newmeans = tmp2d;
            tmp2d = oldvars;  oldvars = newvars;   newvars = tmp2d;
            tmp1d = oldmixw;  oldmixw = newmixw;   newmixw = tmp1d;
            it2d = phoneid;  phoneid = newphoneid; newphoneid = it2d;
            it1d = numphones; numphones = newnumphones; newnumphones = it1d;
        }
    }
    else npermute = nphones;

    if (npermute <= 2){ 
        root->left = root->right = NULL; /* Dont split further */
        return 0;
    }

    // We have npermute clusters now; permute them to get two clusters
    // There are 2^(npermute-1)-1 clusters possible. Test them all out.

    // Create identifiers for 2^(npermute-1) clusters
    for (i=1,ncombinations=1;i<npermute;i++,ncombinations*=2);
    identifier = (char **)ckd_calloc_2d(ncombinations,npermute,sizeof(char));
    tmpid = (char *)ckd_calloc(npermute,sizeof(char));
    for (i=0;i<ncombinations-1;i++){
        for(j=0,tmpid[0]=!tmpid[0];!tmpid[j];j++,tmpid[j]=!tmpid[j]);
        for(j=0;j<npermute;j++) identifier[i][j] = tmpid[j];
    }
    ckd_free(tmpid);

    // Go through the list and find best pair
    for (i=0,bestdec=-1.0e+30;i<ncombinations-1;i++){
        meana = (float32 *)ckd_calloc(ndim,sizeof(float32));
        vara = (float32 *)ckd_calloc(ndim,sizeof(float32));
        meanb = (float32 *)ckd_calloc(ndim,sizeof(float32));
        varb = (float32 *)ckd_calloc(ndim,sizeof(float32));
        counta = countb = 0;
        for (j=0;j<npermute;j++){
            float32 *om = oldmeans[j];
            float32 *ov = oldvars[j];
            cnt = oldmixw[j];
            if (identifier[i][j]){
                counta += cnt;
                for (k=0;k<ndim;k++){
                    meana[k] += cnt * om[k];
                    vara[k] += cnt*(ov[k] + om[k]*om[k]);
                }
            }
            else{
                countb += cnt;
                for (k=0;k<ndim;k++){
                    meanb[k] += cnt * om[k];
                    varb[k] += cnt*(ov[k] + om[k]*om[k]);
                }
            }
        }
        for (k=0;k<ndim;k++){
            meana[k] /= counta; meanb[k] /= countb;
            vara[k] = vara[k]/counta - meana[k]*meana[k];
            varb[k] = varb[k]/countb - meanb[k]*meanb[k];
        }
        reduction = likelhddec(meana,vara, meanb,varb,counta,countb,ndim);
        if (reduction > bestdec) {
            bestdec = reduction;
            bestclust = i;
        }
        ckd_free(meana);ckd_free(vara);ckd_free(meanb);ckd_free(varb);
    }

    // Now we know what the best separation is, set the appropriate left
    // and right trees.

    left = (node *) ckd_calloc(1,sizeof(node));
    right = (node *) ckd_calloc(1,sizeof(node));
    root->left = left;
    root->right = right;
    left->phoneids = (int32 *) ckd_calloc(nphones,sizeof(int32)); //Overalloc
    right->phoneids = (int32 *) ckd_calloc(nphones,sizeof(int32));
    for (j=0;j<npermute;j++){
        if (identifier[bestclust][j]){
            for (l=0;l<numphones[j];l++,left->nphones++)
                left->phoneids[left->nphones] = phoneid[j][l];
        }
        else {
            for (l=0;l<numphones[j];l++,right->nphones++)
                right->phoneids[right->nphones] = phoneid[j][l];
        }
    }

    ckd_free_2d((void **)identifier);    
    ckd_free_3d((void ***)oldmeans); ckd_free_3d((void ***)oldvars);
    ckd_free_3d((void ***)newmeans); ckd_free_3d((void ***)newvars);
    ckd_free_2d((void **)oldmixw); ckd_free_2d((void **)newmixw);
    ckd_free_2d((void **)newphoneid); ckd_free_2d((void **)phoneid);

    // Recurse
    make_tree(means,vars,mixw,left->phoneids,left->nphones,ndim,
              left,npermute);
    make_tree(means,vars,mixw,right->phoneids,right->nphones,ndim,
          right,npermute);
        
    return 0;
}


/* Find the two closest distributions. We assume 1 gaussian/state */

int32  findclosestpair(float32 **pmeans, float32 **pvars,
                       float32 *pmixw, int32 nsets, int32 dim,
                       int *a, int32 *b)
{
    float32    reduction, minreduction;
    int32      i, j, la=0, lb=0;

    minreduction = 1.0e+32;
    for (i=0; i<nsets; i++){
        for (j=i+1;j<nsets;j++){
            if (i != j){
                reduction = likelhddec(pmeans[i],pvars[i],
                                       pmeans[j],pvars[j],
                                       pmixw[i],pmixw[j],
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
 

void free_tree(node *root)
{
    if (root->left != NULL)
        free_tree(root->left);
    if (root->right != NULL)
        free_tree(root->right);

    ckd_free((void *) root->phoneids);
    ckd_free((void *) root);
    return;
}


void   print_tree(node *root, char **phonelist, char *outfile)
{
    FILE  *fp;
    int32 *sortedidx,i;

    if (root->nphones < 2) return;
    if (root->left != NULL) print_tree(root->left, phonelist, outfile);
    if (root->right != NULL) print_tree(root->right, phonelist, outfile);

    fp = fopen(outfile,"a");
    sortedidx = (int32 *)ckd_calloc(root->nphones,sizeof(int32));
    qsort(sortedidx,root->nphones,sizeof(int32),compare);
    for (i=0;i<root->nphones;i++) sortedidx[i] = root->phoneids[i];

    for (i=0;i<root->nphones;i++)
        fprintf(fp," %s",phonelist[sortedidx[i]]);
    fprintf(fp,"\n");
    fclose(fp);
 
    ckd_free((void *)sortedidx);
    return;
}

int compare(int *i, int *j)
{
    if (*i > *j) return -1;
    if (*j > *i) return 1;
    return 0;
}
