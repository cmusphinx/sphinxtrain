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
 *
 * File: quest.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/


#include <s3/quest.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>

#include <string.h>
#include <assert.h>
#include <ctype.h>

char *
s3parse_quest(pset_t *pset, uint32 n_pset, quest_t *q, char *in_str)
{
    char *s, *sp;
    uint32 i;

    s = in_str;
    
    /* skip leading whitespace */
    for (; *s != '\0' && isspace((unsigned char)*s); s++);

    if (*s == '\0')	/* Nothing to parse */
	return s;

    if (*s == '!') {
	q->neg = TRUE;
	++s;
	if (*s == '\0') {
	    E_ERROR("question syntax error");

	    return NULL;
	}
    }
    else
	q->neg = FALSE;
    
    sp = strchr(s, ' ');
    if (sp == NULL) {
	E_ERROR("Expected space after question name\n");
	return NULL;
    }

    *sp = '\0';

    for (i = 0; i < n_pset; i++) {
	if (strcmp(s, pset[i].name) == 0) {
	    q->pset = i;
	    q->member = pset[i].member;
	    q->posn   = pset[i].posn;

	    break;
	}
    }
    if (i == n_pset) {
	E_ERROR("Unknown question %s\n", s);

	return NULL;
    }

    s = sp+1;

    *sp = ' ';	/* undo set to null */

    /* skip whitespace */
    for (; *s != '\0' && isspace((unsigned char)*s); s++);

    if (s[0] == '-') {
	if (s[1] == '1') {
	    q->ctxt = -1;
	}
	s += 2;
    }
    else if (s[0] == '0') {
	q->ctxt = 0;
	s++;
    }
    else if (s[0] == '1') {
	q->ctxt = 1;
	s++;
    }

    /* skip trailing whitespace, if any */
    for (; *s != '\0' && isspace((unsigned char)*s); s++);

    return s;
}

static uint32
count_quest_in_conj(pset_t *pset,
		    uint32 n_pset,
		    char *in_str)
{
    quest_t tmp;
    quest_t *q = &tmp;
    char *t;
    uint32 n_quest;

    n_quest = 0;

    t = in_str;

    for (; *t != '\0' && isspace((unsigned char)*t); t++);
    if (*t == ')') {
	E_ERROR("Empty conjunction\n");
	
	return 0;
    }
    while (t && *t != ')' && *t != '\0') {
	t = s3parse_quest(pset, n_pset, q, t);
	++n_quest;

	for (; t && *t != '\0' && isspace((unsigned char)*t); t++);
    }
    if (t == NULL) {
	E_ERROR("Error while parsing conjunction: %s\n", in_str);

	return 0;
    }
    if (*t != ')') {
	E_ERROR("Error while parsing conjunction: %s\n", in_str);
	
	return 0;
    }

    return n_quest;
}

char *
s3parse_conj(pset_t *pset,
	     uint32 n_pset,
	     quest_t **term,
	     uint32 *n_simple_q,
	     char *in_str)
{
    quest_t *termlst;
    char *s;
    uint32 n_quest;
    uint32 i;

    s = in_str;

    if (*s == '\0') return s;

    /* skip leading whitespace */
    for (; *s != '\0' && isspace((unsigned char)*s); s++);
    
    if (*s == '\0') return s;

    if (*s == '(') {
	++s;
    }
    else {
	E_ERROR("Expected '(' before conjunction\n");

	return NULL;
    }

    for (; *s != '\0' && isspace((unsigned char)*s); s++);

    if (*s == '\0') {
	E_ERROR("No terms and close paren in conjunction\n", in_str);
	
	return NULL;
    }
    
    n_quest = count_quest_in_conj(pset, n_pset, s);
    *n_simple_q = n_quest;

    termlst = (quest_t *)ckd_calloc(n_quest, sizeof(quest_t));
    *term = termlst;

    for (i = 0; i < n_quest; i++) {
	s = s3parse_quest(pset, n_pset, &termlst[i], s);
	for (; *s != '\0' && isspace((unsigned char)*s); s++);
    }

    assert(*s == ')');

    s++;

    return s;
}

static uint32
s3cnt_q_term(char *in_str)
{
    char *s;
    uint32 n_term;

    s = in_str;

    /* skip any leading whitespace */
    for (; *s != '\0' && isspace((unsigned char)*s); s++);
    
    /* assume everything is well-formed for the moment.
     * later processing will catch syntax errors
     * which should be unlikely anyway since this stuff
     * is most likely machine generated */

    for (s++, n_term = 0; *s && (s = strchr(s, '(')); n_term++, s++);

    return n_term;
}

