/* -*- c-basic-offset: 4 -*- */
/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * File: main.c
 *
 *  Modified to build decision trees with continuous models
 *  Rita Singh, May 1998
 * 
 * Description: 
 *    Build senonic decision trees from discrete pdf's.
 *
 * Author: 
 *    Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"
#include <s3/dtree.h>

#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <s3/pset_io.h>
#include <s3/quest.h>
#include <s3/dtree.h>
#include <s3/metric.h>
#include <s3/div.h>
#include <s3/s3.h>
#include <s3/vector.h>
#include <s3/s3gau_io.h>
#include <s3/gauden.h>

#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/err.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define N_DFEAT	4

static void
mk_stwt(float32 *ostwt, float32 *stwt, uint32 t_s, uint32 n_stwt)
{
    float32 tmp;
    int i, j;

    for (i = 0, tmp = 0; i < n_stwt; i++) {
	j = i - t_s;

	if (j < 0)
	    j = -j;

	ostwt[i] = stwt[j];
	tmp += ostwt[i];
    }
    tmp = 1.0 / tmp;
    for (i = 0; i < n_stwt; i++) {
	ostwt[i] *= tmp;
    }
    E_INFO("nrm stwt: ");
    for (i = 0; i < n_stwt; i++) {
	E_INFOCONT("%5.3f ", ostwt[i]);
    }

    E_INFOCONT("\n");
}


static int
find_triphones(model_def_t *mdef, const char *phn, uint32 *p_s, uint32 *p_e)
{
    uint32 targ_base, p, b;

    targ_base = acmod_set_name2id(mdef->acmod_set, phn);
    *p_s = *p_e = NO_ID;

    if (targ_base == NO_ACMOD)
	return -1;
    if (targ_base >= acmod_set_n_ci(mdef->acmod_set)) {
	*p_s = *p_e = targ_base;
	return 0;
    }

    /* Scan for first, p_s, and last phone, p_e,
	 * which have the target base phone */
    for (p = acmod_set_n_ci(mdef->acmod_set);
	 p < acmod_set_n_acmod(mdef->acmod_set); p++) {
	b = acmod_set_base_phone(mdef->acmod_set, p);
	if ((b == targ_base) && (*p_s == NO_ID)) {
	    *p_s = p;
	}
	else if ((b == targ_base) && (*p_s != NO_ID) && (*p_e != NO_ID)) {
	    E_FATAL("n-phones with base phone %s occur non-consecutively in the mdef file\n",
		    phn);
	}
	else if ((b != targ_base) && (*p_s != NO_ID) && (*p_e == NO_ID)) {
	    *p_e = p-1;
	}
    }

    /* If we've reached the end of the list,
	 * then the last phone in the list is the last
	 * target phone */
    if ((p == acmod_set_n_acmod(mdef->acmod_set)) &&
	(*p_e == NO_ID)) {
	*p_e = p-1;
    }

    if (*p_s == NO_ID) {
	E_WARN("No triphones involving %s\n", phn);
	return -1;
    }
    return 0;
}

