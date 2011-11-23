/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * 2004-07-26 : ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First adapted from SamJoo's package. 
 */

#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3gau_io.h>
#include <s3/s3acc_io.h>
#include <s3/gauden.h>
#include <sphinxbase/matrix.h>

#include <s3/mllr.h>
#include <s3/mllr_io.h>
#include <s3/s3cb2mllr_io.h>

#include <sys_compat/file.h>
#include <sys_compat/misc.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define ABS(x)  	((x) < 0 ? (-(x)) : (x))
#define VAR_CONST	0.5


static int
initialize(int argc,
	   char *argv[])
{
    /* define, parse and (partially) validate the command line */
    parse_cmd_ln(argc, argv);

    return S3_SUCCESS;
}



/********************************************************************
 *	Make MLLR matrix A and B using conventional MLLR method
 ********************************************************************/
int 
mllr_mat(float32 	*****out_A, 
      	float32 	****out_B, 
  	const char 	*var_fn, 
  	vector_t 	***mean, 
        vector_t        ***wt_mean,             /* read from bw accum */
	float32         ***wt_dcount,           /* read from bw accum */
  	uint32 		gau_begin, 
  	int32 		*cb2mllr, 
  	uint32 		mllr_mult,		
  	uint32 		mllr_add,		
  	float32		varfloor, 
  	uint32 		n_mgau, 
  	uint32 		n_stream, 
  	uint32 		n_density, 
  	uint32 		n_mllr_class, 
  	const uint32 	*veclen) 


