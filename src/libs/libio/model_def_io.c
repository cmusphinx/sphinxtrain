/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * File: model_def_io.c
 * 
 * Description: 
 * 
 * Author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/


#include <sphinxbase/pio.h>
#include <s3/acmod_set.h>
#include <s3/model_def_io.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>

#include <assert.h>

#include <string.h>

#define NO_NUMBER	(0xffffffff)


/* Since strtok()'s don't nest, I wrote this stupid
 * thing. */
char *comma_whacker(char *s)
{
    static char *p;
    static char *nxt;
    char *ret;

    if (s != NULL)
	p = s;

    ret = p;
    
    if (ret != NULL) {
	nxt = strchr(p, ',');

	if (nxt) {
	    *nxt = '\0';
	}

	p = nxt;
    }

    return ret;
}


const char **
mk_attrib_list(char *comma_sep_attrib)
{
    static char *attrib_list[MAX_N_ATTRIB+1];
    static char buf[1024];
    uint32 i;
    char *attr;

    strncpy (buf, comma_sep_attrib, 1024);

    attr = comma_whacker(buf);
    i = 0;

    do {
	attrib_list[i++] = attr;
    } while ((attr = comma_whacker(NULL)) && (i < MAX_N_ATTRIB));
    attrib_list[i] = NULL;

    if (i == MAX_N_ATTRIB+1) {
	E_WARN("Attribute list may be truncated for acoustic model, increase MAX_N_ATTRIB\n");
    }
    
    return (const char **)attrib_list;
}

static int32
parse_rem(const char ***attrib,
	  uint32 *tmat,
	  uint32 *state,
	  uint32 *n_state,
	  uint32 n_read)
{
    unsigned int max_n_state;
    uint32 i;
    char *tok;

    *attrib = mk_attrib_list(strtok(NULL, " \t"));

    *tmat = atoi(strtok(NULL, " \t"));

    max_n_state = *n_state;

    for (i = 0; (i < max_n_state) && (tok = strtok(NULL, " \t")); i++) {
	if (tok[0] != 'N')
	    state[i] = atoi(tok);
	else
	    state[i] = TYING_NON_EMITTING;
    }
    
    if (i <= max_n_state)
	*n_state = i;
    else {
	fflush(stdout);
	fprintf(stderr, "%s(%d): number of states for phone exceeds max, %d at line %d\n",
		__FILE__, __LINE__,
		max_n_state, n_read);
	fflush(stderr);
	
	return S3_ERROR;
    }

    return S3_SUCCESS;
}

static int32
parse_base_line(char *line,
		int lineno,
		acmod_id_t *acmod_id,
		uint32 *tmat,
		uint32 *state,
		uint32 *n_state,
		acmod_set_t *acmod_set)
{
    char *tok;
    unsigned int n;
    const char **attrib;

    tok = strtok(line, " \t");

    if (acmod_set_name2id(acmod_set, tok) != NO_ACMOD) {
	E_ERROR("%s multiply defined at line %d\n", tok, lineno);
	return S3_ERROR;
    }

    for (n = 0; (n < 3) && (strcmp("-", strtok(NULL, " \t")) == 0); n++);

    if (n < 3) {
	E_ERROR("Expected l, r and posn to be \"-\" for ci phone %s at line %d, currently '%s'\n", tok, lineno, line);
	return S3_ERROR;
    }

    if (parse_rem(&attrib, tmat, state, n_state, lineno) != S3_SUCCESS) {
        return S3_ERROR;
    }

    *acmod_id = acmod_set_add_ci(acmod_set, tok, attrib);
    
    return S3_SUCCESS;
}

