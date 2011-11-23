/* ====================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All rights 
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
 * File: s3phseg_io.c
 * 
 * Description: 
 *     SPHINX-III phone segmentation file I/O functions
 *
 * Author: 
 *     David Huggins-Daines (dhuggins@cs.cmu.edu)
 *********************************************************************/

#include <s3/s3phseg_io.h>
#include <sphinxbase/ckd_alloc.h>
#include <s3/s3.h>
#include <stdio.h>
#include <string.h>

int
s3phseg_read(const char *fn,
	     acmod_set_t *acmod_set,
	     s3phseg_t **out_phseg)
{
	FILE *fp;
	char txt[512];
	s3phseg_t *plist = NULL;
	int n;

	if ((fp = fopen(fn, "r")) == NULL) {
		E_ERROR("Failed to open phseg file %s\n", fn);
		return S3_ERROR;
	}
	/* Should be a header of column names */
	while ((n = fscanf(fp, "%511s", txt))) {
		if (n == EOF) {
			E_ERROR("Failed to read column headers from phseg file\n");
			goto error_out;
		}
		if (!strcmp(txt, "Phone"))
			break;
	}
	/* Get each line */
	while (!feof(fp)) {
		unsigned int sf, ef;
		int score;
		acmod_id_t ci, phone;
		s3phseg_t *phseg;
		char *c, *cc;

		n = fscanf(fp, "%u %u %d", &sf, &ef, &score);
		if (n < 3) /* We probably hit EOF or "Total score" */
			break;
		fgets(txt, sizeof(txt), fp);
		/* Remove newline. */
		if (txt[strlen(txt)-1] == '\n')
			txt[strlen(txt)-1] = '\0';
		/* Find the base phone. */
		cc = txt + strspn(txt, " \t");
		if ((c = strchr(cc, ' ')))
			*c = '\0';
		if ((ci = phone = acmod_set_name2id(acmod_set, txt)) == NO_ACMOD) {
			E_ERROR("Unknown CI phone (%s) in phseg file\n", txt);
			goto error_out;
		}
		/* Restore the space and find the triphone if necessary. */
		if (c) *c = ' ';
		if (acmod_set->n_multi != 0
		    && !acmod_set_has_attrib(acmod_set, phone, "filler")) {
			if ((phone = acmod_set_name2id(acmod_set, txt)) == NO_ACMOD) {
				/* This might be too verbose. */
				E_WARN("Unknown triphone (%s) in phseg file\n", txt);
				/* Back off to CI phone. */
				phone = ci;
			}
		}
		phseg = ckd_calloc(1, sizeof(*phseg));
		phseg->next = plist;
		phseg->phone = phone;
		phseg->sf = sf;
		phseg->ef = ef;
		phseg->score = score;
		plist = phseg;
	}
	fclose(fp);
	if (out_phseg) {
		s3phseg_t *next, *last = NULL;
		/* Now reverse the list. */
		while (plist) {
			next = plist->next;
			*out_phseg = plist;
			(*out_phseg)->next = last;
			last = *out_phseg;
			plist = next;
		}
	}
	else
		s3phseg_free(plist);
	return S3_SUCCESS;
error_out:
	fclose(fp);
	return S3_ERROR;
}

int
s3phseg_write(const char *fn,
	      acmod_set_t *acmod_set,
	      s3phseg_t *phseg)
{
	FILE *fp;

	if ((fp = fopen(fn, "w")) == NULL)
		return S3_ERROR;

	fprintf (fp, "\t%5s %5s %9s %s\n",
		 "SFrm", "EFrm", "SegAScr", "Phone");
	for (; phseg; phseg = phseg->next) {
		fprintf(fp, "\t%5d %5d %9d %s\n",
			phseg->sf, phseg->ef, phseg->score,
			acmod_set_id2name(acmod_set, phseg->phone));
	}
	fclose (fp);

	return S3_SUCCESS;
}

void
s3phseg_free(s3phseg_t *phseg)
{
	s3phseg_t *next;

	while (phseg) {
		next = phseg->next;
		ckd_free(phseg);
		phseg = next;
	}
}

