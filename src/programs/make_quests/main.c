/* -*- c-basic-offset: 4 -*- */
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
/*********************************************************************
 * Clusters CI distributions using a hybrid bottom-up top-down
 * clustering algorithm to build linguistic questions for
 * decision trees.
 *
 * Author: Rita Singh
 *********************************************************************/

/*
 * 
 * $Log$
 * Revision 1.11  2005/07/09  03:13:03  arthchan2003
 * Fix keyword expansion probelm
 * 
 * Revision 1.10  2005/07/09 03:09:47  arthchan2003
 * Fixed typos in error message, added initialization for the variable continuous such that gcc is happy.
 *
 * Revision 1.9  2005/07/09 03:02:10  arthchan2003
 * 1, Remove tempfn and anything the used tempfn. It is never used in
 * the entire SphinxTrain codebase.  2, change the example such that
 * -tempfn is removed but -type .cont. is added. 3, Did **not**
 * removed -tempfn because some users might just update the code but
 * not the perl script.  This keep backward compatibility (but it is
 * definitely stupid). 4, Change the perl script as well. People who
 * update the code and script will then learn the correct usage. 5,
 * Check type such that if it is not .cont. or .semi., nothing stupid
 * will happen. (Well, in general, it is a sin for us to release this
 * program. *sigh*)
 * Revision 1.8 2005/07/09 02:31:47 arthchan2003 1, When user forgot
 *  to specify -type, make_quest.c failed to check whether type is
 *  valid, when passed as an argument to strcpy, strcpy will cause seg
 *  core.  Resolved it by explicitly adding a checking and prompting
 *  user to specify it correctly.  2, Also added keyword for all .c
 *  files.
 */

#include <math.h>
#include "parse_cmd_ln.h"

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <s3/metric.h>
#include <s3/div.h>
#include <s3/s3.h>
#include <s3/vector.h>
#include <s3/s3gau_io.h>
#include <s3/gauden.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define  IS_LEAF(X) ((X != NULL) && (X->left==NULL && X->right==NULL))
#define  IS_TWIG(X) ((X != NULL) && (IS_LEAF(X->left)&&IS_LEAF(X->right)))
#define  IS_QUEST(X) ((X->depth > 0) && ((X->nphones > 1) || (X->depth <= 3)))

typedef struct node_str {int32  nphones;
                     int32   *phoneids;
                     int32   depth;
		     int32   id;
                     float32 lkhd_dec;
                     struct node_str *left;
                     struct node_str *right;
                    } node;

float32    likelhddec(float32 *meana, float32 *vara,
                      float32 *meanb, float32 *varb,
                      float32 **cnta, float32 **cntb,
                      int32 ndensity, int32 nfeat, int32 dim, int32 continuous)
{
    int32   i;
    float32 cntc, la, lb, lc, nm, nv, lkdec, minvar;

    if (continuous) {
        minvar = cmd_ln_float32("-varfloor");
        cntc = cnta[0][0] + cntb[0][0];

        for (i=0, lc=0, lb=0, la=0;i<dim;i++){
            nm = (cnta[0][0]*meana[i] + cntb[0][0]*meanb[i])/cntc;
            nv = cnta[0][0]*(vara[i]+meana[i]*meana[i])+
                 cntb[0][0]*(varb[i]+meanb[i]*meanb[i]);
            nv = nv/cntc - nm*nm;
            if (nv < minvar) nv = minvar;
            lc += (float32)log(nv);
            lb += (float32)log(varb[i]);
            la += (float32)log(vara[i]);
        }
        lkdec = 0.5*(cntc*lc - cntb[0][0]*lb - cnta[0][0]*la);
    }
    else {
        int32 j;
        float32 P,Q;
        for (i=0,lkdec = 0; i < nfeat; i++) {
            for (j=0,P=0,Q=0; j < ndensity; j++) {
                P += cnta[i][j]; Q += cntb[i][j];
                lkdec += ((cnta[i][j]==0) ? 0 : cnta[i][j]*log(cnta[i][j]))
		       + ((cntb[i][j]==0) ? 0 : cntb[i][j]*log(cntb[i][j]))
		       - (((cnta[i][j]+cntb[i][j])==0) ? 0 : 
			  (cnta[i][j]+cntb[i][j])*log(cnta[i][j]+cntb[i][j]));
            }
            lkdec -= 
		((P==0) ? 0 : (P*log(P))) +
		((Q==0) ? 0 : (Q*log(Q))) -
		(((P+Q)==0) ? 0 : ((P+Q)*log(P+Q)));
        }
#if 0
        /* The old code, assumed log(0) would give a reasonable answer */
	for (i=0,lkdec = 0; i < nfeat; i++) {
            for (j=0,P=0,Q=0; j < ndensity; j++) {
                P += cnta[i][j]; Q += cntb[i][j];
                lkdec += cnta[i][j]*log(cnta[i][j]) + cntb[i][j]*log(cntb[i][j])
                           -(cnta[i][j]+cntb[i][j])*log(cnta[i][j]+cntb[i][j]);
            }
            lkdec -= P*log(P) + Q*log(Q) - (P+Q)*log(P+Q);
        }
#endif 
    }

    return(lkdec);
}


/* Find the two closest distributions. We assume 1 gaussian/state */