static int32
parse_tri_line(char *line,
	       int lineno,
	       acmod_id_t *acmod_id,
	       uint32 *tmat,
	       uint32 *state,
	       uint32 *n_state,
	       acmod_set_t *acmod_set)
{
    char *tok;
    unsigned int i;
    uint32 id;
    char *posn_map = WORD_POSN_CHAR_MAP;
    const char **attrib;
    acmod_id_t base, left, right;
    word_posn_t posn;

    tok = strtok(line, " \t");

    if ((id = acmod_set_name2id(acmod_set, tok)) == NO_ACMOD) {
	E_ERROR("%s is an undefined base phone at line %d\n",
		tok, lineno);
	return S3_ERROR;
    }

    base = id;

    tok = strtok(NULL, " \t");

    if ((id = acmod_set_name2id(acmod_set, tok)) == NO_ACMOD) {
	E_ERROR("%s is an undefined base phone at line %d\n",
		tok, lineno);
	return S3_ERROR;
    }

    left = id;

    tok = strtok(NULL, " \t");

    if ((id = acmod_set_name2id(acmod_set, tok)) == NO_ACMOD) {
	E_ERROR("%s is an undefined base phone at line %d\n",
		tok, lineno);
	return S3_ERROR;
    }

    right = id;

    tok = strtok(NULL, " \t");

    for (i = 0; i < strlen(posn_map); i++)
	if (tok[0] == posn_map[i])
	    break;
    
    if (i < strlen(posn_map)) {
	posn = (word_posn_t)i;
    }
    else {
	E_ERROR("Unknown word posn %s found at line %d\n",
		tok, lineno);
	return S3_ERROR;
    }

    if (parse_rem(&attrib, tmat, state, n_state, lineno) != S3_SUCCESS) {
	return S3_ERROR;
    }
    
    *acmod_id = acmod_set_add_tri(acmod_set, base, left, right, posn, attrib);
    if (*acmod_id == NO_ACMOD) {
	return S3_ERROR;
    }

    return S3_SUCCESS;
}

void
update_totals(model_def_t *mdef,
	      model_def_entry_t *defn)
{
    if (defn->n_state > mdef->max_n_state)
	mdef->max_n_state = defn->n_state;

    if (defn->n_state < mdef->min_n_state)
	mdef->min_n_state = defn->n_state;
}


