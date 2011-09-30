/* ====================================================================
 * Copyright (c) 2000 Carnegie Mellon University.  All rights 
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
 * Various functions used in mk_mdef_gen
 *
 * Author: 
 *    Rita Singh (rsingh@cs.cmu.edu) 
 *********************************************************************/


#include <s3/s3.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <s3/s3.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/pio.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "parse_cmd_ln.h"
#include "heap.h"
#include "hash.h"

#define CEILING   10000  
#define IS_FILLER(X)	((X[0]=='+'||strcmp(X,"SIL")==0) ? 1 : 0)


static const char* wordpos2str(word_posn_t wordpos, int ignore_wordpos);

int32 make_ci_list_frm_mdef(const char  *mdeffile,
		            char  ***CIlist, 
                            int32 *cilistsize)
{
    char  **cilist;

    int32 nciphns, id, maxphnsize, phnsize;
    model_def_t *mdef;

    if (model_def_read(&mdef, mdeffile) != S3_SUCCESS) 
	E_ERROR("Unable to read mdef file %s\n",mdeffile);
    *cilistsize = nciphns = mdef->acmod_set->n_ci;

   /* Find phone length */
    maxphnsize = 0;
    for (id = 0; id < nciphns; id++){
	phnsize = strlen(acmod_set_id2name(mdef->acmod_set,id));
 	if (phnsize > maxphnsize) maxphnsize = phnsize;
    }
    *CIlist = cilist = (char**)ckd_calloc_2d(nciphns,maxphnsize+1,sizeof(char));
    for (id = 0; id < nciphns; id++){
        strcpy(cilist[id],acmod_set_id2name(mdef->acmod_set,id));
    }

    return S3_SUCCESS;
}


int32 make_ci_list_cd_hash_frm_phnlist(const char  *phnlist,
		           	    char  ***CIlist, 
                           	    int32 *cilistsize,
			   	    hashelement_t ***CDhash,
				    int32 *NCDphones)
{
    char  bphn[1024],lctx[1024], rctx[1024], wdpos[1024]; 
    char  **cilist, *silence="SIL";
    lineiter_t *line = NULL;
    heapelement_t **heap=NULL, *addciphone;
    hashelement_t **tphnhash, *tphnptr;
    phnhashelement_t  **phnhash, *phnptr;

    int32 swdtphs, bwdtphs, ewdtphs, iwdtphs, maxphnsize, phnsize;
    int32 heapsize, i, nciphns, nwds;
    FILE  *fp;

    fp = fopen(phnlist,"r");
    if (fp==NULL) E_FATAL("Unable to open %s for reading\n",phnlist);

   /* Initially hash everything to remove duplications */
    phnhash = (phnhashelement_t**)calloc(PHNHASHSIZE,sizeof(phnhashelement_t*));
    tphnhash = (hashelement_t**) ckd_calloc(HASHSIZE, sizeof(hashelement_t*));
    maxphnsize = 0;
    swdtphs = bwdtphs = ewdtphs = iwdtphs = 0;
    /* Always install SIL in phonelist */
    phninstall(silence,phnhash);

    for (line = lineiter_start_clean(fp); line; line = lineiter_next(line)) {

        nwds = sscanf(line->buf,"%s %s %s %s",bphn,lctx,rctx,wdpos);

        if (nwds != 1 && nwds != 3 &&  nwds != 4)
	    E_FATAL("Incorrect format in triphone file %s\n%s\n",phnlist,line->buf);
        if (strcmp(bphn,"-") == 0)
	    E_FATAL("Bad entry triphone file %s\n%s\n",phnlist,line->buf);
	    
        if (nwds == 1 || (!strcmp(lctx,"-") && !strcmp(rctx,"-"))) {
	    phnsize = strlen(bphn);
 	    if (phnsize > maxphnsize) maxphnsize = phnsize;
	    phninstall(bphn,phnhash);
        }
        else {
            if (nwds == 3) strcpy(wdpos,"i"); /* default */
            if (!strcmp(lctx,"-") && !strcmp(rctx,"-") && !strcmp(wdpos,"-")){
	        E_WARN("Bad entry triphone file %s\n%s\n",phnlist,line->buf);
		continue;
	    }
	    if (IS_FILLER(lctx) || IS_FILLER(rctx)){
		E_WARN("Fillers in triphone, skipping:\n%s\n",line->buf);
		continue;
	    }
            if (!strcmp(wdpos,"i") && !strcmp(wdpos,"b") && 
		!strcmp(wdpos,"e") && !strcmp(wdpos,"s")){
		E_WARN("Bad word position type %s in triphone list\n",wdpos);
		E_WARN("Mapping it to word internal triphone\n");
		strcpy(wdpos,"i");
	    }
            tphnptr = install(bphn,lctx,rctx,wdpos,tphnhash);
	    if (tphnptr->dictcount == 0){
	        if (strcmp(wdpos,"s")==0) swdtphs++;
	        else if (strcmp(wdpos,"b")==0) bwdtphs++;
	        else if (strcmp(wdpos,"e")==0) ewdtphs++;
	        else if (strcmp(wdpos,"i")==0) iwdtphs++;
	        tphnptr->dictcount++;
	    }
	}
    }
    fclose(fp);

    /* Heap sort CI phones */
    heapsize = 0;
    for (i = 0; i < PHNHASHSIZE; i++) {
        phnptr = phnhash[i];
        while (phnptr != NULL) {
            addciphone = (heapelement_t *) ckd_calloc(1,sizeof(heapelement_t));
            addciphone->basephone = strdup(phnptr->phone);
            heapsize = insert(&heap, heapsize, addciphone);
            phnptr = phnptr->next;
        }
    }
    freephnhash(phnhash);
    *cilistsize = nciphns = heapsize;
    *CIlist = cilist = (char**)ckd_calloc_2d(nciphns,maxphnsize+1,sizeof(char));
    for (i = 0; i < nciphns; i++){
	addciphone = yanktop(&heap,heapsize,&heapsize);
        strcpy(cilist[i],addciphone->basephone);
        free_heapelement(addciphone);
    }
    assert(heapsize == 0);

    E_INFO("%d single word triphones in input phone list\n",swdtphs);
    E_INFO("%d word beginning triphones in input phone list\n",bwdtphs);
    E_INFO("%d word internal triphones in input phone list\n",iwdtphs);
    E_INFO("%d word ending triphones in input phone list\n",ewdtphs);

    *CDhash = tphnhash;
    *NCDphones = swdtphs + bwdtphs + iwdtphs + ewdtphs;

    return S3_SUCCESS;
}


