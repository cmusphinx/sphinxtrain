/* ====================================================================
 * Copyright (c) 2005 Carnegie Mellon University.  All rights 
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
 * Description: 
 * 	Main program for kdtree(1)
 *
 * Author: 
 * 	David Huggins-Daines <dhuggins@cs.cmu.edu>
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/common.h>
#include <s3/s3gau_io.h>
#include <s3/kdtree.h>

#include <stdio.h>
#include <math.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	const char *meanfn, *varfn;
	vector_t ***means, ***variances;
	uint32 n_mgau, n_feat, n_density;
	uint32 r_n_mgau, r_n_feat, r_n_density;
	uint32 *veclen, *r_veclen;
	uint32 i;
	kd_tree_node_t **root;

	parse_cmd_ln(argc, argv);

	meanfn = cmd_ln_str("-meanfn");
	varfn = cmd_ln_str("-varfn");
	if (meanfn == NULL || varfn == NULL)
		E_FATAL("You must specify -meanfn and -varfn\n");

	if (s3gau_read(meanfn, &means, &n_mgau,
		       &n_feat, &n_density, &veclen) != S3_SUCCESS)
		E_FATAL("Failed to read means from %s\n", meanfn);
	if (s3gau_read(varfn, &variances, &r_n_mgau,
		       &r_n_feat, &r_n_density, &r_veclen) != S3_SUCCESS)
		E_FATAL("Failed to read variances from %s\n", varfn);
	if (n_mgau != r_n_mgau)
		E_FATAL("Number of GMMs in variances doesn't match means: %d != %d\n",
			r_n_mgau, n_mgau);
	if (n_mgau != 1)
		E_FATAL("Only semi-continuous models are currently supported\n");
	if (n_density != r_n_density)
		E_FATAL("Number of Gaussians in variances doesn't match means: %d != %d\n",
			r_n_density, n_density);
	if (n_feat != r_n_feat)
		E_FATAL("Number of feature streams in variances doesn't match means: %d != %d\n",
			r_n_feat, n_feat);
	for (i = 0; i < n_feat; ++i)
		if (veclen[i] != r_veclen[i])
			E_FATAL("Size of feature stream %d in variances doesn't match means: %d != %d\n",
				i, r_veclen[i], veclen[i]);
	ckd_free(r_veclen);

	/* Build one kd-tree for each feature stream. */
	root = ckd_calloc(n_feat, sizeof(*root));
	for (i = 0; i < n_feat; ++i) {
		root[i] = build_kd_tree(means[0][i], variances[0][i],
					n_density, veclen[i],
					cmd_ln_float32("-threshold"),
					cmd_ln_int32("-depth"),
					cmd_ln_int32("-absolute"));
	}

	if (cmd_ln_str("-outfn"))
		write_kd_trees(cmd_ln_str("-outfn"),
			       root, n_feat);

	for (i = 0; i < n_feat; ++i)
		free_kd_tree(root[i]);
	ckd_free(root);
	ckd_free(veclen);
	ckd_free_4d(means);
	ckd_free_4d(variances);
	cmd_ln_free();

	return 0;
}