int32
model_def_write(model_def_t *mdef,
		const char *fn)
{
    FILE *fp;
    uint32 n_ci;
    uint32 n_acmod;
    acmod_set_t *acmod_set;
    const char **attrib;
    acmod_id_t b;
    acmod_id_t l;
    acmod_id_t r;
    word_posn_t wp;
    uint32 p;
    char *wp2c = WORD_POSN_CHAR_MAP;
    uint32 i;

    fp = fopen(fn, "w");
    if (fp == NULL) {
	E_ERROR_SYSTEM("Unable to open %s for writing", fn);
	return S3_ERROR;
    }

    acmod_set = mdef->acmod_set;

    fprintf(fp, "%s\n", MODEL_DEF_VERSION);
    n_ci = acmod_set_n_ci(acmod_set);
    fprintf(fp, "%u n_base\n", n_ci);
    fprintf(fp, "%u n_tri\n", acmod_set_n_multi(acmod_set));
    fprintf(fp, "%u n_state_map\n", mdef->n_total_state);
    fprintf(fp, "%u n_tied_state\n", mdef->n_tied_state);
    fprintf(fp, "%u n_tied_ci_state\n", mdef->n_tied_ci_state);
    fprintf(fp, "%u n_tied_tmat\n", mdef->n_tied_tmat);
    fprintf(fp, "#\n# Columns definitions\n");
    fprintf(fp, "#%4s %3s %3s %1s %6s %4s %s\n",
	    "base", "lft", "rt", "p", "attrib", "tmat",
	    "     ... state id's ...");

    n_acmod = acmod_set_n_acmod(acmod_set);
    for (p = 0; p < n_ci; p++) {
	fprintf(fp, "%5s %3s %3s %1s",
		acmod_set_id2name(acmod_set, p),
		"-", "-", "-");

	attrib = acmod_set_attrib(acmod_set, p);
	if ((attrib == NULL) || (attrib[0] == NULL)) {
	    fprintf(fp, " %6s", "n/a");
	}
	else {
	    fprintf(fp, " %6s", attrib[0]);

	    for (i = 1; attrib[i]; i++) {
		fprintf(fp, ",%s", attrib[i]);
	    }
	}
	fprintf(fp, " %4d", mdef->defn[p].tmat);

	for (i = 0; i < mdef->defn[p].n_state; i++) {
	    if (mdef->defn[p].state[i] == NO_ID) {
		fprintf(fp, " N");
	    }
	    else {
		fprintf(fp, " %6u", mdef->defn[p].state[i]);
	    }
	}
	fprintf(fp, "\n");
    }

    for (; p < n_acmod; p++) {
	acmod_set_id2tri(acmod_set,
			 &b, &l, &r, &wp,
			 p);
	
	fprintf(fp, "%5s %3s %3s %c",
		acmod_set_id2name(acmod_set, b),
		acmod_set_id2name(acmod_set, l),
		acmod_set_id2name(acmod_set, r),
		wp2c[(uint32)wp]);

	attrib = acmod_set_attrib(acmod_set, p);
	if ((attrib == NULL) || (attrib[0] == NULL)) {
	    fprintf(fp, " %6s", "n/a");
	}
	else {
	    fprintf(fp, " %6s", attrib[0]);

	    for (i = 1; attrib[i]; i++) {
		fprintf(fp, ",%s", attrib[i]);
	    }
	}
	fprintf(fp, " %4d", mdef->defn[p].tmat);

	for (i = 0; i < mdef->defn[p].n_state; i++) {
	    if (mdef->defn[p].state[i] == NO_ID) {
		fprintf(fp, " N");
	    }
	    else {
		fprintf(fp, " %6u", mdef->defn[p].state[i]);
	    }
	}
	fprintf(fp, "\n");
    }

    fclose(fp);

    return S3_SUCCESS;
}