int32 make_ci_list_cd_hash_frm_mdef(const char  *mdeffile,
		           	    char  ***CIlist, 
                           	    int32 *cilistsize,
			   	    hashelement_t ***CDhash,
				    int32 *NCDphones)
{
    char  bphn[1024],lctx[1024], rctx[1024], wdpos[1024]; 
    char  **cilist;
    hashelement_t **tphnhash, *tphnptr;

    int32 swdtphs, bwdtphs, ewdtphs, iwdtphs, maxphnsize, phnsize;
    int32 nciphns, id, n_acmod;
    model_def_t *mdef;

    if (model_def_read(&mdef, mdeffile) != S3_SUCCESS) 
	E_ERROR("Unable to read mdef file %s\n",mdeffile);
    *cilistsize = nciphns = mdef->acmod_set->n_ci;
    n_acmod = acmod_set_n_acmod(mdef->acmod_set);

   /* Find Max phone length */
    maxphnsize = 0;
    for (id = 0; id < nciphns; id++){
	phnsize = strlen(acmod_set_id2name(mdef->acmod_set,id));
 	if (phnsize > maxphnsize) maxphnsize = phnsize;
    }
    *CIlist = cilist = (char**)ckd_calloc_2d(nciphns,maxphnsize+1,sizeof(char));
    for (id = 0; id < nciphns; id++){
        strcpy(cilist[id],acmod_set_id2name(mdef->acmod_set,id));
    }

    tphnhash = (hashelement_t**) ckd_calloc(HASHSIZE, sizeof(hashelement_t*));
    swdtphs = bwdtphs = ewdtphs = iwdtphs = 0;
    for (;id < n_acmod; id++){
	sscanf(acmod_set_id2name(mdef->acmod_set, id),"%s %s %s %s",
		bphn,lctx,rctx,wdpos);
        tphnptr = install(bphn,lctx,rctx,wdpos,tphnhash);
	if (tphnptr->dictcount == 0){
	    if (strcmp(wdpos,"s")==0) swdtphs++;
	    else if (strcmp(wdpos,"b")==0) bwdtphs++;
	    else if (strcmp(wdpos,"e")==0) ewdtphs++;
	    else if (strcmp(wdpos,"i")==0) iwdtphs++;
	    tphnptr->dictcount++;
	}
    }

    E_INFO("%d single word triphones in triphone list\n",swdtphs);
    E_INFO("%d word beginning triphones in triphone list\n",bwdtphs);
    E_INFO("%d word internal triphones in triphone list\n",iwdtphs);
    E_INFO("%d word ending triphones in triphone list\n",ewdtphs);

    *CDhash = tphnhash;
    *NCDphones = swdtphs + bwdtphs + iwdtphs + ewdtphs;

    return S3_SUCCESS;
}