int32  findclosestpair(float32 **pmeans, float32 **pvars,
                       float32 ***pmixw, int32 nsets, int32 ndensity,
                       int32 nfeat, int32 dim, int32 *a, int32 *b, int32 continuous)
{
    float32    reduction, minreduction;
    int32      i, j, la=0, lb=0;

    minreduction = 1.0e+32;
    for (i=0; i<nsets; i++){
        for (j=i+1;j<nsets;j++){
            if (i != j){
                if (continuous) {
                    reduction = likelhddec(pmeans[i],pvars[i],
                                           pmeans[j],pvars[j],
                                           pmixw[i],pmixw[j],
                                           ndensity,nfeat,dim, continuous);
                }
                else {
                    reduction = likelhddec(NULL,NULL,
                                           NULL,NULL,
                                           pmixw[i],pmixw[j],
                                           ndensity,nfeat,dim, continuous);
                }
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


/* Permute a list of elements (here groups of phones) to obtain the best
   partitioning of the elements into two groups */

float32  permute(float32 **means, float32 **vars, float32 ***mixw, 
                 int32 ndensity, int32 nfeat, int32 dim,
                 int32 **lists, int32 *llists,
                 int32 npermute, int32 **lclass, int32 **rclass, 
                 int32 *nlclass, int32 *nrclass, int32 continuous)
{
    float32  **tmean=NULL, **tvar=NULL, ***tmixw=NULL;
    float32  bestdec,reduction,cnt;
    int32 i,j,k,l,m,n,ncombinations,bestclust=0;
    char  **identifier, *tmpid;
    int32  *llclass,*lrclass,lnlclass,lnrclass,ntot; 

    /* First gather and compute means and variances for the npermute groups */
    tmixw = (float32 ***)ckd_calloc_3d(npermute,nfeat,ndensity,sizeof(float32));
    if (continuous) {
        tmean = (float32 **) ckd_calloc_2d(npermute,dim,sizeof(float32));
        tvar = (float32 **) ckd_calloc_2d(npermute,dim,sizeof(float32));
    }
    for (i=0;i<npermute;i++) {
        for (j=0;j<llists[i];j++) {
            l = lists[i][j];
            if (continuous) {
                cnt = mixw[l][0][0];
                tmixw[i][0][0] += cnt;
                for (k=0;k<dim;k++){
                    tmean[i][k] += cnt*means[l][k];
                    tvar[i][k] += cnt*(vars[l][k] + means[l][k]*means[l][k]);
                }
            }
            else {
                for (m=0;m<nfeat;m++)
                    for (n=0;n<ndensity;n++)
                        tmixw[i][m][n] += mixw[l][m][n];
            }
        }
        if (continuous) {
            for (k=0;k<dim;k++){
                tmean[i][k] /= tmixw[i][0][0];
                tvar[i][k] = tvar[i][k]/tmixw[i][0][0]-tmean[i][k]*tmean[i][k];
            }
        }
    }
            
    /* We have npermute clusters now; permute them to get two clusters */
    /* There are 2^(npermute-1)-1 clusters possible. Test them all out */ 

    /* Create identifiers for 2^(npermute-1) clusters */
    for (i=1,ncombinations=1;i<npermute;i++,ncombinations*=2);
    identifier = (char **)ckd_calloc_2d(ncombinations,npermute,sizeof(char));
    tmpid = (char *)ckd_calloc(npermute,sizeof(char));
    for (i=0;i<ncombinations-1;i++){
        for(j=0,tmpid[0]=!tmpid[0];!tmpid[j];j++,tmpid[j]=!tmpid[j]);
        for(j=0;j<npermute;j++) identifier[i][j] = tmpid[j];
    }
    ckd_free(tmpid); 

    /* Go through the list and find best pair */
    for (i=0,bestdec=-1.0e+30;i<ncombinations-1;i++){
        float32 *meana=NULL, *vara=NULL, *meanb=NULL; 
        float32 *varb=NULL, **counta=NULL, **countb=NULL;

        counta = (float32 **) ckd_calloc_2d(nfeat,ndensity,sizeof(float32));
        countb = (float32 **) ckd_calloc_2d(nfeat,ndensity,sizeof(float32));

        if (continuous) {
            meana = (float32 *)ckd_calloc(dim,sizeof(float32));
            vara = (float32 *)ckd_calloc(dim,sizeof(float32));
            meanb = (float32 *)ckd_calloc(dim,sizeof(float32));
            varb = (float32 *)ckd_calloc(dim,sizeof(float32));
            for (j=0;j<npermute;j++){
                float32 *om = tmean[j];
                float32 *ov = tvar[j];
                cnt = tmixw[j][0][0];
                if (identifier[i][j]){
                    counta[0][0] += cnt;
                    for (k=0;k<dim;k++){
                        meana[k] += cnt * om[k];
                        vara[k] += cnt*(ov[k] + om[k]*om[k]);
                    }
                }
                else{
                    countb[0][0] += cnt;
                    for (k=0;k<dim;k++){
                        meanb[k] += cnt * om[k];
                        varb[k] += cnt*(ov[k] + om[k]*om[k]);
                    }
                }
            }
            for (k=0;k<dim;k++){
                meana[k] /= counta[0][0]; meanb[k] /= countb[0][0];
                vara[k] = vara[k]/counta[0][0] - meana[k]*meana[k];
                varb[k] = varb[k]/countb[0][0] - meanb[k]*meanb[k];
            }
        }
        else {
            for (j=0;j<npermute;j++){
                if (identifier[i][j]){
                    for (m=0;m<nfeat;m++)
                        for (n=0; n<ndensity; n++)
                            counta[m][n] += tmixw[j][m][n];
                }
                else {
                    for (m=0;m<nfeat;m++)
                        for (n=0; n<ndensity; n++)
                            countb[m][n] += tmixw[j][m][n];
                }
            }
        }
        reduction = likelhddec(meana,vara, meanb,varb,counta,countb,
                                                      ndensity,nfeat,dim, continuous);
        if (reduction > bestdec) {
            bestdec = reduction;
            bestclust = i;
        }
        if (continuous) {
            ckd_free(meana);ckd_free(vara);ckd_free(meanb);ckd_free(varb);
        }
        ckd_free_2d((void **)counta); ckd_free_2d((void **)countb);
    }

    for (i=0,ntot=0;i<npermute;i++) ntot += llists[i];
    llclass = (int32 *) ckd_calloc(ntot,sizeof(int32)); /* Overalloc */
    lrclass = (int32 *) ckd_calloc(ntot,sizeof(int32)); /* Overalloc */

    for (j=0,lnlclass=0,lnrclass=0;j<npermute;j++){
        if (identifier[bestclust][j]){
            for (l=0;l<llists[j];l++,lnlclass++)
                llclass[lnlclass] = lists[j][l];
        }
        else {
            for (l=0;l<llists[j];l++,lnrclass++)
                lrclass[lnrclass] = lists[j][l];
        }
    }

    *lclass = llclass; *rclass = lrclass;
    *nlclass = lnlclass; *nrclass = lnrclass;

    if (continuous) {
        ckd_free_2d((void **)tmean); ckd_free_2d((void **)tvar);
    }
    ckd_free_3d((void ***)tmixw); ckd_free_2d((void **)identifier);

    return(bestdec);
}

    
node *make_simple_tree (float32 **means, float32 **vars, float32 ***mixw,
           int32 *nodephoneids, int32 nphones, int32 ndensity, 
           int32 nfeat, int32 ndim, int32 npermute, int32 depth, int32 continuous)
{
    float32  **oldmeans=NULL, **oldvars=NULL, **newmeans=NULL, **newvars=NULL;
    float32  ***oldmixw, ***newmixw, **tmp2d, ***tmp3d;
    float32  minvar=0, bestdec;
    int32    **phoneid, **newphoneid, *numphones, *newnumphones, **it2d, *it1d;
    int32    i,j,k,l,a,b,set,nsets;
    int32    *lphoneids,*rphoneids,lnphones,rnphones;
    node     *root;

    /* Allocate and set basic root parameters */
    root = (node *) ckd_calloc(1,sizeof(node));
    root->phoneids = (int32 *) ckd_calloc(nphones,sizeof(int32));
    root->nphones = nphones;
    memcpy(root->phoneids,nodephoneids,nphones*sizeof(int32));
    root->depth = depth;

    /* Build the node by clustering and partitioning */
    oldmixw = (float32***)ckd_calloc_3d(nphones,nfeat,ndensity,sizeof(float32));
    newmixw = (float32***)ckd_calloc_3d(nphones,nfeat,ndensity,sizeof(float32));
    phoneid = (int32 **)ckd_calloc_2d(nphones,nphones,sizeof(int32));
    numphones = (int32 *) ckd_calloc(nphones,sizeof(int32));
    newphoneid = (int32 **)ckd_calloc_2d(nphones,nphones,sizeof(int32));
    newnumphones = (int32 *) ckd_calloc(nphones,sizeof(int32));

    if (continuous) {
        minvar = cmd_ln_float32("-varfloor");
        oldmeans = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
        oldvars = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
        newmeans = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
        newvars = (float32 **) ckd_calloc_2d(nphones,ndim,sizeof(float32));
    }

    for (i=0;i<nphones;i++){
        numphones[i] = 1;
        phoneid[i][0] =  nodephoneids[i]; /* Phone ids */
        for (j=0;j<nfeat;j++)
            for (k=0;k<ndensity;k++)
                oldmixw[i][j][k] = mixw[nodephoneids[i]][j][k];
        if (continuous) {
            for (l=0;l<ndim;l++){
                oldmeans[i][l] = means[nodephoneids[i]][l];
                oldvars[i][l] = vars[nodephoneids[i]][l];
            }
        }
    }
    if (nphones > npermute){

        for (nsets = nphones; nsets > npermute; nsets--) {
            /* Find the closest distributions */
            findclosestpair(oldmeans,oldvars,oldmixw,nsets,ndensity,
                                                         nfeat,ndim,&a,&b, continuous);

            /* Copy and Merge distributions... */
            /* Copy unmerged distributions first */
            for (i=0,set=0;i<nsets;i++){
                if (i != a && i != b){
                    newnumphones[set] = numphones[i];
                    for (l=0;l<numphones[i];l++)
                        newphoneid[set][l] = phoneid[i][l];
                    for (j=0;j<nfeat;j++)
                        for (k=0;k<ndensity;k++)
                            newmixw[set][j][k] = oldmixw[i][j][k];
                    if (continuous) {
                        for (l=0;l<ndim;l++){
                            newmeans[set][l] = oldmeans[i][l];
                            newvars[set][l] = oldvars[i][l];
                        }
                    }
                    set++;
                }
            }
            /* Merge a and b */
            newnumphones[set] =  numphones[a]+numphones[b];
            for (i=0;i<numphones[a];i++)
                newphoneid[set][i] = phoneid[a][i];
            for (l=0;l<numphones[b];l++,i++)
                newphoneid[set][i] = phoneid[b][l];
            if (continuous) {
                float32 *nm = newmeans[set];
                float32 *nv = newvars[set];
                float32 *oma = oldmeans[a];
                float32 *ova = oldvars[a];
                float32 *omb = oldmeans[b];
                float32 *ovb = oldvars[b];
                float32 cnta, cntb;

                cnta = oldmixw[a][0][0]; cntb = oldmixw[b][0][0];
                newmixw[set][0][0] = cnta + cntb;
                for (l=0;l<ndim;l++){
                    nm[l] = (cnta*oma[l] + cntb*omb[l]) / (cnta + cntb);
                    nv[l] = cnta*(ova[l]+oma[l]*oma[l]) +
                            cntb*(ovb[l]+omb[l]*omb[l]);
                    nv[l] = nv[l]/(cnta+cntb) - nm[l]*nm[l];
                    if (nv[l] < minvar) nv[l] = minvar;
                }
            }
            else {
                for (j=0;j<nfeat;j++)
                    for (k=0;k<ndensity;k++)
                        newmixw[set][j][k] = oldmixw[a][j][k]+oldmixw[b][j][k];
            }

            /* Switch old and new variables */
            tmp2d = oldmeans; oldmeans = newmeans; newmeans = tmp2d;
            tmp2d = oldvars;  oldvars = newvars;   newvars = tmp2d;
            tmp3d = oldmixw;  oldmixw = newmixw;   newmixw = tmp3d;
            it2d = phoneid;  phoneid = newphoneid; newphoneid = it2d;
            it1d = numphones; numphones = newnumphones; newnumphones = it1d;
        }
    }
    else npermute = nphones;

    if (nphones < 2){
        root->left = root->right = NULL; /* Dont split further */
        return(root);
    }

    bestdec = permute(means,vars,mixw,ndensity,nfeat,ndim,phoneid,numphones,
                      npermute,&lphoneids,&rphoneids,&lnphones,&rnphones, continuous);

    root->lkhd_dec = bestdec;

    if (continuous) {
        ckd_free_2d((void **)oldmeans); ckd_free_2d((void **)oldvars);
        ckd_free_2d((void **)newmeans); ckd_free_2d((void **)newvars);
    }
    ckd_free_3d((void ***)oldmixw); ckd_free_3d((void ***)newmixw);
    ckd_free_2d((void **)newphoneid); ckd_free_2d((void **)phoneid);

    /* Recurse */
    root->left = make_simple_tree(means,vars,mixw,lphoneids,lnphones,
              ndensity,nfeat,ndim,npermute,root->depth+1, continuous);
    root->right = make_simple_tree(means,vars,mixw,rphoneids,rnphones,
              ndensity,nfeat,ndim,npermute,root->depth+1, continuous);

    return(root);
}


/* Associates a unique id with all the nodes in a tree */
int32 id_nodes(node *root, node **idlist, int32 id)
{
    if (root->left != NULL) id = id_nodes(root->left,idlist,id);
    if (root->right != NULL) id = id_nodes(root->right,idlist,id);
    root->id = id; idlist[id] = root;
    return(id+1);
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



/* Pares a tree down to npermute leaves */
int32 pare_tree(node *sroot, int32 npermute, int32 **lists, int32 *llist)
{
    float32 *twiglkhddec,maxdec;
    int32  *leaflist, nleaves, *twiglist, ntwigs;
    int32  nnodes,i,maxid;
    node   **idlist;

    /* Overallocate idlist */
    idlist = (node **) ckd_calloc(1024,sizeof(node *));
    /* First "id" all the nodes */
    nnodes = id_nodes(sroot,idlist,0);

    /* Allocate lists (overallocate for safety margin) */
    leaflist = (int32 *) ckd_calloc(nnodes,sizeof(int32));
    twiglist = (int32 *) ckd_calloc(nnodes,sizeof(int32));
    twiglkhddec = (float32 *) ckd_calloc(nnodes,sizeof(float32));

    nleaves = ntwigs = 0;
    for(i=0;i<nnodes;i++) {
        if (IS_LEAF(idlist[i])) leaflist[nleaves++] = i;
        else if (IS_TWIG(idlist[i])) {
            twiglist[ntwigs] = i;
            twiglkhddec[ntwigs++] = (idlist[i])->lkhd_dec;
        }
    }

    while (nleaves > npermute) {
        /* find twig for which likelihood decerase was maximum 
         * this is the worst merge */
        maxdec = twiglkhddec[0]; maxid = twiglist[0];
        for (i=1; i<ntwigs;i++) {
            if (twiglkhddec[i] > maxdec) {
                maxdec = twiglkhddec[i];
                maxid = twiglist[i];
            }
        }
        /* remove children of twig, make twig a leaf, eliminate if from
           the node list, and continue */
        idlist[idlist[maxid]->left->id] = NULL;
        idlist[idlist[maxid]->right->id] = NULL;
        free_tree(idlist[maxid]->left); free_tree(idlist[maxid]->right);
        idlist[maxid]->left = idlist[maxid]->right = NULL;

        nleaves = ntwigs = 0;
        for(i=0;i<nnodes;i++) {
            if (IS_LEAF(idlist[i])) leaflist[nleaves++] = i;
            else if (IS_TWIG(idlist[i])) {
                twiglist[ntwigs] = i;
                twiglkhddec[ntwigs++] = (idlist[i])->lkhd_dec;
            }
        }
    }
    for (i=0;i<nleaves;i++) {
        llist[i] = idlist[leaflist[i]]->nphones;
        memcpy(lists[i],idlist[leaflist[i]]->phoneids,sizeof(int32)*llist[i]);
    }
    ckd_free(idlist); ckd_free(leaflist);
    ckd_free(twiglist); ckd_free(twiglkhddec);

    return(nleaves);
}


/* Prunes a tree down to nnode question nodes */
int32 prune_quests(node *sroot, int32 ndesirdnodes)
{
    float32 *twiglkhddec,maxdec;
    int32  *leaflist, nquests, nleaves, *twiglist, ntwigs;
    int32  nnodes,i,maxid;
    node   **idlist;

    /* Ignore root node == preincrement ndesirdnodes */
    ndesirdnodes++;
    /* Overallocate idlist */
    idlist = (node **) ckd_calloc(1024,sizeof(node *));
    /* First "id" all the nodes */
    nnodes = id_nodes(sroot,idlist,0);

    /* Allocate lists (overallocate for safety margin) */
    leaflist = (int32 *) ckd_calloc(nnodes,sizeof(int32));
    twiglist = (int32 *) ckd_calloc(nnodes,sizeof(int32));
    twiglkhddec = (float32 *) ckd_calloc(nnodes,sizeof(float32));

    nleaves = ntwigs = nquests = 0;
    for(i=0;i<nnodes;i++) {
        if (IS_QUEST(idlist[i])) nquests++;
        if (IS_LEAF(idlist[i])) leaflist[nleaves++] = i;
        else if (IS_TWIG(idlist[i])) {
            twiglist[ntwigs] = i;
            twiglkhddec[ntwigs++] = (idlist[i])->lkhd_dec;
        }
    }

    while (nquests > ndesirdnodes) {
        /* find twig for which likelihood decerase was maximum */
        maxdec = twiglkhddec[0]; maxid = twiglist[0];
        for (i=1; i<ntwigs;i++) {
            if (twiglkhddec[i] > maxdec) {
                maxdec = twiglkhddec[i];
                maxid = twiglist[i];
            }
        }
        /* remove children of twig, make twig a leaf, eliminate it from
           the node list, and continue */
        free_tree(idlist[maxid]->left); free_tree(idlist[maxid]->right);
        idlist[maxid]->left = idlist[maxid]->right = NULL;

        ckd_free(idlist);
        idlist = (node **) ckd_calloc(1024,sizeof(node *));
        nnodes = id_nodes(sroot,idlist,0);
        nleaves = ntwigs = nquests = 0;
        for(i=0;i<nnodes;i++) {
            if (IS_QUEST(idlist[i])) nquests++;
            if (IS_LEAF(idlist[i])) leaflist[nleaves++] = i;
            else if (IS_TWIG(idlist[i])) {
                twiglist[ntwigs] = i;
                twiglkhddec[ntwigs++] = (idlist[i])->lkhd_dec;
            }
        }
    }
    ckd_free(idlist); ckd_free(leaflist);
    ckd_free(twiglist); ckd_free(twiglkhddec);

    return(nquests);
}


node  *make_tree (float32 **means, float32 **vars, float32 ***mixw,
           int32 *phnids, int32 nphones, int32 ndensity, int32 nfeat,
           int32 dim, int32 npermute, int32 depth, int32 continuous)
{
    float32 lkhddec;
    int32 niter,iter,jpermute, **lists, *llists;
    int32 *lclass, nlclass, *rclass, nrclass;
    node *sroot;

    niter = cmd_ln_int32("-niter");

    sroot = make_simple_tree(means,vars,mixw,phnids,nphones,ndensity,nfeat,dim,npermute,depth, continuous);
    for (iter = 0; iter < niter; iter++) {
        /* overallocate lists of lists */
        lists = (int32 **) ckd_calloc_2d(npermute,nphones,sizeof(int32));
        llists = (int32 *) ckd_calloc(npermute,sizeof(int32));

        jpermute = pare_tree(sroot,npermute,lists,llists);
        if (jpermute > 1){
            lkhddec = permute(means,vars,mixw,ndensity,nfeat,dim,lists,llists,
                                   jpermute,&lclass,&rclass,&nlclass,&nrclass, continuous);
            free_tree(sroot->left); free_tree(sroot->right);
            sroot->lkhd_dec = lkhddec;
            sroot->left = 
                make_tree(means,vars,mixw,lclass,nlclass,ndensity,nfeat,
                          dim,npermute,depth+1, continuous);
            sroot->right = 
                make_tree(means,vars,mixw,rclass,nrclass,ndensity,nfeat,
                          dim,npermute,depth+1, continuous);
        }
        else { sroot->left = sroot->right = NULL; }
        ckd_free_2d((void **)lists); ckd_free(llists);
    }
    return(sroot);
}


int compare(const void *a, const void *b)
{
    const int *i = (int*)a;
    const int *j = (int*)b;
    if (*i > *j) return 1;
    if (*j > *i) return -1;
    return 0;
}

int32 get_quests(node *root,int32 **qarr,int32 *nph,int32 nquests,int32 depth)
{
    int32 *sortedidx,i;

    if (root->nphones < 2 && depth > 3) return nquests;

    if (depth > 0){
        sortedidx = (int32 *)ckd_calloc(root->nphones,sizeof(int32));
        for (i=0;i<root->nphones;i++) sortedidx[i] = root->phoneids[i];
        qsort(sortedidx,root->nphones,sizeof(int32),compare);
        nph[nquests] = root->nphones;
        qarr[nquests++] = sortedidx;
    }

    if (root->nphones < 2) return nquests;
    if (root->left != NULL) 
        nquests = get_quests(root->left,qarr,nph,nquests,depth+1);
    if (root->right != NULL) 
        nquests = get_quests(root->right,qarr,nph,nquests,depth+1);

    return nquests;
}



void   sort_quests(int32 **qarr, int32 *nqfone, uint32 nfone, int32 *nquests)
{
    int32  i, j, k;
    int32  *marker, *flag;

    marker = (int32 *) ckd_calloc(*nquests,sizeof(int32));

    for (i=0;i<*nquests;i++){
        for (j=i+1; j < *nquests; j++){
            if (nqfone[i]+nqfone[j] != nfone) continue;
            flag = (int32 *) ckd_calloc(nfone,sizeof(int32));
            for (k=0;k < nqfone[i];k++) flag[qarr[i][k]] = 1;
            for (k=0;k < nqfone[j];k++) flag[qarr[j][k]] = 1;
            for (k=0;k < nfone;k++) if (flag[k] == 0) break;
            if (k == nfone) { /* the two questions are complements */
                if (nqfone[i] > nqfone[j]) marker [i] = 1;
                else marker[j] = 1;
            }
            ckd_free((void *)flag);
        }
    }
    for (i=0,j=0;i<*nquests;i++){
        if (marker[i] == 1) {
            ckd_free((void *)qarr[i]);
            continue;
        }
        qarr[j] = qarr[i];
        nqfone[j] = nqfone[i];
        j++;
    }
    *nquests = j;
       
    ckd_free((void *) marker);
}
            


static int
init(float32 *****out_mixw,
     float32 ****out_mean,
     float32 ****out_var,
     uint32  *out_veclen,
     uint32 *out_n_model,
     uint32 *out_n_state,
     uint32 *out_n_feat,
     uint32 *out_n_density,
     char   ***out_phone,
     int continuous)
{
    const char *moddeffn;
    const char *mixwfn;
    model_def_t *mdef;
    uint32 p_s = NO_ID, p_e = NO_ID, s, m, n_ci, id;
    uint32 mixw_s, mixw_e;
    float32 ****mixw=0;
    float32 ****mixw_occ=0;
    float32 ***in_mixw;
    uint32 n_state, n_model, n_in_mixw, n_stream, n_density;
    uint32 i, j, k;
    float64 dnom;
    uint32  *l_veclen, *t_veclen;
    uint32  l_nstates, t_nstates;
    uint32  t_nfeat, t_ndensity;
    vector_t  ***fullmean=NULL;
    vector_t  ***fullvar=NULL;
    vector_t  ****fullvar_full=NULL;
    float32   ***mean=NULL;
    float32   ***var=NULL;
    float32   varfloor;
    uint32    ll=0,n,nn,sumveclen;
    char      **phone;
    
    moddeffn = cmd_ln_str("-moddeffn");
    if (moddeffn == NULL)
	E_FATAL("Specify -moddeffn\n");

    E_INFO("Reading: %s\n", moddeffn);
    if (model_def_read(&mdef, moddeffn) != S3_SUCCESS)
	return S3_ERROR;

    n_ci = mdef->acmod_set->n_ci;
    p_s = 0; p_e = n_ci - 1;
    phone = (char **) ckd_calloc_2d(n_ci,100,sizeof(char));
    for (id = 0;id < n_ci;id++) 
        strcpy(phone[id],acmod_set_id2name(mdef->acmod_set,id));
    *out_phone = phone;

    /* Find first and last mixing weight used for p_s through p_e */
    mixw_s = mdef->defn[0].state[0];
    mixw_e = mdef->defn[p_e].state[mdef->defn[p_e].n_state-2];

    mixwfn = cmd_ln_str("-mixwfn");
    if (mixwfn == NULL)
	E_FATAL("Specify -mixwfn\n");

    E_INFO("Reading: %s\n", mixwfn);
    if (s3mixw_read(mixwfn,
		   &in_mixw,
		   &n_in_mixw,
		   &n_stream,
		   &n_density) != S3_SUCCESS)
	return S3_ERROR;

    n_model = n_ci;
    n_state = mdef->defn[p_s].n_state - 1;
    *out_n_feat = n_stream;
    *out_n_state = n_state;
    *out_n_density = n_density;

    for (i = p_s+1; i <= p_e; i++) {
	if ((mdef->defn[i].n_state - 1) != n_state) {
	    E_FATAL("Models do not have uniform topology\n");
	}
    }

    /*
     * Build the 4D array:
     *
     *     mixw[0..n_model-1][0..n_state-1][0..n_feat-1][0..n_density-1]
     *
     * out of the usual mixw[][][] array
     *
     */
    mixw_occ = (float32 ****)ckd_calloc_2d(n_model, n_state, sizeof(float32 **));
    *out_mixw = mixw;

    /* Re-index mixing weights by model and topological state position */
    for (i = p_s, j = 0; i <= p_e; i++, j++) {
	for (k = 0; k < n_state; k++) {
	    s = mdef->defn[i].state[k] - mixw_s;
	    mixw_occ[j][k] = in_mixw[s];
	}
    }

    assert(j == n_model);
    
    if (continuous) {      /* bother with means and variances only if not semi*/
	int32 var_is_full = cmd_ln_int32("-fullvar");
        /* Read Means and Variances; perform consistency checks */
        if (s3gau_read(cmd_ln_str("-meanfn"),
                       &fullmean,
                       &l_nstates,
                       &t_nfeat,
                       &t_ndensity,
                       &l_veclen) != S3_SUCCESS)
            E_FATAL("Error reading mean file %s\n",cmd_ln_str("-meanfn"));
        *out_veclen = (uint32)l_veclen[0];
        if (t_nfeat != n_stream && t_ndensity != n_density)
            E_FATAL("Mismatch between Mean and Mixture weight files\n");

	if (var_is_full) {
	    if (s3gau_read_full(cmd_ln_str("-varfn"),
				&fullvar_full,
				&t_nstates,
				&t_nfeat,
				&t_ndensity,
				&t_veclen) != S3_SUCCESS)
		E_FATAL("Error reading var file %s\n",cmd_ln_str("-varfn"));
	}
	else {
	    if (s3gau_read(cmd_ln_str("-varfn"),
			   &fullvar,
			   &t_nstates,
			   &t_nfeat,
			   &t_ndensity,
			   &t_veclen) != S3_SUCCESS)
		E_FATAL("Error reading var file %s\n",cmd_ln_str("-varfn"));
	}
        if (t_nfeat != n_stream && t_ndensity != n_density)
            E_FATAL("Mismatch between Variance and Mixture weight files\n");
        for (i=0;i<n_stream;i++)
            if (t_veclen[i] != l_veclen[i])
               E_FATAL("Feature length %d in var file != %d in mean file for feature %d\n",t_veclen[i],l_veclen[i],i);
        if (l_nstates != t_nstates)
           E_FATAL("Total no. of states %d in var file != %d in mean file\n",t_nstates,l_nstates);

        if (t_nfeat > 1)
            E_FATAL("N-Feat = %d! N-Feat > 1 not implemented yet..\n",t_nfeat);
        if (t_ndensity > 1)
            E_WARN("The state distributions given have %d gaussians per state;\n..*..shrinking them down to 1 gau per state..\n",t_ndensity);

        /* Allocate for out_mean and out_var. If input are multi_gaussian
           distributions convert to single gaussians. Copy appropriate
           states to out_mean and out_var */
        for (i=0,sumveclen=0; i < n_stream; i++) sumveclen += t_veclen[i];
        mean = (float32 ***)ckd_calloc_3d(n_model,n_state,sumveclen,sizeof(float32));
	/* Use only the diagonals regardless of whether -varfn is full. */
        var = (float32 ***)ckd_calloc_3d(n_model,n_state,sumveclen,sizeof(float32));
        mixw = (float32 ****)ckd_calloc_4d(n_model,n_state,1,1,sizeof(float32));
        varfloor = cmd_ln_float32("-varfloor");
        for (i = p_s, j = 0, m = mixw_s; i <= p_e; i++, j++) {
            for (k = 0; k < n_state; k++, m++) {
                float32 *featmean=NULL,*featvar=NULL;

                featmean = mean[j][k];
                featvar = var[j][k];
                dnom  = 0;
                for (n = 0; n < n_density; n++) {
                    float32 mw = mixw_occ[j][k][0][n];
                    dnom += mw;
                    for (nn = 0; nn < l_veclen[0]; nn++) {
                        featmean[nn] += mw * fullmean[m][0][n][nn];
			if (var_is_full)
			    featvar[nn] += 
				mw *(fullmean[m][0][n][nn]*fullmean[m][0][n][nn] +
				     fullvar_full[m][0][n][nn][nn]);
			else
			    featvar[nn] += 
				mw *(fullmean[m][0][n][nn]*fullmean[m][0][n][nn] +
				     fullvar[m][0][n][nn]);
                    }
                }
                if (dnom != 0) {
                    for (nn = 0; nn < l_veclen[0]; nn++) {
                        featmean[nn] /= dnom;
                        featvar[nn] = featvar[nn]/dnom - featmean[nn]*featmean[nn];
                        if (featvar[nn] < varfloor) featvar[nn] = varfloor;
                    }
                }
                else {
                    for (nn = 0; nn < l_veclen[0]; nn++) {
                        if (featmean[nn] != 0)
                            E_FATAL("dnom = 0, but featmean[nn] != 0, =  %f for ll = %d\n",featmean[nn],ll);
                    }
                }
              /* Now on we need only have global counts for the mixws,
                 so we store them all in mixw_occ[][] */
                mixw[j][k][0][0] = dnom;
            }
        }
        for (i=0,j=0;i<n_ci;i++){
            if (strstr(phone[i],"+") == NULL && strcmp(phone[i],"SIL") != 0){
                strcpy(phone[j],phone[i]);
                for (k=0;k<n_state;k++){
                    mixw[j][k][0][0] = mixw[i][k][0][0];
                    for (n=0;n<l_veclen[0];n++){
                        mean[j][k][n] = mean[i][k][n];
                        var[j][k][n] = var[i][k][n];
                    }
                }
                j++;
            }
        }
        n_model = j;
        *out_n_feat = *out_n_density = 1;
    }
    else {
        int32 l;
        mixw = (float32 ****)ckd_calloc_4d(n_model,n_state,n_stream,n_density,sizeof(float32));
        for (i=0,j=0;i<n_ci;i++){
            if (strstr(phone[i],"+") == NULL && strcmp(phone[i],"SIL") != 0){
                strcpy(phone[j],phone[i]);
                for (k=0;k<n_state;k++){
                    for (l=0;l<n_stream;l++){
                        for (m=0;m<n_density;m++)
                            mixw[j][k][l][m] = mixw_occ[i][k][l][m];
                    }
                }
                j++;
            }
        }
        n_model = j;
    }

    *out_n_model = n_model;
    *out_mean = mean;
    *out_var = var;
    *out_mixw = mixw;
    if (continuous) {
        ckd_free_4d((void ****)fullmean);
	if (fullvar)
	    ckd_free_4d((void ****)fullvar);
	if (fullvar_full)
	    gauden_free_param_full(fullvar_full);
    }

    return S3_SUCCESS;
}

int
main(int argc, char *argv[])
{
    float32 ****mixw = NULL,***lmixw = NULL;
    float32 ***means = NULL, **lmeans = NULL;
    float32 ***vars = NULL, **lvars = NULL;
    uint32 veclen = 0;
    uint32 n_model = 0;
    uint32 n_state = 0;
    uint32 n_feat = 0;
    uint32 n_density = 0;
    uint32 state;
    uint32 npermute, nquests_per_state;
    int32 i, j, k, continuous, **questarr, *nquestphone, nquests=0;
    int32  *phoneids,nphones;
    char   **phone_list = NULL;
    const char   *outfile;
    node   *root;
    FILE   *fp;
    const char* type;

    parse_cmd_ln(argc, argv);

    type = cmd_ln_str("-type");
    
    if(type==NULL){
      E_FATAL("-type is empty. Please specify -type correctly, either \".cont.\" or \".semi.\"\n");
    }

    continuous = -1;
    if (strcmp(type,".cont.") == 0) 
        continuous = 1;
    else if (strcmp(type,".semi.") == 0) 
        continuous = 0;
    else{
      E_FATAL("Unknown type %s, either \".cont.\" or \".semi.\"\n", type);
    }
      
    if(continuous ==-1){
      E_FATAL("-type is not set correctly\n");
    }

    outfile = cmd_ln_str("-questfn");
    npermute = cmd_ln_int32("-npermute");
    nquests_per_state = cmd_ln_int32("-qstperstt");

    /* Test and cleanup outfile */
    if ((fp = fopen(outfile,"w")) == NULL)
        E_FATAL("Unable to open %s for writing!\n",outfile);
    fprintf(fp,"WDBNDRY_B\nWDBNDRY_E\nWDBNDRY_S\nWDBNDRY_I\nSILENCE   SIL\n");
    fclose(fp);

    if (init(&mixw,
             &means,
             &vars,
             &veclen,
	     &n_model,
	     &n_state,
	     &n_feat,
             &n_density,
	     &phone_list,
	     continuous) != S3_SUCCESS) {
	E_FATAL("Initialization failed\n");
    }

    for (state = 0; state < n_state; state++){
        phoneids = (int32 *) ckd_calloc(n_model,sizeof(int32));
        root = (node *) ckd_calloc(1,sizeof(node));
        nphones = n_model;
        lmixw = (float32 ***)ckd_calloc_3d(n_model,n_feat,n_density,sizeof(float32));
        if (continuous) {
            lmeans = (float32 **)ckd_calloc_2d(n_model,veclen,sizeof(float32));
            lvars = (float32 **)ckd_calloc_2d(n_model,veclen,sizeof(float32));
            for (i=0;i<n_model;i++){
                phoneids[i] = i;
                lmixw[i][0][0] = mixw[i][state][0][0];
                for (j=0;j<veclen;j++){
                    lmeans[i][j] = means[i][state][j];
                    lvars[i][j] = vars[i][state][j];
                }
            }
        }
        else {
            for (i=0;i<n_model;i++){
                phoneids[i] = i;
                for (j=0;j<n_feat;j++)
                    for (k=0;k<n_density;k++)
                        lmixw[i][j][k] = mixw[i][state][j][k];
            }
        }
        root = make_tree(lmeans,lvars,lmixw,phoneids,nphones,n_density,n_feat,veclen,npermute,0, continuous);
        if (continuous) {
            ckd_free_2d((void **)lmeans); ckd_free_2d((void **)lvars); 
        }
        ckd_free_3d((void ***)lmixw); ckd_free((void *)phoneids);

        questarr = (int32 **)ckd_calloc(n_state*n_model,sizeof(int32 *));
        nquestphone = (int32 *)ckd_calloc(n_state*n_model,sizeof(int32));

        prune_quests(root,nquests_per_state);
        nquests = get_quests(root,questarr,nquestphone,0,0);

        free_tree(root);

        fp = fopen(outfile,"a");
        for (i=0;i<nquests;i++){
            if (state < n_state/2) fprintf(fp,"QUESTION%d_%d_R ",state,i);
            else if (state == n_state/2) fprintf(fp,"QUESTION%d ",i);
            else fprintf(fp,"QUESTION%d_%d_L ",state,i);
            for (j=0;j<nquestphone[i];j++) 
                fprintf(fp," %s",phone_list[questarr[i][j]]);
            fprintf(fp,"\n");
        }
        fclose(fp);
        E_INFO("Done building questions using state %d\n",state);
        E_INFO(" %d questions from state %d\n",nquests,state);
        for(i=0;i<nquests;i++) ckd_free((void *)questarr[i]);
        ckd_free((void **)questarr);
        ckd_free((void *)nquestphone);
    }
    E_INFO ("Stored questions in %s\n",outfile);
    return 0;
}