static int
init(model_def_t **out_mdef,
     const char *phn,
     uint32 state,
     uint32 *out_mixw_s,
     float32 *****out_mixw,
     float32 *****out_mixw_occ,
     float32 *****out_mean,
     float32 *****out_var,
     uint32   **out_veclen,
     uint32 *out_n_model,
     uint32 *out_n_state,
     uint32 *out_n_stream,
     uint32 *out_n_density,
     float32 **out_stwt,
     uint32 ***out_dfeat,
     pset_t **out_pset,
     uint32 *out_n_pset,
     quest_t **out_all_q,
     uint32 *out_n_all_q)
{
    const char *moddeffn;
    const char *mixwfn;
    const char *psetfn;
    model_def_t *mdef;
    uint32 p, p_s = NO_ID, p_e = NO_ID, s, m;
    int allphones;
    uint32 mixw_s, mixw_e;
    uint32 **dfeat;
    pset_t *pset;
    uint32 n_pset;
    acmod_id_t b, l, r;
    word_posn_t pn;
    float32 ****mixw;
    float32 ****mixw_occ;
    float32 ***in_mixw;
    uint32 n_state, n_model, n_in_mixw, n_stream, n_density;
    uint32 i, j, k;
    float32 *stwt;
    float32 *istwt;
    const char **stwt_str;
    quest_t *all_q;
    uint32 n_l_q, n_r_q;
    uint32 n_all_q;
    uint32 n_phone_q, n_wdbndry;
    float64 norm;
    float64 dnom;
    float32 mwfloor;
    float64 wt_ent;
    float64 s_wt_ent=0;
    const char*   type;
    uint32  continuous;
    uint32  *l_veclen, *t_veclen;
    uint32  l_nstates, t_nstates;
    uint32  t_nfeat, t_ndensity;
    vector_t  ***fullmean;
    vector_t  ***fullvar = NULL;
    vector_t  ****fullvar_full = NULL;
    float32   ****mean;
    float32   ****var;
    float32   varfloor;
    uint32    mm, kk,ll,n,nn,sumveclen;
    
    char      *cntflag;
    float32   cntthreshold,stcnt;

    

    moddeffn = cmd_ln_str("-moddeffn");
    if (moddeffn == NULL)
	E_FATAL("Specify -moddeffn\n");

    E_INFO("Reading: %s\n", moddeffn);
    if (model_def_read(&mdef, moddeffn) != S3_SUCCESS)
	return S3_ERROR;

    *out_mdef = mdef;

    allphones = cmd_ln_int32("-allphones");
    if (allphones) {
      p_s = acmod_set_n_ci(mdef->acmod_set);
      p_e = acmod_set_n_acmod(mdef->acmod_set)-1;
    }
    else if (phn) {
	if (find_triphones(mdef, phn, &p_s, &p_e) == -1)
	  return S3_ERROR;
    }
    else {
      E_FATAL("No -phone, -start_phone, or -end_phone specified!\n");
    }

    E_INFO("Building trees for [%s]", acmod_set_id2name(mdef->acmod_set, p_s));
    E_INFOCONT(" through [%s]\n", acmod_set_id2name(mdef->acmod_set, p_e));

    for (p = p_s, i = mdef->defn[p_s].state[0]-1; p <= p_e; p++) {
	for (j = 0; j < mdef->defn[p].n_state; j++) {
	    if (mdef->defn[p].state[j] != TYING_NON_EMITTING) {
		if (mdef->defn[p].state[j] != i+1) {
		    E_ERROR("States in triphones for %s are not consecutive\n", phn);
		    
		    return S3_ERROR;
		}
		
		i = mdef->defn[p].state[j];
	    }
	}
    }

    cntflag = (char *)ckd_calloc(p_e-p_s+1,sizeof(char));
    cntthreshold = cmd_ln_float32("-cntthresh");

    /* Find first and last mixing weight used for p_s through p_e */
    mixw_s = mdef->defn[p_s].state[0];
    mixw_e = mdef->defn[p_e].state[mdef->defn[p_e].n_state-2];

    *out_mixw_s = mixw_s;

    E_INFO("Covering states |[%u %u]| == %u\n",
	   mixw_s, mixw_e, mixw_e - mixw_s + 1);

    mixwfn = cmd_ln_str("-mixwfn");
    if (mixwfn == NULL)
	E_FATAL("Specify -mixwfn\n");

    E_INFO("Reading: %s\n", mixwfn);
    if (s3mixw_intv_read(mixwfn, mixw_s, mixw_e,
			 &in_mixw,
			 &n_in_mixw,
			 &n_stream,
			 &n_density) != S3_SUCCESS)
	return S3_ERROR;

    *out_n_stream = n_stream;
    *out_n_density = n_density;
    n_state = mdef->defn[p_s].n_state - 1;
    *out_n_state = n_state;

    for (i = p_s+1; i <= p_e; i++) {
	if ((mdef->defn[i].n_state - 1) != n_state) {
	    E_FATAL("Models do not have uniform topology\n");
	}
    }

    for (i=p_s,j=0,mm=0; i<=p_e ;i++,j++) { 
        cntflag[j] = 1;
        for (k=0; k < n_state; k++) {
            s = mdef->defn[i].state[k] - mixw_s;
            for (kk=0; kk<n_stream; kk++) {
                stcnt = 0;
                for (ll=0; ll<n_density; ll++) {
                    stcnt += in_mixw[s][kk][ll];
                }
                if (stcnt < cntthreshold) cntflag[j] = 0;
            }
        }
        if (cntflag[j]==1) mm++;
    }

    n_model = mm;
    *out_n_model = n_model;
    E_INFO("%d of %d models have observation count greater than %f\n",n_model,p_e-p_s+1,cntthreshold);
        

    /* Allocate the state weight array for weighting the 
     * similarity of neighboring states */
    istwt = ckd_calloc(n_state, sizeof(float32));
    stwt = ckd_calloc(n_state, sizeof(float32));
    *out_stwt = stwt;
    stwt_str = cmd_ln_str_list("-stwt");
    if (stwt_str == NULL) {
	E_FATAL("Specify state weights using -stwt\n");
    }

    for (i = 0; i < n_state; i++) {
	if (stwt_str[i] == NULL)
	    E_FATAL("Fewer state weights than states\n");

	istwt[i] = atof(stwt_str[i]);
    }
    if (stwt_str[i] != NULL) {
	E_FATAL("More weights than states\n");
    }

    /* Normalize the weights so they sum to 1.0 */
    for (i = 0, norm = 0; i < n_state; i++) 
	norm += istwt[i];
    norm = 1.0 / norm;
    for (i = 0; i < n_state; i++)
	istwt[i] *= norm;

    mk_stwt(stwt, istwt, state, n_state);
    ckd_free((void *)istwt);

    /*
     * Build the 4D array:
     *
     *     mixw[0..n_model-1][0..n_state-1][0..n_feat-1][0..n_density-1]
     *
     * out of the usual mixw[][][] array
     *
     */
    mixw_occ = (float32 ****)ckd_calloc_2d(n_model, n_state, sizeof(float32 **));
    mixw     = (float32 ****)ckd_calloc_4d(n_model, n_state, n_stream, n_density,
					   sizeof(float32));
    *out_mixw_occ = mixw_occ;
    *out_mixw = mixw;

    for (i = p_s, j = 0, mm = 0; i <= p_e; i++, mm++) {
        if (cntflag[mm]==1) {
	    for (k = 0; k < n_state; k++) {
	        s = mdef->defn[i].state[k] - mixw_s;
	        mixw_occ[j][k] = in_mixw[s];
	    }
            j++;
	}
    }

    assert(j == n_model);
    mwfloor = cmd_ln_float32("-mwfloor");

    type = cmd_ln_str("-ts2cbfn");
    if (strcmp(type,".semi.")!=0 && strcmp(type,".cont.") != 0)
        E_FATAL("Type %s unsupported; trees can only be built on types .semi. or .cont.\n",type);
    if (strcmp(type,".cont.") == 0) 
        continuous = 1;
    else 
        continuous = 0;

#if 0 /* This is rather arbitrary (in actual fact we should treat all models as continuous) */
   /* Additional check for meaningless input */
    if (continuous == 0 && n_density < 256) {
        E_FATAL("Attempt to build trees on semi-continuous HMMs with %d < 256 gaussians!\n****A minimum of 256 gaussians are expected!\n",n_density);
    }
#endif

    for (s = 0, wt_ent = 0; s < n_state; s++) {
	if (continuous==0) s_wt_ent = 0;
	for (i = 0; i < n_model; i++) {
	    for (j = 0; j < n_stream; j++) {
		/* The denominators for each stream should be nearly
		   identical, but... */
		for (k = 0, dnom = 0; k < n_density; k++) {
		    dnom += mixw_occ[i][s][j][k];
		}
		if (dnom != 0) {
		    norm = 1.0 / dnom;
		    for (k = 0; k < n_density; k++) {
			mixw[i][s][j][k] = mixw_occ[i][s][j][k] * norm;
			if (mixw[i][s][j][k] < mwfloor) {
			    mixw[i][s][j][k] = mwfloor;
			}
		    }
		    if (continuous == 0)
                        s_wt_ent += dnom * ent_d(mixw[i][s][j], n_density);
		}
	    }
	}
	if (continuous == 0) wt_ent += stwt[s] * s_wt_ent;
    }

    if (continuous == 0) E_INFO("%u-class entropy: %e\n", n_model, wt_ent);

    if (continuous == 1) {
	int32 var_is_full = cmd_ln_int32("-fullvar");
        /* Read Means and Variances; perform consistency checks */
        if (s3gau_read(cmd_ln_str("-meanfn"),
                       &fullmean,
                       &l_nstates,
                       &t_nfeat,
                       &t_ndensity,
                       &l_veclen) != S3_SUCCESS)
            E_FATAL("Error reading mean file %s\n",cmd_ln_str("-meanfn"));
        *out_veclen = l_veclen;
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

        if (t_ndensity > 1)
            E_WARN("The state distributions given have %d gaussians per state;\n..*..shrinking them down to 1 gau per state..\n",t_ndensity);

        /* Allocate for out_mean and out_var. If input are multi_gaussian
           distributions convert to single gaussians. Copy appropriate
           states to out_mean and out_var */
        for (i=0,sumveclen=0; i < n_stream; i++) sumveclen += t_veclen[i];
        mean = (float32 ****)ckd_calloc_4d(n_model,n_state,n_stream,sumveclen,sizeof(float32));
	/* Use only the diagonals regardless of whether -varfn is full. */
        var = (float32 ****)ckd_calloc_4d(n_model,n_state,n_stream,sumveclen,sizeof(float32));
        varfloor = cmd_ln_float32("-varfloor");
 
        for (i = p_s, j = 0, m = mixw_s, mm = 0; i <= p_e; i++, mm++) {
            if (cntflag[mm]==1) {
                for (k = 0; k < n_state; k++, m++) {
                    for (ll = 0; ll < n_stream; ll++) {
                        float32 *featmean,*featvar;

                        featmean = mean[j][k][ll];
                        featvar = var[j][k][ll];
                        dnom  = 0;
                        for (n = 0; n < n_density; n++) {
                            float32 mw = mixw_occ[j][k][ll][n];
                            dnom += mw;
                            for (nn = 0; nn < l_veclen[ll]; nn++) {
                                featmean[nn] += mw * fullmean[m][ll][n][nn];
				if (var_is_full)
				    featvar[nn] += 
					mw *(fullmean[m][ll][n][nn]*fullmean[m][ll][n][nn] +
					     fullvar_full[m][ll][n][nn][nn]);
				else
				    featvar[nn] += 
					mw *(fullmean[m][ll][n][nn]*fullmean[m][ll][n][nn] +
					     fullvar[m][ll][n][nn]);
                            }
                        }
                        if (dnom != 0) {
                            for (nn = 0; nn < l_veclen[ll]; nn++) {
                                featmean[nn] /= dnom;
                                featvar[nn] = featvar[nn]/dnom - featmean[nn]*featmean[nn];
                                if (featvar[nn] < varfloor) featvar[nn] = varfloor;
                            }
                        }
                        else {
                            for (nn = 0; nn < l_veclen[ll]; nn++) {
                                if (featmean[nn] != 0)
                                    E_FATAL("dnom = 0, but featmean[nn] != 0, =  %f for ll = %d\n",featmean[nn],ll);
                            }
                        }
                      /* Now on we need only have global counts for the mixws,
                         so we store them all in mixw_occ[][][0] */
                        mixw_occ[j][k][ll][0] = dnom;
                    }
                }
                j++;
            }
            else m += n_state; /* account for states of skipped model */
        }
        assert(j == n_model);

        /* Now n_density = 1 */
        n_density = 1;
        *out_n_density = n_density;
        *out_mean = mean;
        *out_var = var;
        ckd_free_4d((void ****)fullmean);
	if (fullvar)
	    ckd_free_4d((void ****)fullvar);
	if (fullvar_full)
	    gauden_free_param_full(fullvar_full);
    }


    /*
     * Allocate a set of decision tree features
     * associated with each phone
     */
    dfeat = (uint32 **)ckd_calloc_2d(n_model, N_DFEAT, sizeof(uint32));
    *out_dfeat = dfeat;

    for (p = p_s, j = 0, mm = 0; p <= p_e; p++, mm++) {
        if (cntflag[mm] == 1) {
	    acmod_set_id2tri(mdef->acmod_set, &b, &l, &r, &pn, p);

	    dfeat[j][0] = (uint32)l;
	    dfeat[j][1] = (uint32)b;
	    dfeat[j][2] = (uint32)r;
	    dfeat[j][3] = (uint32)pn;
            j++;
        }
    }
    assert(j == n_model);
    ckd_free(cntflag);

    psetfn = cmd_ln_str("-psetfn");

    E_INFO("Reading: %s\n", psetfn);
    *out_pset = pset = read_pset_file(psetfn, mdef->acmod_set, &n_pset);
    *out_n_pset = n_pset;

    /* Determine the # of phone sets and word boundary
     * questions there are */

    n_l_q = n_r_q = 0;
    for (i = 0, n_phone_q = 0, n_wdbndry = 0; i < n_pset; i++) {
	if (pset[i].member){
            if (strstr(pset[i].name,"_L") != NULL) {
                n_phone_q++;
                n_l_q++;
            }
            else if (strstr(pset[i].name,"_R") != NULL) {
                n_phone_q++;
                n_r_q++;
            }
            else if (allphones)
	      n_phone_q += 3;
	    else
	      n_phone_q += 2;
        }
	else
	    n_wdbndry++;
    }

    /* Compute the total number of simple questions */
    if (allphones) /* Ask questions about the phone itself */
	n_all_q = 2 * n_phone_q + 2 * n_wdbndry;
    else
	n_all_q = 2 * n_phone_q + 2 * n_wdbndry;
    *out_n_all_q = n_all_q;

    /* Allocate an array to hold all the simple questions */
    all_q = ckd_calloc(n_all_q, sizeof(quest_t));
    *out_all_q = all_q;

    E_INFO("%u total simple questions (%u phone; %u word bndry)\n",
	   n_all_q, 2 * n_phone_q, 2 * n_wdbndry);
    E_INFO("%u Left Only questions, and %u Right Only questions\n",
           2*n_l_q, 2*n_r_q);

    /* Generate all the L simple questions to be asked when generating
     * simple trees */
    for (i = 0, l = 0; i < n_pset; i++) {
	if (pset[i].member) {
	    /* Generate the phonetic questions */

            if (strstr(pset[i].name,"_L") != NULL ||
                (strstr(pset[i].name,"_L") == NULL && 
                 strstr(pset[i].name,"_R") == NULL)){
	        all_q[l].pset = i;
	        all_q[l].member = pset[i].member;
	        all_q[l].neg  = FALSE;
	        all_q[l].ctxt = -1;    /* one phone to the left of base phone */
	        l++;
            }
            if (strstr(pset[i].name,"_R") != NULL ||
                (strstr(pset[i].name,"_L") == NULL && 
                 strstr(pset[i].name,"_R") == NULL)){
	        all_q[l].pset = i;
	        all_q[l].member = pset[i].member;
	        all_q[l].neg  = FALSE;
	        all_q[l].ctxt = 1;    /* one phone to the right of base phone */
	        l++;
            }
	    if (allphones
		&& strstr(pset[i].name,"_R") == NULL
		&& strstr(pset[i].name,"_L") == NULL) {
		all_q[l].pset = i;
		all_q[l].member = pset[i].member;
		all_q[l].neg = FALSE;
		all_q[l].ctxt = 0; /* the base phone itself */
		l++;
	    }

	    /* The negations of the above questions */
            if (strstr(pset[i].name,"_L") != NULL ||
                (strstr(pset[i].name,"_L") == NULL && 
                 strstr(pset[i].name,"_R") == NULL)){
	        all_q[l].pset = i;
	        all_q[l].member = pset[i].member;
	        all_q[l].neg  = TRUE;
	        all_q[l].ctxt = -1;
	        l++;
            }
            if (strstr(pset[i].name,"_R") != NULL ||
                (strstr(pset[i].name,"_L") == NULL && 
                 strstr(pset[i].name,"_R") == NULL)){
	        all_q[l].pset = i;
	        all_q[l].member = pset[i].member;
	        all_q[l].neg  = TRUE;
	        all_q[l].ctxt = 1;
	        l++;
            }
	    if (allphones
		&& strstr(pset[i].name,"_R") == NULL
		&& strstr(pset[i].name,"_L") == NULL) {
		all_q[l].pset = i;
		all_q[l].member = pset[i].member;
		all_q[l].neg = TRUE;
		all_q[l].ctxt = 0;
		l++;
	    }
	}
	else if (pset[i].posn) {
	    /* Word position question */
	    all_q[l].pset = i;
	    all_q[l].posn = pset[i].posn;
	    all_q[l].neg = FALSE;
	    l++;

	    /* Negation of the above question */
	    all_q[l].pset = i;
	    all_q[l].posn = pset[i].posn;
	    all_q[l].neg = TRUE;
	    l++;
	}
	else
	  E_ERROR("Invalid phoneme set %s\n", pset[i].name);

    }

    return S3_SUCCESS;
}