int32  read_dict(const char *dictfile, const char *fillerdict, 
		 dicthashelement_t ***dicthash)
{
    char *dictsent;
    const char *dictfn[2];
    lineiter_t *liter = NULL;
    char  *dictword, *word, *phone, *tphn;
    dicthashelement_t   **lhash, *sptr;
    int32  maxphnlen, vocabsiz=0, nphns, numdicts, idict;
    FILE   *dict;

    numdicts = 1;  dictfn[0] = dictfile;
    if (fillerdict != NULL){
        dictfn[1] = fillerdict; 
        numdicts = 2;
    }

    lhash = (dicthashelement_t**)calloc(DICTHASHSIZE,sizeof(dicthashelement_t));
    if (lhash == NULL) E_FATAL("Unable to allocate space for dictionary\n");

    /* Create hash table with dictionary words as entries */
    for (idict = 0; idict < numdicts; idict++){
	E_INFO("Reading dict %s\n",dictfn[idict]);
        if ((dict = fopen(dictfn[idict],"r")) == NULL)
            E_FATAL_SYSTEM("Unable to open dictionary %s", dictfile);

        vocabsiz = 0;
        for (liter = lineiter_start_clean(dict); liter; liter = lineiter_next(liter)) {

	    if (liter->buf[0] == 0) {
		E_WARN("Empty line %d in the dictionary file %s\n", 
			lineiter_lineno(liter), dictfn[idict]);
		continue;
	    }

	    dictsent = strdup(liter->buf);
            if ((dictword = strtok(dictsent, " \t\n")) == NULL)
                E_FATAL("Empty line in dictionary!\n");

            if ((sptr = dictinstall(dictword,lhash)) == NULL)
                E_FATAL("Unable to install dict word %s\n",dictword);

            if (sptr->nphns != 0)
                E_FATAL("Duplicate entry for %s in dictionary\n",dictword);

            /* Count number of phones in pronunciation */
	    maxphnlen = 0;
            for(nphns=0; (tphn=strtok(NULL," \t\n")) != NULL; nphns++){
	        if (strlen(tphn) > maxphnlen) maxphnlen = strlen(tphn);
            }
	    if (nphns == 0)
	        E_FATAL("Dictionary word %s has no pronunciation\n",dictword);
            sptr->nphns = nphns; maxphnlen++;
            sptr->phones = (char**)ckd_calloc_2d(nphns,maxphnlen,sizeof(char));

            word = strtok(liter->buf," \t\n");
            for(nphns=0;(phone = strtok(NULL," \t\n")) != NULL;nphns++)
	        strcpy(sptr->phones[nphns],phone);
	
            ++vocabsiz;
            
            free(dictsent);
        }
        fclose(dict);
        E_INFO("%d words in dict %s\n", vocabsiz, dictfn[idict]);
    }

    *dicthash = lhash;

    lineiter_free(liter);
    return(vocabsiz);
}

    
int32 make_dict_triphone_list (dicthashelement_t **dicthash,
			  hashelement_t ***triphonehash,
			  int ignore_wpos)
{
    hashelement_t **tphnhash, *tphnptr;
    dicthashelement_t *word_el;
    phnhashelement_t *bphnptr, **bphnhash, *ephnptr, **ephnhash;
    char *bphn, *lctx, *rctx;
    const char* wpos;
    char *silencephn = "SIL";
    int32 totaltphs, totwds, bwdtphs, ewdtphs, iwdtphs, swdtphs, lnphns;
    int32 i,j,k;

    tphnhash = (hashelement_t**) ckd_calloc(HASHSIZE, sizeof(hashelement_t));
    bphnhash = (phnhashelement_t**)calloc(PHNHASHSIZE,sizeof(phnhashelement_t));
    ephnhash = (phnhashelement_t**)calloc(PHNHASHSIZE,sizeof(phnhashelement_t));
    if (bphnhash == NULL || ephnhash == NULL)
	E_FATAL("Unable to alloc %d size phone hashtables!\n",(int)PHNHASHSIZE);

    /*First count all phones that can begin or end a word (SIL can by default)*/
    phninstall(silencephn,bphnhash);
    phninstall(silencephn,ephnhash);
    for (i = 0; i < DICTHASHSIZE; i++){
        word_el = dicthash[i];
        while (word_el != NULL){
	    if (!IS_FILLER(word_el->phones[0]))
	        phninstall(word_el->phones[0],bphnhash);
	    if (!IS_FILLER(word_el->phones[word_el->nphns - 1]))
	        phninstall(word_el->phones[word_el->nphns - 1],ephnhash);
	    word_el = word_el->next;
        }
    }

    /* Scan dictionary and make triphone list */
    totwds = bwdtphs = ewdtphs = iwdtphs = swdtphs = 0;
    for (i = 0; i < DICTHASHSIZE; i++){
        word_el = dicthash[i];
        while (word_el != NULL){
	    totwds++;
            lnphns = word_el->nphns;
            if (lnphns == 1) {
                wpos = wordpos2str(WORD_POSN_SINGLE, ignore_wpos);
                bphn = word_el->phones[0];
		if (IS_FILLER(bphn)) {word_el = word_el->next; continue;}
		for (j = 0; j < PHNHASHSIZE; j++){
		    ephnptr = ephnhash[j];
		    while (ephnptr != NULL){
			lctx = ephnptr->phone;
		        for (k = 0; k < PHNHASHSIZE; k++){
		    	    bphnptr = bphnhash[k];
		    	    while (bphnptr != NULL){
				rctx = bphnptr->phone;
                                tphnptr = install(bphn,lctx,rctx,wpos,tphnhash);
				if (tphnptr->dictcount == 0) swdtphs++;
				tphnptr->dictcount++;
				bphnptr = bphnptr->next;
			    }
			}
			ephnptr = ephnptr->next;
		    }
		}
            }
            else {
                wpos = wordpos2str(WORD_POSN_BEGIN, ignore_wpos);
                bphn = word_el->phones[0];
		if (IS_FILLER(bphn)) {word_el = word_el->next; continue;}
		rctx = word_el->phones[1];
		if (IS_FILLER(rctx)) rctx = silencephn;
		for (j = 0; j < PHNHASHSIZE; j++){
		    ephnptr = ephnhash[j];
		    while (ephnptr != NULL){
			lctx = ephnptr->phone;
                        tphnptr = install(bphn,lctx,rctx,wpos,tphnhash);
			if (tphnptr->dictcount == 0) bwdtphs++;
			tphnptr->dictcount++;
			ephnptr = ephnptr->next;
		    }
		}
                wpos = wordpos2str(WORD_POSN_INTERNAL, ignore_wpos);
                for (j=1;j<lnphns-1;j++){
                    bphn = word_el->phones[j];
		    if (IS_FILLER(bphn)) continue;
                    lctx = word_el->phones[j-1];
		    if (IS_FILLER(lctx)) lctx = silencephn;
                    rctx = word_el->phones[j+1];
		    if (IS_FILLER(rctx)) rctx = silencephn;
                    tphnptr = install(bphn,lctx,rctx,wpos,tphnhash);
		    if (tphnptr->dictcount == 0) iwdtphs++;
		    tphnptr->dictcount++;
                }
                wpos = wordpos2str(WORD_POSN_END, ignore_wpos);
                bphn = word_el->phones[lnphns-1];
		if (IS_FILLER(bphn)) {word_el = word_el->next; continue;}
		lctx = word_el->phones[lnphns-2];
		if (IS_FILLER(lctx)) lctx = silencephn;
		for (j = 0; j < PHNHASHSIZE; j++){
		    bphnptr = bphnhash[j];
		    while (bphnptr != NULL){
			rctx = bphnptr->phone;
                        tphnptr = install(bphn,lctx,rctx,wpos,tphnhash);
			if (tphnptr->dictcount == 0) ewdtphs++;
			tphnptr->dictcount++;
			bphnptr = bphnptr->next;
		    }
		}
            }
	    word_el = word_el->next;
        }
    }
    totaltphs = swdtphs + bwdtphs + iwdtphs + ewdtphs;
    E_INFO("%d words in dictionary\n",totwds);
    E_INFO("%d unique single word triphones in dictionary\n",swdtphs);
    E_INFO("%d unique word beginning triphones in dictionary\n",bwdtphs);
    E_INFO("%d unique word internal triphones in dictionary\n",iwdtphs);
    E_INFO("%d unique word ending triphones in dictionary\n",ewdtphs);

    *triphonehash = tphnhash;

    freephnhash(bphnhash); freephnhash(ephnhash);

    return S3_SUCCESS;
}