int
s3parse_comp_quest(pset_t *pset,
		   uint32 n_pset,
		   comp_quest_t *q,
		   char *in_str)
{
    char *s;
    uint32 i;

    s = in_str;

    for (; *s != '\0' && isspace((unsigned char)*s); s++);

    if (*s == '\0') {
	E_ERROR("Empty string seen for composite question\n");

	return S3_ERROR;
    }

    if (*s != '(') {
	E_ERROR("Composite question does not begin with '(' : %s\n",
		in_str);
	
	return S3_ERROR;
    }
	
    q->sum_len  = s3cnt_q_term(in_str);
    q->conj_q   = (quest_t **)ckd_calloc(q->sum_len, sizeof(quest_t *));
    q->prod_len = (uint32 *)ckd_calloc(q->sum_len, sizeof(uint32));

    ++s;	/* skip the open paren */

    i = 0;
    do {
	s = s3parse_conj(pset,
			 n_pset,
			 &q->conj_q[i],
			 &q->prod_len[i],
			 s);
	++i;
    } while (s && *s && *s == '(');

    if (s == NULL) {
	E_ERROR("Error while parsing %s\n", in_str);

	return S3_ERROR;
    }

    return S3_SUCCESS;
}

static void
parse_simple_q(quest_t *q,
	       char *q_str)
{
    int i;
    int len;
    uint32 pset;

    assert(q != NULL);
    assert(q_str != NULL);

    len = strlen(q_str);

    /* skip leading whitespace */
    for (i = 0; i < len && isspace((unsigned char)q_str[i]); i++);

    if (i == len)
	return;

    if (q_str[i] == '~') {
	q->neg = TRUE;
	i++;
    }
    else {
	q->neg = FALSE;
    }

    pset = atoi(&q_str[i]);

    if (pset >= 400) {
	q->ctxt = 1;
	pset -= 400;
    }
    else if (pset < 400) {
	q->ctxt = -1;
    }

    q->pset = pset;

    /* HACK to get around WDBNDRY question context */
    if (pset < 3)
	q->ctxt = 0;
}

char *
parse_conj(quest_t **term,
	   uint32 *n_simple_q,
	   char *q_str)
{
    char *t, *eot;
    int n_q;
    char t_str[64];
    char *simp_q_str;
    quest_t *out;
    int i;
    
    /* copy the next product into t_str */
    eot = strchr(q_str, '|');

    if (eot) {
	strncpy(t_str, q_str, (eot - q_str));
	t_str[(eot - q_str)] = '\0';
    }
    else {
	strcpy(t_str, q_str);
    }

    /* count the # of terms in the product */
    t = t_str-1;
    n_q = 1;
    do {
	t = strchr(t+1, '&');
	if (t) {
	    n_q++;
	}
    } while (t);

    /* allocate a simple question for each term in product */
    out = ckd_calloc(n_q, sizeof(quest_t));

    *term = out;
    *n_simple_q = n_q;
    
    /* parse each simple question */
    simp_q_str = strtok(t_str, "&");
    i = 0;
    do {
	parse_simple_q(&out[i], simp_q_str);
	simp_q_str = strtok(NULL, "&");
	i++;
    } while (simp_q_str);

    return eot;
}

uint32
cnt_q_term(char *q_str)
{
    char *t;
    uint32 n_term;

    t = q_str-1;
    n_term = 1;

    do {
	t = strchr(t+1, '|');
	if (t) ++n_term;
    } while (t);

    return n_term;
}

void
parse_compound_q(comp_quest_t *q,
		 char *q_str)
{
    char *rem_q_str;
    uint32 i;

    q->sum_len = cnt_q_term(q_str);
    q->conj_q = ckd_calloc(q->sum_len, sizeof(quest_t *));
    q->prod_len = ckd_calloc(q->sum_len, sizeof(uint32));

    i = 0;
    rem_q_str = q_str-1;
    do {
	rem_q_str = parse_conj(&q->conj_q[i],
			       &q->prod_len[i],
			       rem_q_str+1);
	++i;
    } while (rem_q_str);
}

void
print_quest(FILE *fp,
	    pset_t *pset,
	    quest_t *q)
{
    if (pset == NULL) {
	fprintf(fp, "%s%d %d",
		(q->neg ? "!" : ""),
		q->pset,
		q->ctxt);
    }
    else {
	fprintf(fp, "%s%s %d",
		(q->neg ? "!" : ""),
		pset[q->pset].name,
		q->ctxt);
    }
}