int
s3lattice_read(const char *fn,
	       s3lattice_t **lattice)
{
  FILE *fp;
  uint32 id;
  char line[1024], temp[16];
  s3lattice_t *out_lattice;
  uint32 i, j, n;
  
  if ((fp = fopen(fn, "r")) == NULL) {
    E_ERROR("Failed to open lattice file %s\n", fn);
    return S3_ERROR;
  }
  
  out_lattice = ckd_calloc(1, sizeof(*out_lattice));
  
  /* process file head */
  /* read the number of total arcs */
  fgets(line, sizeof(line), fp);
  if (strstr(line, "Total arcs") == NULL) {
    E_ERROR("Lattice Format Error, missing Total arcs\n");
    goto error_out;
  }
  fgets(line, sizeof(line), fp);
  n = sscanf(line, "%d", &out_lattice->n_arcs);
  if (n!=1) {
    E_ERROR("Lattice Format Error, missing Total arcs\n");
    goto error_out;
  }
  if (out_lattice->n_arcs == 0) {
    E_ERROR("No arc exits in the lattice\n");
    goto error_out;
  }

  /* read the number of true arcs */
  fgets(line, sizeof(line), fp);
  if (strstr(line, "True arcs") == NULL) {
    E_ERROR("Lattice Format Error, missing True arcs\n");
    goto error_out;
  }
  fgets(line, sizeof(line), fp);
  n = sscanf(line, "%d", &out_lattice->n_true_arcs);
  if (n!=1) {
    E_ERROR("Lattice Format Error, missing True arcs\n");
    goto error_out;
  }
  if (out_lattice->n_true_arcs == 0) {
    E_ERROR("No arc from the numerator lattice\n");
    goto error_out;
  }
  if (out_lattice->n_true_arcs > out_lattice->n_arcs) {
    E_ERROR("The number of arcs from numerator lattice is larger than the number of total arcs\n");
    goto error_out;
  }

  /* read parameter lists */
  fgets(line, sizeof(line), fp);
  if (strstr(line, "arc_id") == NULL) {
    E_ERROR("Lattice Format Error\n");
    goto error_out;
  }
  
  /* allocate memory for arcs */
  out_lattice->arc = ckd_calloc(out_lattice->n_arcs, sizeof(*out_lattice->arc));
  
  i = 0;
  /* Get each arc */
  while (fscanf(fp, "%d", &id) != EOF) {/* arc id */
    fscanf(fp, "%s", out_lattice->arc[i].word);/* word */
    fscanf(fp, "%d", &out_lattice->arc[i].sf);/* start frame */
    fscanf(fp, "%d", &out_lattice->arc[i].ef);/* end frame */
    fscanf(fp, "%lf", &out_lattice->arc[i].lm_score);/* LM score */
    fscanf(fp, "%d", &out_lattice->arc[i].n_prev_arcs);/* num of previous arcs */
    fscanf(fp, "%d", &out_lattice->arc[i].n_next_arcs);/* num of succeeding arcs */
    
    /* read preceding arc ids */
    fscanf(fp, "%s", temp);/* move over '<' */
    if (out_lattice->arc[i].n_prev_arcs == 0) {
      E_ERROR("No preceding arc exits\n");
      goto error_out;
    }
    out_lattice->arc[i].prev_arcs = ckd_calloc(out_lattice->arc[i].n_prev_arcs, sizeof(int));
    for (j=0; j<out_lattice->arc[i].n_prev_arcs; j++)
      fscanf(fp, "%d", &out_lattice->arc[i].prev_arcs[j]);
    
    /* read succeeding arc ids */
    fscanf(fp, "%s", temp);/* move over '<' */
    if (out_lattice->arc[i].n_next_arcs == 0) {
      E_ERROR("No succeeding arc exits\n");
      goto error_out;
    }
    out_lattice->arc[i].next_arcs = ckd_calloc(out_lattice->arc[i].n_next_arcs, sizeof(int));
    for (j=0; j<out_lattice->arc[i].n_next_arcs; j++)
      fscanf(fp, "%d", &out_lattice->arc[i].next_arcs[j]);
    
    i++;
  }
  fclose(fp);
  
  *lattice = out_lattice;
  
  return S3_SUCCESS;
 error_out:
  fclose(fp);
  return S3_ERROR;
}