int32  count_triphones (const char *transfile,
			dicthashelement_t **dicthash,
			hashelement_t **tphnhash,
			phnhashelement_t ***phnhash,
			int ignore_wpos)
{
    int32  nbwdtphns, newdtphns, niwdtphns, nswdtphns;
    int32  nwords, lnphns, n_totalwds;
    lineiter_t *line = NULL;
    char *tline;
    char   *word, *basephone, *lctxt, *rctxt; 
    char   silencephn[4];
    const char* wpos;
    int32  i, j;
    dicthashelement_t **wordarr;
    phnhashelement_t **lphnhash, *phnptr;
    hashelement_t *tphnptr;
    FILE   *fp;

    strcpy(silencephn,"SIL");
    if ((fp = fopen(transfile,"r")) == NULL)
	E_FATAL("Unable to open transcript file %s for reading!\n",transfile);

    E_INFO("Out of vocabulary words in transcript will be mapped to SIL!\n");

    lphnhash = (phnhashelement_t**)calloc(PHNHASHSIZE,sizeof(phnhashelement_t));

    n_totalwds = 0;
    nbwdtphns = newdtphns = niwdtphns = nswdtphns = 0;

    for (line = lineiter_start_clean(fp); line; line = lineiter_next(line)) {

	tline = strdup(line->buf);
        if (strtok(tline," \t\n") == NULL) {
           free(tline);
    	   continue;
        }

        /* Count number of phones in pronunciation */
        for(nwords=1; strtok(NULL," \t\n") != NULL; nwords++);
	n_totalwds += nwords;
	wordarr = (dicthashelement_t **)ckd_calloc(nwords+2,sizeof(dicthashelement_t*));
        word = strtok(line->buf," \t\n"); 
        if ((wordarr[1] = dictlookup(word,dicthash)) == NULL) {
            E_WARN("Word %s not found in dictionary. Mapping to SIL.\n", word);
        }
        for (j=2; (word = strtok(NULL," \t\n")) != NULL; j++) {
            if ((wordarr[j] = dictlookup(word,dicthash)) == NULL) {
                /* If word is surrounded by "()", assume it's the
                 * utterance ID, and don't report it as an OOV */
                if ((word[0] != '(') && (word[strlen(word) - 1] != ')')) {
                    E_WARN("Word %s not found in dictionary. Mapping to SIL.\n", word);
                }
            }
        }       
	for (i=1; i<=nwords; i++){/* Indices account for padded wordarr array */
	    if (wordarr[i] == NULL) continue;

	    lnphns = wordarr[i]->nphns;
	    if (lnphns == 1) {
		wpos = wordpos2str(WORD_POSN_SINGLE, ignore_wpos);
		basephone = wordarr[i]->phones[0];
		phnptr = phninstall(basephone,lphnhash);
		phnptr->count++;
		if (IS_FILLER(basephone)) continue;
		if (wordarr[i-1] != NULL){
		    lctxt = wordarr[i-1]->phones[wordarr[i-1]->nphns - 1];
		    if (IS_FILLER(lctxt)) lctxt = silencephn;
		}
		else lctxt = silencephn;
		if (wordarr[i+1] != NULL){
		    rctxt = wordarr[i+1]->phones[0];
		    if (IS_FILLER(rctxt)) rctxt = silencephn;
		}
		else rctxt = silencephn;
		tphnptr = lookup(basephone,lctxt,rctxt,wpos,tphnhash);
		(tphnptr->count)++;
		nswdtphns++;
	    }
	    else {
		wpos = wordpos2str(WORD_POSN_BEGIN, ignore_wpos);
		basephone = wordarr[i]->phones[0];
		phnptr = phninstall(basephone,lphnhash);
		phnptr->count++;
		if (IS_FILLER(basephone)) continue;
                if (!IS_FILLER(basephone)){
		    if (wordarr[i-1] != NULL){
		        lctxt = wordarr[i-1]->phones[wordarr[i-1]->nphns - 1];
		        if (IS_FILLER(lctxt)) lctxt = silencephn;
		    }
		    else lctxt = silencephn;
		    rctxt = wordarr[i]->phones[1];
		    if (IS_FILLER(rctxt)) rctxt = silencephn;
		    tphnptr = lookup(basephone,lctxt,rctxt,wpos,tphnhash);
		    (tphnptr->count)++; 
                    nbwdtphns++;
	        }
		wpos = wordpos2str(WORD_POSN_INTERNAL, ignore_wpos);
		for (j=1;j<lnphns-1;j++){
		    basephone = wordarr[i]->phones[j];
		    phnptr = phninstall(basephone,lphnhash);
		    phnptr->count++;
		    if (IS_FILLER(basephone)) continue;
		    lctxt = wordarr[i]->phones[j-1];
		    if (IS_FILLER(lctxt)) lctxt = silencephn;
		    rctxt = wordarr[i]->phones[j+1];
		    if (IS_FILLER(rctxt)) rctxt = silencephn;
		    tphnptr = lookup(basephone,lctxt,rctxt,wpos,tphnhash);
		    (tphnptr->count)++;
		    niwdtphns++;
		}

		wpos = wordpos2str(WORD_POSN_END, ignore_wpos);
		basephone = wordarr[i]->phones[lnphns-1];
		phnptr = phninstall(basephone,lphnhash);
		phnptr->count++;
		if (IS_FILLER(basephone)) continue;

		lctxt = wordarr[i]->phones[lnphns-2];
		if (IS_FILLER(lctxt)) lctxt = silencephn;
		if (wordarr[i+1] != NULL){
		    rctxt = wordarr[i+1]->phones[0];
		    if (IS_FILLER(rctxt)) rctxt = silencephn;
		}
		else rctxt = silencephn;
		tphnptr = lookup(basephone,lctxt,rctxt,wpos,tphnhash);
		(tphnptr->count)++;
		newdtphns++;
	    }
	}
        free(wordarr);
        free(tline);
    }
    fclose(fp);
    lineiter_free(line);

    *phnhash = lphnhash;
    E_INFO("%d words in transcripts\n",n_totalwds);
    E_INFO("%d single word triphones in transcripts\n",nswdtphns);
    E_INFO("%d word beginning triphones in transcripts\n",nbwdtphns);
    E_INFO("%d word internal triphones in transcripts\n",niwdtphns);
    E_INFO("%d word ending triphones in transcripts\n",newdtphns);

    return S3_SUCCESS;
}