{
      vector_t 	***var  	= NULL; 
      float32 	****A  		= NULL;		
      float32 	***B   		= NULL; 	

      float32 	*****regl  	= NULL; 
      float32 	****regr   	= NULL; 

      float32     wt_mean_var;
      float32     wt_dcount_var;
      float32     wt_dcount_var_mean;

      float32	*tmean		= NULL; 
      uint32 	n_mgau_rd; 
      uint32 	n_stream_rd; 
      uint32 	n_density_rd; 

      uint32    *veclen_rd	= NULL; 

      uint32 i, j, k, l, s, p , q;
      int32 m, mc;
      int32 len=0;

      /*      uint32 	i, j, k, l, p, q, s; 
	      int32	m, mc, mi; */

      E_INFO("\n");
      E_INFO(" ---- mllr_solve(): Conventional MLLR method\n"); 

      if (cmd_ln_int32("-fullvar")) {
	  /* Extract diagonals to solve MLLR (we are not doing
	     variance adaptation, yet) */
	  vector_t ****fullvar;
	  uint32 i, j, k, l;

	  if (s3gau_read_full(var_fn, 
			      &fullvar, 
			      &n_mgau_rd, 
			      &n_stream_rd, 
			      &n_density_rd, 
			      &veclen_rd) != S3_SUCCESS) { 
	      E_FATAL("Couldn't read %s", var_fn); 
	  } 
	  var = gauden_alloc_param(n_mgau_rd,
				   n_stream_rd,
				   n_density_rd,
				   veclen_rd);
	  for (i = 0; i < n_mgau_rd; ++i)
	      for (j = 0; j < n_stream_rd; ++j)
		  for (k = 0; k < n_density_rd; ++k)
		      for (l = 0; l < veclen_rd[j]; ++l)
			  var[i][j][k][l] =
			      fullvar[i][j][k][l][l];
	  gauden_free_param_full(fullvar);
      }
      else {
	  if (s3gau_read(var_fn, 
			 &var, 
			 &n_mgau_rd, 
			 &n_stream_rd, 
			 &n_density_rd, 
			 &veclen_rd) != S3_SUCCESS) { 
	      E_FATAL("Couldn't read %s", var_fn); 
	  } 
      }

      if (n_mgau != n_mgau_rd) { 
  	E_FATAL("n_mgau mismatch (%u : %u)\n",n_mgau,n_mgau_rd); 
      } 
      if (n_stream != n_stream_rd) { 
  	E_FATAL("n_stream mismatch (%u : %u)\n",n_stream,n_stream_rd); 
      } 
      if (n_density != n_density_rd) { 
  	E_FATAL("n_density mismatch (%u : %u)\n",n_density,n_density_rd); 
      } 
      for (s = 0; s < n_stream; s++) { 
  	if (veclen[s] != veclen_rd[s]) { 
  	    E_FATAL("vector length of stream %u (== %u) " 
  		    "!= prior length (== %u)\n", s, veclen_rd[s], veclen[s]); 
  	} 
      } 
      ckd_free((void *)veclen_rd); 
      veclen_rd = NULL; 


      /* Invert variances. */
      for (i = 0; i < n_mgau; i++) { 
          for (j = 0; j < n_stream; j++) { 
              for (k = 0; k < n_density; k++) { 
                  for (l = 0; l < veclen[j]; l++) { 
                      if (var[i][j][k][l] <= 0.) { 
  			var[i][j][k][l] = VAR_CONST;	
  		    } 
  		    else if (var[i][j][k][l] < varfloor) { 
  			var[i][j][k][l] = 1. / varfloor; 
  		    } 
  		    else { 
  		        var[i][j][k][l] = 1. / var[i][j][k][l]; 
  		    } 
                  } 
              } 
          } 
      } 


      fprintf(stderr,"\n"); 

      E_INFO(" ---- A. Accum regl, regr\n"); 
      E_INFO(" No classes %d, no. stream %d\n",n_mllr_class,n_stream);

      /* Legetter's set of G matrices, one per dimension per class. */
      regl = (float32 *****)ckd_calloc_2d(n_mllr_class, n_stream, sizeof(float32 ***)); 
      /* Legetter's Z matrices, one per class. */
      regr = (float32 ****) ckd_calloc_2d(n_mllr_class, n_stream, sizeof(float32 **)); 
      
      for (i = 0; i < n_mllr_class; i++) { 
          for (j = 0; j < n_stream; j++) { 
              len = veclen[j]; 
              regl[i][j] = (float32 ***)ckd_calloc_3d(len, len+1, len+1, sizeof(float32)); 
              regr[i][j] = (float32 **) ckd_calloc_2d(len, len+1, sizeof(float32)); 
          } 
      } 
      
      /*      E_INFO(" Checking\n");*/

      for (i = gau_begin; i < n_mgau; i++) { 
        mc = cb2mllr[i]; 
	if (mc < 0) continue;	/* skip */

        for (j = 0; j < n_stream; j++) { 
	  len=veclen[j];

	  for (k = 0; k < n_density; k++) { 
	    if (wt_dcount[i][j][k] > 0.) { 
	      tmean = mean[i][j][k]; 
	      for (l = 0; l < len; l++) { 
		wt_mean_var   = wt_mean[i][j][k][l] * var[i][j][k][l]; 
		wt_dcount_var = wt_dcount[i][j][k]  * var[i][j][k][l]; 
		
		for (p = 0; p < len; p++) { 
		  wt_dcount_var_mean = wt_dcount_var * tmean[p]; 
		  for (q = p; q < len; q++) { 
		    /* This is g(l)_{pq} = sum_r(v_{ii}(r) d_{pq}(r))*/
		    /* or in other words sum(likelihood * invvar * outer(mean, mean)) */
		    regl[mc][j][l][p][q] += (float32) (wt_dcount_var_mean * tmean[q]); 
		  } 
		  /* G corresponding to extended element of q. */
		  regl[mc][j][l][p][len] += (float32)(wt_dcount_var_mean); 
		  /* This is z_{lp} = sum(likelihood * invvar * obs * mean_p) */
		  regr[mc][j][l][p] += (float32)(wt_mean_var * tmean[p]); 
		} 
		/* G corresponding to extended element of p and q. */
		/* Question: what about regl[mc][j][l][len][0..len]? */
		/* Answer: G(l) is symmetric, so we already calculated them. */
		regl[mc][j][l][len][len] += (float32)wt_dcount_var; 
		/* Z corresponding to extended element of p. */
		regr[mc][j][l][len] += (float32)(wt_mean_var); 
	      } 
	    } 
	  } 
	} 
      }

      gauden_free_param(mean); 
      gauden_free_param(wt_mean); 
      ckd_free_3d((void ***)wt_dcount); 

      /* Fill in the lower triangular part of regl. */
      for (m = 0; m < n_mllr_class; m++) { 
        for (j = 0; j < n_stream; j++) { 
	  for (l = 0; l < veclen[j]; l++) { 
	    for (p = 0; p <= veclen[j]; p++) { 
	      for (q = p+1; q <= veclen[j]; q++) { 
		regl[m][j][l][q][p] = regl[m][j][l][p][q]; 
	      } 
	    } 
	  } 
        } 
      } 

      E_INFO(" ---- B. Compute MLLR matrices (A,B)\n"); 
      if(compute_mllr(regl, 
  		    regr, 
  		    veclen, 
  		    n_mllr_class, 
  		    n_stream, 
  		    mllr_mult, 
  		    mllr_add, 
  		    &A, 
  		    &B) != S3_SUCCESS) {  
  	E_FATAL("MLLR computation failed\n"); 
      } 

      free_mllr_reg(regl,regr, n_mllr_class, n_stream); 

      *out_A = A; 
      *out_B = B; 

      return S3_SUCCESS; 
}

