/* ====================================================================
 * Copyright (c) 2001 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * File: mk_s2sendump.c
 * 
 * Description: 
 *    Make sendump file in converting S3 models to S2 models
 *
 * Author: 
 *    Alan W Black (awb@cs,cmu.edu)
 * Picked together out of rkm's sen2s2.c
 *********************************************************************/

#include <stdio.h>
#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>

static void output_phonefile(model_def_t *mdef,FILE *phonefd)
{
    int i;
    int basephone;
    acmod_t *multi;

    for (i = 0; i < mdef->n_defn; i++)
    {
	basephone = mdef->defn[i].tmat;
	fprintf(phonefd,"%s",mdef->acmod_set->ci[basephone].name);
	if (i >= mdef->acmod_set->n_ci)
	{   /* a context dependent phone */
	    multi = &mdef->acmod_set->multi[i-mdef->acmod_set->n_ci];
	    fprintf(phonefd,"(%s,%s)",
		    mdef->acmod_set->ci[multi->left_context].name,
		    mdef->acmod_set->ci[multi->right_context].name);
	    if (multi->posn == WORD_POSN_BEGIN)
		fprintf(phonefd,"b");
	    else if (multi->posn == WORD_POSN_INTERNAL)
		fprintf(phonefd,"%s","");
	    else if (multi->posn == WORD_POSN_END)
		fprintf(phonefd,"e");
	    else if (multi->posn == WORD_POSN_SINGLE)
		fprintf(phonefd,"s");
	}
	fprintf(phonefd,
		"\t%d\t%d\t%d\t%d\n",
		(i < mdef->acmod_set->n_ci) ? 0 : -1,
		0,
		mdef->defn[i].tmat,
		i);
    }
}

static void output_mapfile(model_def_t *mdef,FILE *mapfd)
{
    int i,j;
    acmod_t *multi;
    int max_state_num, prev_max_state;

    max_state_num = 0;

    for (i=0; i < mdef->acmod_set->n_ci; i++)
    {
	for (j=0; j < mdef->defn[i].n_state -1; j++)
	    if (mdef->defn[i].state[j] > max_state_num)
		max_state_num = mdef->defn[i].state[j];
    }
    for (i=mdef->acmod_set->n_ci; i < mdef->n_defn; i++)
    {
	if (mdef->defn[i].tmat != mdef->defn[i-1].tmat)
	    prev_max_state = max_state_num;
	for (j=0; j < mdef->defn[i].n_state -1; j++)
	{   
	    multi = &mdef->acmod_set->multi[i-mdef->acmod_set->n_ci];
	    fprintf(mapfd,"%s(%s,%s)",
		    mdef->acmod_set->ci[multi->base].name,
		    mdef->acmod_set->ci[multi->left_context].name,
		    mdef->acmod_set->ci[multi->right_context].name);
	    if (multi->posn == WORD_POSN_BEGIN)
		fprintf(mapfd,"b");
	    else if (multi->posn == WORD_POSN_INTERNAL)
		fprintf(mapfd,"%s","");
	    else if (multi->posn == WORD_POSN_END)
		fprintf(mapfd,"e");
	    else if (multi->posn == WORD_POSN_SINGLE)
		fprintf(mapfd,"s");
	    fprintf(mapfd,"<%d>",j);
	    fprintf(mapfd,"\t%5d\n",mdef->defn[i].state[j]-prev_max_state);
	    if (mdef->defn[i].state[j] > max_state_num)
		max_state_num = mdef->defn[i].state[j];
	}
    }
#if 0
	    k = 0, d=mdef->acmod_set->n_ci;
	    lpmax = mdef->acmod_set->n_ci * n_emit_state - 1;
	    for (p = 0; p < mdef->acmod_set->n_ci; p++) {
		/* CD senones first; find start and end points in S3 data */
		for (pmax = lpmax ; mdef->defn[d].tmat == p; d++)
		{
		    for (d2=0; d2 < n_emit_state; d2++)
			if (mdef->defn[d].state[d2] > pmax)
			    pmax = mdef->defn[d].state[d2];
		}
		sb = lpmax + 1;
		se = pmax;
		lpmax = pmax;
		
		for (i = sb; i <= se; i++) {
		    m = s->sen2mgau[i];
		    assert (m == 0);
		    j = s->mgau2sen_idx[i];
		    assert (j == i);
		    
		    sp[k++] = fw[f].prob[j][c];
		}

		/* CI senones next */
		sb = p * n_emit_state;
		se = sb + n_emit_state - 1;
		
		for (i = sb; i <= se; i++) {
		    m = s->sen2mgau[i];
		    assert (m == 0);
		    j = s->mgau2sen_idx[i];
		    assert (j == i);
		    
		    sp[k++] = fw[f].prob[j][c];
		}
	    }
	    assert (k == mdef->n_tied_state);
#endif
}
	    
int main (int32 argc, char **argv)
{
    model_def_t *m;
    char *mdeffile, *phonefile, *mapfile;
    FILE *mapfd; 
    FILE *phonefd;
    
    parse_cmd_ln(argc, argv);

    mdeffile = (char *)cmd_ln_access("-moddeffn");
    phonefile = (char *)cmd_ln_access("-phonefn");
    mapfile = (char *)cmd_ln_access("-mapfn");

    model_def_read(&m, mdeffile);

    if ((mapfd = fopen(mapfile,"w")) == NULL)
    {
	E_FATAL("can't open mapfile %s for writing\n", mapfile);
    }

    if ((phonefd = fopen(phonefile,"w")) == NULL)
    {
	E_FATAL("can't open phonefile %s for writing\n", phonefile);
    }

    output_phonefile(m,phonefd);
    output_mapfile(m,mapfd);

    fclose(phonefd);
    fclose(mapfd);

    return 0;
}