int32 find_threshold(hashelement_t  **triphonehash)
{
    hashelement_t *triphone_el;
    int32 tottph, ltottph, mincnt, maxtph, *countofcounts, *lcountofcounts;
    int32 i, cnt, unique, threshold, ceiling;

    mincnt = cmd_ln_int32("-minocc");
    maxtph = cmd_ln_int32("-maxtriphones");

    ceiling = mincnt < CEILING ? CEILING : mincnt+1;

    tottph = ltottph = unique = 0;
    countofcounts = (int32 *) ckd_calloc(ceiling+1,sizeof(int32));
    lcountofcounts = (int32 *) ckd_calloc(ceiling+1,sizeof(int32));
    for (i = 0; i < HASHSIZE; i++){
	triphone_el = triphonehash[i];
	while (triphone_el != NULL){
	    cnt = triphone_el->count;
	    if (cnt > 0) {
	        tottph += cnt;
	        if (cnt >= mincnt) {
	            ltottph ++;
		    if (cnt > ceiling) cnt = ceiling;
		    lcountofcounts[cnt]++;
	        }
	        countofcounts[cnt]++; unique++;
	    }
	    triphone_el = triphone_el->next;
	}
    }

    threshold = mincnt; 
    while (ltottph > maxtph) {
	ltottph -= lcountofcounts[threshold];
	threshold++;
    }

    E_INFO("%d triphones extracted from transcripts\n",tottph);
    E_INFO("%d unique triphones extracted from transcripts\n",unique);
    E_INFO("%d triphones occur once in the transcripts\n",countofcounts[1]);
    E_INFO("%d triphones occur twice in the transcripts\n",countofcounts[2]);
    E_INFO("%d triphones occur thrice in the transcripts\n",countofcounts[3]);
    E_INFO("The rest of the triphones occur more than three times\n");
    E_INFO("Count threshold is %d\n",threshold);

    ckd_free(lcountofcounts); ckd_free(countofcounts);
    return(threshold);
}


