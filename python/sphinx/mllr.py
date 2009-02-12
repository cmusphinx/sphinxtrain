#!/usr/bin/env python

"""
Adapt acoustic models using maximum-likelihood linear regression.

This module implements single-class mean and variance adaptation using
MLLR as described in M.J.F. Gales & P.C. Woodland, "Mean and Variance
Adaptation within the MLLR Framework", Computer Speech and Language,
vol. 10, pp 249-264.

TODO: Multiple regression classes.
"""

# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision $"

import numpy as np

import sys
import s3gaucnt
import s3gau
import getopt
import s3lda

def estimate_mllr_mean(stats, inmean, invar):
    """
    Estimate an MLLR transformation based on observed statistics.
    
    This function calculates an MLLR transformation W (an n by n+1
    matrix) which, when applied to C{inmean}, maximizes the likelihood
    of the data as represented by C{stats}.

    Currently this does only one class and one stream, but it will
    promptly be extended once the "learning exercise" is over.

    @param stats: Diagonal covariance observation counts, as returned
                  by C{sphinx.s3gaucnt.accumdirs}
    @type stats: sphinx.s3gaucnt.S3GauCnt
    @param inmean: Input mean parameters
    @type inmean: sphinx.s3gau.S3Gau
    @param invar: Input diagonal covariance parameters
    @type inmvar: sphinx.s3gau.S3Gau
    @return: MLLR transformation
    @rtype: numpy.ndarray
    """
    ndim = inmean.veclen[0]

    # Collection of G matrices
    G = np.zeros((ndim, ndim+1, ndim+1))
    # Z matrix (for the single class and stream)
    Z = np.zeros((ndim, ndim+1))
    # W matrix
    W = np.zeros((ndim, ndim+1))
    # Actual mean and variance parameters
    m = inmean.getall()
    v = invar.getall()
    # One-class MLLR: just sum over all densities
    for i in range(0, means.n_mgau):
        for k in range(0, means.density):
            # Extended mean vector
            xmean = np.concatenate(1, m[i,0,k])
            # Inverse variance
            invvar = 1./v[i,0,k].clip(0,1e-5)
            # Sum of posteriors (i.e. sum_t L_m_r(t))
            dnom = stats.getdnom()[i,0,k]
            # Sum of mean statistics
            obsmean = stats.getmeans()[i,0,k]
            
            for l in range(0, ndim):
                G[l] += dnom * invvar[l] * np.outer(xmean, xmean)
            Z[l] += np.outer(invvar * obsmean, xmean)
    # Now solve for the rows of W
    for i in range(0, ndim):
        W[i] = np.linalg.solve(G[i], Z[i])
    return W

if __name__ == '__main__':
    def usage():
        sys.stderr.write("Usage: %s INMEAN INVAR OUTMEAN OUTVAR ACCUMDIRS...\n" % sys.argv[0])

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hl:", ["help", "lda="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    if len(args) < 5:
        usage()
        sys.exit(2)
    ldafn = None
    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()

    inmean = s3gau.open(args[0])
    invar = s3gau.open(args[1])
    accumdirs = args[4:]
    stats = s3gaucnt.accumdirs(accumdirs)

    mllr = estimate_mllr(stats, inmean, invar)
