/* ====================================================================
 * Copyright (c) 1994-2005 Carnegie Mellon University.  All rights 
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
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    Program to compute a Multimodal gaussian distribution for a given
    data set. Estimates DIAGNOAL covariances ONLY (i.e all modes are assumed
    to have diagonal covariance matrices).
    Does NOT compute optimal number of modes; has to be given the number of
    modes required.
    The routine runs a variable number of iterations of EM each time it 
    is called. The number of iterations is passed as a variable

    The parameters passed are 
	x[][] - the data set (float)
        N - the number of vectors in the data set (int)
	Ndim - the dimensionality of the dataset (int)
	K - the number of modes required in the computed distribution (int)
	mean[][] - Some initial values for the means of the gaussians(float)
	var[][] - Initial values for the Variances of the modes(float)
	c[] - Initial values for the a-priori probabilities(float)
        tempfile - File to store partially converged distributions to
        numiters - Number of EM iterations to run

      Before passing to this routing mean, var and c have to be initialized.
      The mean, var and c values can be initialized by doing a quick VQ on
      the data and using the means,variances and population ratios of each
      of the bins for initial values.

    The returned values are
	c[] - the a-priori probabilities of each of the modes, i.e the
	      mixing proportions (float)
        mean[][] - the means of the modes (float)
	var[][] - the variances of the modes (float)

    Note - This program requires that the varibles x[][], mean[][] and var[][]
           have been allocated using the alloc2d() routine!

    Coded by Bhiksha Raj

    Incoroporated excellent changes by PJM - Begin working in the log domain
    to eliminate corprod scaling problems. Dec 95
*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header.h"

#define  QUIT(x)  {printf x; fflush (stdout); exit(-1);}


/*----------------------------------------------------------------------------
  Routine to compute the a-posteriori probabilities of each of the modes in
  the Mixture distribution defined by c,mean and var for the the vector
  defined by x[]. Returns the posteriors in Tau[]
------------------------------------------------------------------------------*/
float Expectation(float *Tau,	/* Mode probablity of observation vector x */
		  float *x,	/* Observation vector x */
		  float *c,	/* Mixing proportions of modes */
		  float **mean,	/* Means of the modes */
		  float **hafinvvar,	/* 1.0/(2.0*variance) of the modes */
		  float *corprod,	/* Precomputed 1/sqrt(2PI**Ndim * mod(variance)) */
		  int K,	/* Number of modes */
		  int Ndim	/* Dimensionality of observation/distribution */
    )
{
	float Probability, max, sum, invsum;

	int i;


	Tau[0] = max = gauss(x, mean[0], hafinvvar[0], corprod[0], Ndim);
	for (i = 1; i < K; ++i) {
		/*
		 * Note: prior c[i] has already been factored into corprod
		 */
		Tau[i] = gauss(x, mean[i], hafinvvar[i], corprod[i], Ndim);
		if (Tau[i] > max)
			max = Tau[i];
	}

	/* convert Tau from log to linear */
	sum = 0.0;
	for (i = 0; i < K; ++i) {
		if (Tau[i] > max - 22) {	/* only consider probs within 10^10 of max */
			Tau[i] = exp(Tau[i] - max + 10);
			sum += Tau[i];
		} else
			Tau[i] = 0;
	}
	invsum = 1.0 / sum;
	for (i = 0; i < K; ++i)
		Tau[i] *= invsum;

	Probability = max + log(sum) - 10;
	return (Probability);
}