int32 make_CD_heap(hashelement_t  **triphonehash,
		   int32  threshold,
		   heapelement_t ***CDheap,
		   int32  *cdheapsize)
{
    heapelement_t **heap=NULL, *addtriphone;
    hashelement_t *triphone_el;
    int32 i, heapsize;

    heapsize = 0;
    for (i = 0; i < HASHSIZE; i++){
	triphone_el = triphonehash[i];
	while (triphone_el != NULL){
	    if (triphone_el->count >= threshold) {
		addtriphone = (heapelement_t *) calloc(1,sizeof(heapelement_t));
		if (addtriphone == NULL)
		    E_FATAL("Heap install error. Out of memory!\n");
		addtriphone->basephone = strdup(triphone_el->basephone);
		addtriphone->leftcontext = strdup(triphone_el->leftcontext);
		addtriphone->rightcontext = strdup(triphone_el->rightcontext);
	        addtriphone->wordposition = strdup(triphone_el->wordposition);
		heapsize = insert(&heap, heapsize, addtriphone);
            }
	    triphone_el = triphone_el->next;
        }
    }
    *CDheap = heap;
    *cdheapsize = heapsize;

    return S3_SUCCESS;
}


int32   print_counts(const char *countfn, phnhashelement_t  **CIhash,
		     hashelement_t **CDhash)
{
    heapelement_t **CDheap=NULL, **CIheap=NULL, *addphone, *addtriphone;
    hashelement_t *triphone_el; 
    phnhashelement_t *ciphone_el;
    int32 i, heapsize,cdheapsize,ciheapsize;
    FILE  *ofp;

    if ((ofp = fopen(countfn,"w")) == NULL){
	E_WARN("Unable to open %s for writing. Not writing counts!\n",countfn);
        return S3_ERROR;
    }
    fprintf(ofp,"base\tleft\tright\twdpos\tcount\n");

    ciheapsize = 0;
    for (i = 0; i < PHNHASHSIZE; i++){
	ciphone_el = CIhash[i];
	while (ciphone_el != NULL){
	    if (ciphone_el->count < 1) {
		ciphone_el = ciphone_el->next;
		continue;
	    }
	    addphone = (heapelement_t *) ckd_calloc(1,sizeof(heapelement_t));
	    addphone->basephone = strdup(ciphone_el->phone);
	    addphone->count = ciphone_el->count;
	    ciheapsize = insert(&CIheap, ciheapsize, addphone);
	    ciphone_el = ciphone_el->next;
        }
    }
    heapsize = ciheapsize;
    for (i = 0; i < heapsize; i++) {
        addphone = yanktop(&CIheap,ciheapsize,&ciheapsize);
	fprintf(ofp,"%s\t-\t-\t-\t%d\n",addphone->basephone,addphone->count);
	free_heapelement(addphone);
    }

    cdheapsize = 0;
    for (i = 0; i < HASHSIZE; i++){
	triphone_el = CDhash[i];
	while (triphone_el != NULL){
	    if (triphone_el->count < 1) {
		triphone_el = triphone_el->next;
		continue;
	    }
	    addtriphone = (heapelement_t *) ckd_calloc(1,sizeof(heapelement_t));
	    addtriphone->basephone = strdup(triphone_el->basephone);
	    addtriphone->leftcontext = strdup(triphone_el->leftcontext);
	    addtriphone->rightcontext = strdup(triphone_el->rightcontext);
	    addtriphone->wordposition = strdup(triphone_el->wordposition);
	    addtriphone->count = triphone_el->count;
	    cdheapsize = insert(&CDheap, cdheapsize, addtriphone);
	    triphone_el = triphone_el->next;
        }
    }
    heapsize = cdheapsize;
    for (i = 0; i < heapsize; i++) {
        addtriphone = yanktop(&CDheap,cdheapsize,&cdheapsize);
	fprintf(ofp,"%s\t%s\t%s\t%s\t%d\n",addtriphone->basephone,
					   addtriphone->leftcontext,
					   addtriphone->rightcontext,
					   addtriphone->wordposition,
					   addtriphone->count);
	free_heapelement(addtriphone);
    }
    fclose(ofp);
    return S3_SUCCESS;
}