int main(int argc, char *argv[])
{
    float32 ****mixw_occ = NULL;
    float32 ****mixw = NULL;
    float32 ****means = NULL;
    float32 ****vars = NULL;
    uint32  *veclen = NULL;
    uint32 mixw_s;
    uint32 n_model = 0;
    uint32 n_state = 0;
    uint32 n_stream = 0;
    uint32 n_density = 0;
    model_def_t *mdef = NULL;
    const char *phn;
    pset_t *pset = NULL;
    uint32 n_pset;
    uint32 **dfeat = NULL;
    quest_t *all_q = NULL;
    uint32 n_all_q = 0;
    uint32 m;
    uint32 *id;
    float32 *stwt = NULL;
    uint32 state;
    dtree_t *tr;
    FILE *fp;
    float32 mwfloor;

    parse_cmd_ln(argc, argv);

    phn = cmd_ln_str("-phone");

    state = cmd_ln_int32("-state");

    if (init(&mdef,
	     phn,
	     state,
	     &mixw_s,
	     &mixw,
	     &mixw_occ,
             &means,
             &vars,
             &veclen,
	     &n_model,
	     &n_state,
	     &n_stream,
	     &n_density,
	     &stwt,
	     &dfeat,
	     &pset,
	     &n_pset,
	     &all_q,
	     &n_all_q) != S3_SUCCESS) {
	E_FATAL("Initialization failed\n");
    }

    mwfloor = cmd_ln_float32("-mwfloor");

    id = (uint32 *)ckd_calloc(n_model, sizeof(uint32));

    /* Initially, all states in the same class */
    for (m = 0; m < n_model; m++) {
	id[m] = m;
    }

    /* Build the composite tree.  Recursively generates
    * the composite decision tree.  See dtree.c in libcommon */
    tr = mk_tree_comp(mixw_occ, means, vars, veclen, n_model, n_state, 
                      n_stream, n_density, stwt,
		      id, n_model,
		      all_q, n_all_q, pset, acmod_set_n_ci(mdef->acmod_set),
		      dfeat, N_DFEAT,
		      cmd_ln_int32("-ssplitmin"),
		      cmd_ln_int32("-ssplitmax"),
		      cmd_ln_float32("-ssplitthr"),
		      cmd_ln_int32("-csplitmin"),
		      cmd_ln_int32("-csplitmax"),
		      cmd_ln_float32("-csplitthr"),
		      mwfloor);

    /* Save it to a file */
    fp = fopen(cmd_ln_str("-treefn"), "w");
    if (fp == NULL) {
	E_FATAL_SYSTEM("Unable to open %s for writing",
		       cmd_ln_str("-treefn"));
    }
    print_final_tree(fp, &tr->node[0], pset);
    fclose(fp);

    return 0;
}