void estimate_multi_modals(float **x,	/* The observation vectors */
			   int N,	/* Number of observation vectors */
			   int Ndim,	/* Dimensionality of observations */
			   int K,	/* Number of modes in the distribution */
			   float **mean,	/* The means of the various modes */
			   float **var,	/* The variances of all the modes */
			   float *c,	/* A-priori probabilities of each of the
					   modes, or mixing proportion */
			   const char *tempfile,	/* File to store temporary distributions */
			   int numiters,	/* Number of iterations of EM to run */
			   float Threshold      /* Convergence ratio */
    )
{
	float **Newvar, **hafinvvar,
	    **Newmean,
	    *Newc,
	    *Tau,
	    *corprod,
	    Const, SumNewc, Prevlogprob, LogProb, Improvement;

	int i, j, k, iter = 0;

	FILE *temp;

	/*
	 * The Constant term that occurs in the computation of the gaussians.
	 * As it turns out, we never use it. This one is a dummy :-)
	 */
	Const = pow(2 * PI, (Ndim / 2));

	/*
	 * some initializations
	 */
	Improvement = 100;

	/*
	 * Allocate spaces for the local parameter arrays
	 */
	Newmean = (float **) ckd_calloc_2d(K, Ndim, sizeof(float));
	Newvar = (float **) ckd_calloc_2d(K, Ndim, sizeof(float));
	hafinvvar = (float **) ckd_calloc_2d(K, Ndim, sizeof(float));
	Newc = (float *) ckd_calloc(K, sizeof(float));
	Tau = (float *) ckd_calloc(K, sizeof(float));
	corprod = (float *) ckd_calloc(K, sizeof(float));

	/*
	 * Initialize all New values to 0
	 * Note the array position computation for Newmean, as it is a 1-D array
	 */
	for (k = 0; k < K; ++k) {
		Newc[k] = 0;
		for (j = 0; j < Ndim; ++j) {
			Newmean[k][j] = 0;
			Newvar[k][j] = 0;
		}
	}

	/* 
	 * Compute the ratio of the prior probabilities of the modulus of the
	 * variance. We do this operation in the log domain to keep the
	 * dynamic range in check
	 */

	for (k = 0; k < K; ++k) {
		corprod[k] = log((double) c[k]);
		for (j = 0; j < Ndim; ++j) {
			corprod[k] -= 0.5 * log((double) var[k][j]);
			if (var[k][j] > 0)
				hafinvvar[k][j] = 1.0 / (2.0 * var[k][j]);
			else
				hafinvvar[k][j] = 1e+20;
		}
	}

	/*
	 * Estimate means and variances and priors while computing overall
	 * Likelihood
	 * Note: variance estimated as Sum((x-mean)*(x-mean))/count
	 * rather than Sum(x*x)/count - mean*mean
	 * because the latter is an unstable formula and tends to give -ve
	 * variances due to numerical errors
	 */
	Prevlogprob = 0;
	for (i = 0; i < N; ++i) {
		Prevlogprob +=
		    Expectation(Tau, x[i], c, mean, hafinvvar, corprod, K,
				Ndim);
		for (k = 0; k < K; ++k) {
			if (Tau[k] > 0) {
				Newc[k] += Tau[k];
				for (j = 0; j < Ndim; ++j)
					Newmean[k][j] += Tau[k] * x[i][j];
			}
		}
	}
	for (k = 0; k < K; ++k)
		for (j = 0; j < Ndim; ++j)
			Newmean[k][j] /= Newc[k];
	for (i = 0; i < N; ++i) {
		Expectation(Tau, x[i], c, mean, hafinvvar, corprod, K,
			    Ndim);
		for (k = 0; k < K; ++k) {
			if (Tau[k] > 0) {
				for (j = 0; j < Ndim; ++j)
					Newvar[k][j] +=
					    Tau[k] * (x[i][j] -
						      Newmean[k][j])
					    * (x[i][j] - Newmean[k][j]);
			}
		}
	}
	printf("EM : Initial log probablity = %f \n", Prevlogprob);

	while ((Improvement > Threshold) && (iter < numiters)) {
		/* 
		 * We use SumNewc instead of N as in the formula because the
		 * Newc's may not sum to N, because of accuracy errors of the 
		 * computer for large N.
		 */
		SumNewc = 0;
		for (k = 0; k < K; ++k)
			SumNewc += Newc[k];

		for (k = 0; k < K; ++k) {
			for (j = 0; j < Ndim; ++j) {
				mean[k][j] = Newmean[k][j];
				var[k][j] = Newvar[k][j] / Newc[k];
			}
			c[k] = Newc[k] / SumNewc;
		}

		/*
		 * Store partially converged distribution
		 */
		temp = fopen(tempfile, "w");
		fprintf(temp, "%d %d\n", K, Ndim);
		for (i = 0; i < K; ++i) {
			fprintf(temp, "%f\n", c[i]);
			for (j = 0; j < Ndim; ++j)
				fprintf(temp, "%f ", mean[i][j]);
			fprintf(temp, "\n");
			for (j = 0; j < Ndim; ++j)
				fprintf(temp, "%f ", var[i][j]);
			fprintf(temp, "\n");
		}
		fclose(temp);

		/*
		 * Initialize all New values to 0
		 */
		for (k = 0; k < K; ++k) {
			Newc[k] = 0;
			for (j = 0; j < Ndim; ++j) {
				Newmean[k][j] = 0;
				Newvar[k][j] = 0;
			}
		}

		/* 
		 * Compute the ratio of the prior probabilities of the modulus of the
		 * variance. We do this operation in the log domain to keep the
		 * dynamic range in check
		 */

		for (k = 0; k < K; ++k) {
			corprod[k] = log((double) c[k]);
			for (j = 0; j < Ndim; ++j) {
				corprod[k] -=
				    0.5 * log((double) var[k][j]);
				if (var[k][j] > 0)
					hafinvvar[k][j] =
					    1.0 / (2.0 * var[k][j]);
				else
					hafinvvar[k][j] = 1e+20;
			}
		}

		LogProb = 0;
		for (i = 0; i < N; ++i) {
			LogProb +=
			    Expectation(Tau, x[i], c, mean, hafinvvar,
					corprod, K, Ndim);
			for (k = 0; k < K; ++k) {
				if (Tau[k] > 0) {
					Newc[k] += Tau[k];
					for (j = 0; j < Ndim; ++j) {
						Newmean[k][j] +=
						    Tau[k] * x[i][j];
						Newvar[k][j] +=
						    Tau[k] * (x[i][j] -
							      mean[k][j]) *
						    (x[i][j] - mean[k][j]);
					}
				}
			}
		}
		for (k = 0; k < K; ++k)
			for (j = 0; j < Ndim; ++j)
				Newmean[k][j] /= Newc[k];

/*
        for (i=0;i<N;++i)
        {
            Expectation(Tau,x[i],c,mean,hafinvvar,corprod,K,Ndim);
            for (k=0; k<K ; ++k)
            {
                for (j=0;j<Ndim;++j)
		    Newvar[k][j] += Tau[k] * (x[i][j] - Newmean[k][j])
		                           * (x[i][j] - Newmean[k][j]);
            }
        }
*/

		Improvement = (LogProb - Prevlogprob) / LogProb;
		if (LogProb < 0)
			Improvement = -Improvement;
		++iter;
		printf
		    ("EM : Log Prob = %f, improvement = %f after %d iterations\n",
		     LogProb, Improvement, iter);
		fflush(stdout);
		Prevlogprob = LogProb;
	}
	/*
	 * Free local arrays
	 */
	ckd_free(Tau);
	ckd_free(corprod);
	ckd_free_2d((void **)Newvar);
	ckd_free_2d((void **)Newmean);
	ckd_free(Newc);
	return;
}