static const char* wordpos2str(word_posn_t wordpos, int ignore_wordpos)
{
    char *result;

    if (ignore_wordpos) {
	if (wordpos == WORD_POSN_UNDEFINED)
	    return "-";
	return "i";
    }

    switch (wordpos) {
	case WORD_POSN_SINGLE: 
	    result = "s";
	    break;
	case WORD_POSN_BEGIN: 
	    result = "b";
	    break;
	case WORD_POSN_INTERNAL: 
	    result = "i";
	    break;
	case WORD_POSN_END: 
	    result = "e";
	    break;
	default:
	    result = "-";
	    break;
    }
    return result;
}

word_posn_t posnstr2wordpos(char *posn_str)
{
    if (posn_str == NULL) {
	E_WARN("No word position defined. Assuming word internal\n");
        return WORD_POSN_INTERNAL;
    }
    if (posn_str[0] == 's') return WORD_POSN_SINGLE;
    if (posn_str[0] == 'b') return WORD_POSN_BEGIN;
    if (posn_str[0] == 'i') return WORD_POSN_INTERNAL;
    if (posn_str[0] == 'e') return WORD_POSN_END;
    if (posn_str[0] == 'u') return WORD_POSN_UNDEFINED;

    E_WARN("unknown word position %s; assuming word internal\n", posn_str);
    return WORD_POSN_INTERNAL;
}


int32
output_model_def(FILE *fp,
		 char *pgm,
		 char **base_str,
		 char **left_str,
		 char **right_str,
		 char **posn_str,
		 uint32 *tmat,
		 uint32 **state,
		 uint32 n_base,
		 uint32 n_tri,
		 uint32 n_total,
		 uint32 n_state_pm,
		 uint32 n_tied_state,
		 uint32 n_tied_ci_state,
		 uint32 n_tied_tmat)
{
    time_t t;
    uint32 i, j;
    char *at;

    t = time(NULL);
    at = ctime((const time_t *)&t);
    at[strlen(at)-1] = '\0';

    fprintf(fp, "# Generated by %s on %s\n", pgm, at);

    fprintf(fp, "%s\n", MODEL_DEF_VERSION);
    fprintf(fp, "%u n_base\n", n_base);
    fprintf(fp, "%u n_tri\n", n_tri);
    fprintf(fp, "%u n_state_map\n", (n_base + n_tri) * n_state_pm);
    fprintf(fp, "%u n_tied_state\n", n_tied_state);
    fprintf(fp, "%u n_tied_ci_state\n", n_tied_ci_state);
    fprintf(fp, "%u n_tied_tmat\n", n_tied_tmat);
      
    fprintf(fp, "#\n# Columns definitions\n");
    fprintf(fp, "#%4s %3s %3s %1s %6s %4s %s\n",
	    "base", "lft", "rt", "p", "attrib", "tmat",
	    "     ... state id's ...");
    for (i = 0; i < n_total; i++) {
	if ((base_str[i][0] == '+') ||
	    (strncmp(base_str[i], "SIL", 3) == 0)) {
	    fprintf(fp, "%5s %3s %3s %1s %6s %4d",
		    base_str[i],
		    (left_str[i] != NULL ? left_str[i] : "-"),
		    (right_str[i] != NULL ? right_str[i] : "-"),
		    (posn_str[i] != NULL ? posn_str[i] : "-"),
		    "filler",
		    tmat[i]);
	}
	else {
	    fprintf(fp, "%5s %3s %3s %1s %6s %4d",
		    base_str[i],
		    (left_str[i] != NULL ? left_str[i] : "-"),
		    (right_str[i] != NULL ? right_str[i] : "-"),
		    (posn_str[i] != NULL ? posn_str[i] : "-"),
		    "n/a",
		    tmat[i]);
	}
	    

	for (j = 0; j < n_state_pm-1; j++) {
	    fprintf(fp, " %4u", state[i][j]);
	}
	fprintf(fp, "    N\n");
    }
    return S3_SUCCESS;
}