int main(int argc, char *argv[])
{
    const char  *out_mllr_fn;
    const char  **accum_dir;
    const char  *mean_fn;
    const char  *var_fn;
    const char  *cb2mllrfn;
    const char  *moddeffn;

    uint32      cdonly;
    uint32      mllr_mult;		/* option 0 or 1 */
    uint32      mllr_add;		/* option 0 or 1 */

    float32     varfloor;
    vector_t    ***mean         = NULL;         /* baseline mean */
    vector_t    ***wt_mean      = NULL;		/* read from bw accum */
    float32     ***wt_dcount    = NULL;		/* read from bw accum */
    vector_t    ***wt_var       = NULL;         /* not used */
    int32       pass2var;                       /* not used */

    uint32      n_mgau;
    uint32      n_stream;
    uint32      n_density;
    uint32      n_mllr_class;
    uint32      n_mgau_rd;
    uint32      n_stream_rd;
    uint32      n_density_rd;
    uint32 *veclen	= NULL;
    uint32 *veclen_rd	= NULL;

    float32     ****A  		= NULL;          /* Output mllr: A */
    float32     ***B   		= NULL;          /* Output mllr: B */

    int32       *cb2mllr   	= NULL;


    /*    int32       **mllr2cb  	= NULL;
	  int32       *n_mllr2cb 	= NULL;*/

    model_def_t *mdef;
    uint32	gau_begin;
    uint32 i,s ;

    if (initialize(argc, argv) != S3_SUCCESS) {
	E_FATAL("errors initializing.\n");
    }

    out_mllr_fn         = cmd_ln_str("-outmllrfn");
    accum_dir           = cmd_ln_str_list("-accumdir");
    mean_fn             = cmd_ln_str("-meanfn");
    var_fn              = cmd_ln_str("-varfn");

    cb2mllrfn           = cmd_ln_str("-cb2mllrfn");
    cdonly              = cmd_ln_int32("-cdonly");
    moddeffn            = cmd_ln_str("-moddeffn");

    mllr_mult      = cmd_ln_int32("-mllrmult");
    mllr_add            = cmd_ln_int32("-mllradd");
    varfloor            = cmd_ln_float32("-varfloor");


    assert(accum_dir[0] != NULL);	/* must be at least one accum dir */

    if (! (out_mllr_fn && accum_dir && mean_fn)) {
        E_FATAL("Some of options are missing.\n");
    }
    if (varfloor < 0.) {
        E_FATAL("varfloor is negative (%e)\n",varfloor);
    }


    if (cb2mllrfn && strcmp(cb2mllrfn,"NO") == 0) {
        cb2mllrfn = NULL;
    }
    if (moddeffn && strcmp(moddeffn,"NO") == 0) {
        moddeffn = NULL;
    }

    /*--------------------------------------------------------------------*/
    fprintf(stderr,"\n");
    E_INFO("-- 1. Read input mean, (var) and accumulation.\n");
    /*--------------------------------------------------------------------*/

    /*-------------- Read baseline mean --------------*/
    if (s3gau_read(mean_fn,
                   &mean,
                   &n_mgau,
                   &n_stream,
                   &n_density,
                   &veclen) != S3_SUCCESS) {
        E_FATAL("Couldn't read %s", mean_fn);
    }

    /*-------- Read accum_dir (accumulation from bw) --------*/
    for (i = 0; accum_dir[i]; i++) {
        E_INFO("Reading and accumulating counts from %s\n", accum_dir[i]);
        if (rdacc_den(accum_dir[i],
                      &wt_mean,
                      &wt_var,          
                      &pass2var,        
                      &wt_dcount,
                      &n_mgau_rd,
                      &n_stream_rd,
                      &n_density_rd,
                      &veclen_rd) != S3_SUCCESS) {
            E_FATAL("Error in reading %s\n", accum_dir[i]);
        }
    }

    if (n_mgau != n_mgau_rd) {
        E_FATAL("n_mgau mismatch (%u : %u)\n",n_mgau,n_mgau_rd);
    }
    if (n_stream != n_stream_rd) {
        E_FATAL("n_stream mismatch (%u : %u)\n",n_stream,n_stream_rd);
    }
    if (n_density != n_density_rd) {
        E_FATAL("n_density mismatch (%u : %u)\n",n_density,n_density_rd);
    }
    for (s = 0; s < n_stream; s++) {
        if (veclen[s] != veclen_rd[s]) {
            E_FATAL("vector length of stream %u (== %u) "
                    "!= prior length (== %u)\n",
                    s, veclen_rd[s], veclen[s]);
        }
    }
    ckd_free((void *)veclen_rd);
    veclen_rd = NULL;
    if (wt_var) {               /* We don't use 'wt_var' in this program. */
        gauden_free_param(wt_var);
        wt_var = NULL;
    }

    /*--------------------------------------------------------------------*/
    fprintf(stderr,"\n");
    E_INFO("-- 2. Read cb2mllrfn\n");
    /*--------------------------------------------------------------------*/

    if (strcmp(cb2mllrfn, ".1cls.") == 0) {
        n_mllr_class = 1;
        cb2mllr = (int32 *) ckd_calloc(n_mgau, sizeof(int32));
    }
    else {
        if (s3cb2mllr_read(cb2mllrfn,
                           &cb2mllr,
                           &n_mgau_rd,
                           &n_mllr_class) != S3_SUCCESS) {
            E_FATAL("Unable to read %s\n",cb2mllrfn);
        }
        if (n_mgau_rd != n_mgau) {
            E_FATAL("cb2mllr maps %u cb, but read %u cb from files\n",
                    n_mgau_rd, n_mgau);
        }
    }
    E_INFO("n_mllr_class = %d\n", n_mllr_class);

    gau_begin = 0;
    if (cdonly) {

        if (! moddeffn) {
            E_FATAL("-moddeffn is not given.\n");
        }
        else if (model_def_read(&mdef, moddeffn) != S3_SUCCESS) {
            E_FATAL("Can not read model definition file %s\n", moddeffn);
        }
        gau_begin = mdef->n_tied_ci_state;
        for (i=0; i<gau_begin; i++) {
            cb2mllr[i] = -1;                    /* skip CI senones */
        }
        E_INFO("Use CD senones only. (index >= %d)\n",mdef->n_tied_ci_state);
    }

    /*--------------------------------------------------------------------*/
    fprintf(stderr,"\n");
    E_INFO("-- 3. Calculate mllr matrices\n");
    /*--------------------------------------------------------------------*/

    mllr_mat(&A,
	     &B,
	     var_fn,
	     mean,
	     wt_mean,
	     wt_dcount,
	     gau_begin,
	     cb2mllr,
	     mllr_mult,
	     mllr_add,
	     varfloor,
	     n_mgau,
	     n_stream,
	     n_density,
	     n_mllr_class,
	     veclen);

    /*--------------------------------------------------------------------*/
    fprintf(stderr,"\n");
    E_INFO("-- 4. Store mllr matrices (A,B) to %s\n", out_mllr_fn);
fflush(stderr);
    /*--------------------------------------------------------------------*/

    if(store_reg_mat(out_mllr_fn,
                     veclen,
                     n_mllr_class,
                     n_stream,
                     A,
                     B) != S3_SUCCESS) {
        E_FATAL("Unable to write %s\n", out_mllr_fn);
    }

    ckd_free((void *)veclen);
    free_mllr_A(A, n_mllr_class, n_stream);
    free_mllr_B(B, n_mllr_class, n_stream);
    return 0 ;
}