int
eval_quest(quest_t *q,
	   uint32 *feat,
	   uint32 n_feat)
{
    uint32 ctxt;
    int ret = FALSE;

    ctxt = q->ctxt + 1;

    if (q->member)
	ret = q->member[feat[ctxt]];
    else if (q->posn)
	ret = q->posn[feat[n_feat-1]];
    else {
	E_FATAL("Ill-formed question\n");
    }
    
    if (q->neg) ret = !ret;

#if 0
    E_INFO("eval: (%s%u %d) %u -> %u\n",
	   (q->neg ? "!" : ""),
	   q->pset,
	   q->ctxt,
	   (q->member ? q->member[feat[ctxt]] :
	    q->posn[feat[n_feat-1]]),
	   ret);
#endif
    
    return ret;
}

int
eval_comp_quest(comp_quest_t *q,
		uint32 *feat,
		uint32 n_feat)
{
    int i, j;

    for (i = 0; i < q->sum_len; i++) {
	for (j = 0; j < q->prod_len[i]; j++) {
	    if (!eval_quest(&q->conj_q[i][j], feat, n_feat))
		break;
	}

	/* One of the terms in the disjunction
	* is satisfied; so the whole is satisfied */
	if (j == q->prod_len[i])
	    return TRUE;
    }

    /* visited all terms in the disjunction and none
     * were satisified; so neither is the disjunction */
    return FALSE;
}

void
print_comp_quest(FILE *fp,
		 pset_t *pset,
		 comp_quest_t *q)
{
    int i, j;

    fprintf(fp, "(");
    for (i = 0; i < q->sum_len; i++) {
	fprintf(fp, "(");
	print_quest(fp, pset, &q->conj_q[i][0]);
	
	for (j = 1; j < q->prod_len[i]; j++) {
	    fprintf(fp, " ");
	    print_quest(fp, pset, &q->conj_q[i][j]);
	}
	fprintf(fp, ")");
    }
    fprintf(fp, ")");
}

int
is_subset(quest_t *a,
	  quest_t *b,
	  uint32 n_phone)
{
    uint32 p;
    int f_a, f_b;

    if (a->member && b->member) {
	if (a->ctxt != b->ctxt)
	    return FALSE;

	for (p = 0; p < n_phone; p++) {
	    if (a->neg)
		f_a = !a->member[p];
	    else
		f_a = a->member[p];
	    
	    if (b->neg)
		f_b = !b->member[p];
	    else
		f_b = b->member[p];
	    
	    if (f_a && (f_a != f_b)) {
		break;
	    }
	}
	if (p != n_phone)
	    return FALSE;
	else
	    return TRUE;
    }
    else if ((a->member && b->posn) ||
	     (a->posn && b->member)) {
	/* one question about word boundary
	 * and the other is about phone context
	 * so not a subset */
	return FALSE;
    }
    else if (a->posn && b->posn) {
	/* Not handled at the moment */
	return FALSE;
    }
    return FALSE;
}


int
simplify_conj(quest_t *conj,
	      uint32 n_term,
	      uint32 n_phone)
{
    uint32 i, j;
    int *del, exist_del = FALSE;

    assert(n_term != 0);

    if (n_term == 1)	/* Only one term; nothing to do */
	return 1;

    del = ckd_calloc(n_term, sizeof(int));

    /* Search for all pairs (i,j) where
     * term_i is a subset of term_j.  Mark
     * all such term_j's for deletion since
     * term_i && term_j == term_i */
    for (i = 0; i < n_term; i++) {
	for (j = 0; j < n_term; j++) {
	    if ((i != j) && (!del[i] || !del[j])) {
		if (is_subset(&conj[i], &conj[j], n_phone)) {
		    /* mark the superset for deletion */
		    del[j] = TRUE;
		    exist_del = TRUE;
		}
	    }
	}
    }

    /* compact the conjunction by removing
     * term_j's that are marked for deletion.
     */
    for (i = 0, j = 0; j < n_term; i++, j++) {
	if (del[j]) {
	    /* move j to the next
	     * non-deleted term (if any) */
	    for (j++; del[j] && (j < n_term); j++);
	    
	    if (j == n_term)
		break;
	}
	if (i != j) {
	    conj[i] = conj[j];
	}
    }

    ckd_free(del);

    return i;	/* return new n_term */
}

int
simplify_comp_quest(comp_quest_t *q,
		    uint32 n_phone)
{
    int i;
    int ret = FALSE;
    int prod_len;

    for (i = 0; i < q->sum_len; i++) {
	prod_len = simplify_conj(q->conj_q[i], q->prod_len[i], n_phone);
	if (prod_len < q->prod_len[i]) {
	    assert(!(prod_len > q->prod_len[i]));
	    
	    q->prod_len[i] = prod_len;

	    ret = TRUE;
	}
    }

    /* TRUE if there is at least one term in the composite
     * question that was simplified */
    return ret;
}