int32 make_mdef_from_list(const char *mdeffile,
			char **CIlist, 
			int32  nciphones,
			heapelement_t **CDheap,
			int32  cdheapsize,
			char  *pgm)
{
    int32  n_base, n_tri, n_state_pm, n_total;
    int32  n_tied_state_ci, n_tied_state_cd, n_tied_tmat;
    char   **base_str, **left_str, **right_str, **posn_str;
    acmod_id_t base, left, right;
    word_posn_t posn;
    acmod_set_t  *acmod_set;
    int32 i,j,k;
    const char *filler_attr[] = {"filler", NULL};
    const char *base_attr[] = {"base", NULL};
    const char *na_attr[] = {"n/a", NULL};
    uint32 *tmat; 
    uint32 **smap;
    heapelement_t *cdphone;
    FILE  *fp;

    n_state_pm = cmd_ln_int32("-n_state_pm") + 1;
    n_base = nciphones;
    n_tri = cdheapsize;
    E_INFO("%d n_base, %d n_tri\n", n_base, n_tri);

    n_tied_state_ci = n_base * (n_state_pm-1);
    n_tied_state_cd = n_tri * (n_state_pm-1);
    n_tied_tmat = n_base;

    n_total = n_base + n_tri;
    base_str = ckd_calloc(n_total, sizeof(char *));
    left_str = ckd_calloc(n_total, sizeof(char *));
    right_str = ckd_calloc(n_total, sizeof(char *));
    posn_str = ckd_calloc(n_total, sizeof(char *));

    tmat = ckd_calloc(n_total, sizeof(uint32));

    smap = (uint32 **)ckd_calloc_2d(n_total, n_state_pm-1, sizeof(uint32));

    for (i = 0; i < n_base; i++) {
        base_str[i] = strdup(CIlist[i]);
    }

    for (j = 0; j < n_tri; j++,i++) {
        cdphone = yanktop(&CDheap,cdheapsize,&cdheapsize);
	base_str[i] = strdup(cdphone->basephone);
	left_str[i] = strdup(cdphone->leftcontext);
	right_str[i] = strdup(cdphone->rightcontext);
	posn_str[i] = strdup(cdphone->wordposition);
	free_heapelement(cdphone);
    }
    if (cdheapsize != 0) 
        E_FATAL("Error in CD heap! Error in dictionary or phone list!\n");

    for (i = 0, k = 0; i < n_total; i++)
        for (j = 0; j < n_state_pm-1; j++)
            smap[i][j] = k++;

    acmod_set = acmod_set_new();

    acmod_set_set_n_ci_hint(acmod_set, n_base);
    acmod_set_set_n_tri_hint(acmod_set, n_tri);

    for (i = 0; i < n_base; i++) {
        if (base_str[i][0] == '+') {
            base = acmod_set_add_ci(acmod_set, base_str[i], filler_attr);
        }
        else {
            base = acmod_set_add_ci(acmod_set, base_str[i], base_attr);
        }
        tmat[base] = base;
    }

    for (; i < n_total; i++) {
        if ((base = acmod_set_name2id(acmod_set, base_str[i])) == NO_ACMOD)
	    E_FATAL("Error in dictionary or phonelist. Bad basephone %s in triphone list!\n",base_str[i]);
        if ((left = acmod_set_name2id(acmod_set, left_str[i])) == NO_ACMOD)
	    E_FATAL("Error in dictionary or phonelist. Bad leftphone %s in triphone list!\n",left_str[i]);
        if ((right = acmod_set_name2id(acmod_set, right_str[i])) == NO_ACMOD)
	    E_FATAL("Error in dictionary or phonelist. Bad rightphone %s in triphone list!\n",right_str[i]);

	posn = posnstr2wordpos(posn_str[i]);

        acmod_set_add_tri(acmod_set, base, left, right, posn, na_attr);
        tmat[i] = base;
    }

    if ((fp = fopen(mdeffile, "w")) == NULL)
        E_FATAL("Unable to open %s for writing!\n",mdeffile);
    output_model_def(fp,
             pgm,
             base_str, left_str, right_str, posn_str, tmat, smap,
             n_base, n_tri, n_total, n_state_pm,
             n_tied_state_ci+n_tied_state_cd, n_tied_state_ci, n_tied_tmat);
    fclose(fp);
    E_INFO("Wrote mdef file %s\n",mdeffile);

    for (j = 0; j < n_tri; j++,i++) {
	if (base_str[j] != NULL) free(base_str[j]);
	if (left_str[j] != NULL) free(left_str[j]);
	if (right_str[j] != NULL) free(right_str[j]);
	if (posn_str[j] != NULL) free(posn_str[j]);
    }
    free(base_str); free(left_str); free(right_str); free(posn_str);
    ckd_free_2d((void**)smap);

    return 0;
}