int32
model_def_read(model_def_t **out_model_def,
	       const char *file_name)
{
    lineiter_t *li = NULL;
    uint32 n;
    char tag[32];
    acmod_set_t *acmod_set;
    uint32 i, j;
    acmod_id_t acmod_id;
    uint32 tmat;
    uint32 n_state;
    uint32 n_tri;
    uint32 n_base;
    uint32 n_total_map;
    uint32 n_tied_state;
    uint32 n_tied_ci_state;
    uint32 n_tied_tmat;
    uint32 state[MAX_N_STATE];
    uint32 n_total;
    model_def_t *omd;
    model_def_entry_t *mdef;
    uint32 *all_state;
    uint32 max_tmat;
    uint32 max_state;
    uint32 max_ci_state;
    
    FILE *fp;

    fp = fopen(file_name, "r");
    if (fp == NULL) {
	E_ERROR_SYSTEM("Unable to open %s for reading",
		      file_name);

	return S3_ERROR;
    }
    
    li = lineiter_start_clean(fp);

    if (li == NULL) {
	E_ERROR("ERROR not even a version number in %s!?\n",
		file_name);

	fclose(fp);

        lineiter_free(li);
	return S3_ERROR;
    }

    if (strcmp(li->buf, MODEL_DEF_VERSION) != 0) {
	E_ERROR("ERROR version(%s) == \"%s\", but expected %s at line %d.\n",
		file_name, li->buf, MODEL_DEF_VERSION, lineiter_lineno(li));

	fclose(fp);
	
	if (strcmp(li->buf, "0.1") == 0) {
	    E_ERROR("You must add an attribute field to all the model records.  See SPHINX-III File Formats manual\n");
	}
	
	if (strcmp(li->buf, "0.2") == 0) {
	    E_ERROR("You must add n_tied_state, n_tied_ci_state and n_tied_tmat definitions at the head of the file.  See /net/alf19/usr2/eht/s3/cvtmdef.csh\n");
	}
	
        lineiter_free(li);
	return S3_ERROR;
    }

    n_tri = n_base = n_total_map = n_tied_state = n_tied_ci_state = n_tied_tmat = NO_NUMBER;
    for ( i = 0; i < 6; i++) {
        li = lineiter_next(li);
        if (li == NULL) {
	    E_ERROR("Incomplete count information in %s!?\n",
		    file_name);
	    
	    fclose(fp);
            lineiter_free(li);
	    return S3_ERROR;
	}

	sscanf(li->buf, "%u %s", &n, tag);

	if (strcmp(tag, "n_base") == 0) {
	    n_base = n;
	}	
	else if (strcmp(tag, "n_tri") == 0) {
	    n_tri = n;
	}
	else if (strcmp(tag, "n_state_map") == 0) {
	    n_total_map = n;
	}
	else if (strcmp(tag, "n_tied_state") == 0) {
	    n_tied_state = n;
	}
	else if (strcmp(tag, "n_tied_ci_state") == 0) {
	    n_tied_ci_state = n;
	}
	else if (strcmp(tag, "n_tied_tmat") == 0) {
	    n_tied_tmat = n;
	}
	else {
	    E_ERROR("Unknown tag %s in file at line %d\n",
		    tag, lineiter_lineno(li));
	    	    
	    fclose(fp);

            lineiter_free(li);
	    return S3_ERROR;
	}
    }
    li = lineiter_next(li);

    *out_model_def = omd = ckd_calloc(1, sizeof(model_def_t));
    omd->acmod_set = acmod_set = acmod_set_new();

    /* give the acmod_set module some storage allocation requirements */
    acmod_set_set_n_ci_hint(acmod_set, n_base);
    acmod_set_set_n_tri_hint(acmod_set, n_tri);

    n_total = n_base + n_tri;

    omd->defn = mdef = ckd_calloc(n_total, sizeof(model_def_entry_t));
    omd->n_total_state = n_total_map;

    all_state = ckd_calloc(n_total_map, sizeof(uint32));
    
    omd->n_tied_ci_state = n_tied_ci_state;
    omd->n_tied_state = n_tied_state;
    omd->n_tied_tmat = n_tied_tmat;

    omd->max_n_state = 0;
    omd->min_n_state = MAX_N_STATE;

    for (i = 0, j = 0, max_state = 0, max_ci_state = 0, max_tmat = 0;
	 i < n_base; i++, j += n_state) {
	n_state = MAX_N_STATE;
	if (parse_base_line(li->buf,
			    lineiter_lineno(li),
	                    &acmod_id,
			    &tmat,
			    state,
			    &n_state,
			    acmod_set) != S3_SUCCESS) {

	    fclose(fp);
            lineiter_free(li);
            return S3_ERROR;
	}

	mdef[i].p = acmod_id;
	mdef[i].tmat = tmat;
	mdef[i].n_state = n_state;
	mdef[i].state = &all_state[j];

	memcpy((char *)mdef[i].state, (const char *)state,
	       n_state * sizeof(uint32));

	update_totals(omd, &mdef[i]);

	li = lineiter_next(li);
    }

    for (; i < n_total; i++, j += n_state) {
	n_state = MAX_N_STATE;

	if (parse_tri_line(li->buf,
			   lineiter_lineno(li),
			   &acmod_id,
			   &tmat,
			   state,
			   &n_state,
			   acmod_set) != S3_SUCCESS) {
	    fclose(fp);
            lineiter_free(li);
	    return S3_ERROR;
	}

	mdef[i].p = acmod_id;
	mdef[i].tmat = tmat;
	mdef[i].n_state = n_state;
	mdef[i].state = &all_state[j];
	memcpy((char *)mdef[i].state,
	       (const char *)state,
	       n_state * sizeof(uint32));

	update_totals(omd, &mdef[i]);
	li = lineiter_next(li);
    }

    omd->n_defn = n_total;

    assert(j == n_total_map);
    
    E_INFO("Model definition info:\n");
    E_INFO("%u total models defined (%u base, %u tri)\n", omd->n_defn, n_base, n_tri);
    E_INFO("%u total states\n", omd->n_total_state);
    E_INFO("%u total tied states\n", omd->n_tied_state);
    E_INFO("%u total tied CI states\n", omd->n_tied_ci_state);
    E_INFO("%u total tied transition matrices\n", omd->n_tied_tmat);
    E_INFO("%u max state/model\n", omd->max_n_state);
    E_INFO("%u min state/model\n", omd->min_n_state);

    fclose(fp);

    lineiter_free(li);
    return S3_SUCCESS;
}

int32
model_def_free(model_def_t *mdef)
{
  uint32 i;
  uint32 len;

  if (mdef->acmod_set) {
    if (mdef->acmod_set->ci) {
      for (i = 0; i < mdef->acmod_set->n_ci; i++) {
	ckd_free(mdef->acmod_set->ci[i].name);
	if (mdef->acmod_set->ci[i].attrib) {
	  for (len = 0; mdef->acmod_set->ci[i].attrib[len]; len++)
    	     ckd_free(mdef->acmod_set->ci[i].attrib[len]);
	  ckd_free(mdef->acmod_set->ci[i].attrib);
	}
      }
      ckd_free(mdef->acmod_set->ci);
    }
    mdef->acmod_set->ci = NULL;

    if (mdef->acmod_set->multi) {
      for (i = 0; i < mdef->acmod_set->n_multi; i ++) {
	if (mdef->acmod_set->multi[i].attrib) {
	  for (len = 0; mdef->acmod_set->multi[i].attrib[len]; len++)
	     ckd_free(mdef->acmod_set->multi[i].attrib[len]);
	  ckd_free(mdef->acmod_set->multi[i].attrib);
	}
      }
      ckd_free(mdef->acmod_set->multi);
    }
    mdef->acmod_set->multi = NULL;

    if (mdef->acmod_set->multi_idx) {
      for (i = 0; i < mdef->acmod_set->n_ci; i++) {
	ckd_free(mdef->acmod_set->multi_idx[i]->cell);
	ckd_free(mdef->acmod_set->multi_idx[i]);
      }
      ckd_free(mdef->acmod_set->multi_idx);
    }
    mdef->acmod_set->multi_idx = NULL;

    if (mdef->acmod_set->attrib) {
      for (len = 0; mdef->acmod_set->attrib[len]; len++)
	  ckd_free(mdef->acmod_set->attrib[len]);
      ckd_free(mdef->acmod_set->attrib);
    }
    mdef->acmod_set->attrib = NULL;

    if (mdef->acmod_set->n_with)
      ckd_free(mdef->acmod_set->n_with);
    mdef->acmod_set->n_with = NULL;
  }
  ckd_free(mdef->acmod_set);
  mdef->acmod_set = NULL;

  if (mdef->defn) {
    ckd_free(mdef->defn->state);
    for (i = 0; i < mdef->n_defn; i++) {
      if (mdef->defn[i].attrib) {
	for (len = 0; mdef->defn[i].attrib[len]; len++)
	    ckd_free(mdef->defn[i].attrib[len]);
	ckd_free(mdef->defn[i].attrib);
      }
    }
  }
  ckd_free(mdef->defn);
  mdef->defn = NULL;

  if (mdef->cb)
    ckd_free(mdef->cb);
  mdef->cb = NULL;

  if (mdef->ts2ci)
    ckd_free(mdef->ts2ci);
  mdef->ts2ci = NULL;

  ckd_free(mdef);
  mdef = NULL;

  return S3_SUCCESS;
}

