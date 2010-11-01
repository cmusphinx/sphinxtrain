#!/usr/bin/env python

"""
Adapt acoustic models using maximum-likelihood linear regression.

This module implements single-class mean and variance adaptation using
MLLR as described in M.J.F. Gales & P.C. Woodland, \"Mean and Variance
Adaptation within the MLLR Framework\", Computer Speech and Language,
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

def extend(mean):
    """
    Produce an "extended mean vector".
    """
    return np.concatenate(((1,),mean))

def estimate_mllr_mean(stats, inmean, invar):
    """
    Estimate an MLLR transformation of the means based on observed
    statistics.
    
    This function calculates an MLLR transformation W (an n by n+1
    matrix) for each feature stream which, when applied to C{inmean},
    maximizes the likelihood of the data as represented by C{stats}.

    Currently this does only one class, but it will promptly be
    extended once the \"learning exercise\" is over.

    @param stats: Observation counts, as returned
                  by C{cmusphinx.s3gaucnt.accumdirs}
                  or C{cmusphinx.s3gaucnt.accumdirs_full}.
    @type stats: cmusphinx.s3gaucnt.S3GauCnt
    @param inmean: Input mean parameters
    @type inmean: cmusphinx.s3gau.S3Gau
    @param invar: Input diagonal covariance parameters
    @type inmvar: cmusphinx.s3gau.S3Gau
    @return: MLLR transformations, one per feature stream
    @rtype: list(numpy.ndarray)
    """
    # List of W matrices
    Ws = []
    for i in range(0, inmean.n_feat):
        ndim = inmean.veclen[i]
        # Collection of G matrices
        G = np.zeros((ndim, ndim+1, ndim+1))
        # Z matrix (for the single class and stream)
        Z = np.zeros((ndim, ndim+1))
        # W matrix
        W = np.zeros((ndim, ndim+1))
        # One-class MLLR: just sum over all densities
        for j in range(0, inmean.n_mgau):
            for k in range(0, inmean.density):
                # Extended mean vector
                xmean = extend(inmean[j][i][k])
                # Inverse variance (also use only the diagonal)
                invvar = invar[j][i][k]
                if len(invvar.shape) > 1:
                    invvar = np.diag(invvar)
                invvar = 1./invvar.clip(1e-5,np.inf)
                # Sum of posteriors (i.e. sum_t L_m_r(t))
                dnom = stats.dnom[j,i,k]
                # Sum of mean statistics
                obsmean = stats.mean[j][i][k]
                for l in range(0, ndim):
                    # v_{ll} = sum_t L(t) \Sigma_{ll}^{-1}
                    # D = \ksi \ksi^T
                    # G^{l} = v_{ll} D
                    G[l] += dnom * invvar[l] * np.outer(xmean, xmean)
                # Z = \sum_r\sum_t L(t) \Sigma_r^{-1} o(t) \ksi_r^T
                Z += np.outer(invvar * obsmean, xmean)
        # Now solve for the rows of W
        for j in range(0, ndim):
            W[j] = np.linalg.solve(G[j], Z[j])
        Ws.append(W)
    return Ws

def write_mllr(fout, Ws, Hs=None):
    """
    Write out MLLR transformations of the means in the format that
    Sphinx3 understands.

    @param Ws: MLLR transformations of means, one per feature stream
    @ptype Ws: list(numpy.ndarray)
    @param Hs: MLLR transformations of variances, one per feature stream
    @ptype Hs: list(numpy.ndarray)
    @param fout: Filename or filehandle to write to.
    @ptype fout: string or file
    """
    if isinstance(fout, file):
        fh = fout
    else:
        fh = file(fout, 'w')
    # One-class MLLR for now
    fh.write("%d\n" % 1)
    fh.write("%d\n" % len(Ws))
    for i,W in enumerate(Ws):
        fh.write("%d\n" % W.shape[0])
        # Write rotation and bias terms separately
        for w in W:
            for x in w[1:]:
                fh.write("%f " % x)
            fh.write("\n")
        for x in W[:,0]:
            fh.write("%f " % x)
        fh.write("\n")
        if Hs != None:
            for x in Hs[i]:
                fh.write("%f " % x)
            fh.write("\n")
        else:
            fh.write("1.0 " * W.shape[0])
            fh.write("\n")

def estimate_mllr_variance(stats, inmean, invar, Ws):
    """
    Estimate a diagonal MLLR transformation of the variances based on
    observed statistics.
    
    This function calculates an MLLR transformation H (a diagonal nxn
    matrix, represented as a vector) which maximizes the likelihood of
    the data as represented by C{stats}, when applied to the inverse
    Cholesky factor of the covariance matrix B as B^T H B.  For
    diagonal covariances this reduces to a scaling of the variance by
    the diagonal of H, since the diagonal b = (sqrt(var^{-1}))^{-1} =
    var^{0.5} and thus B^T H B = \Sigma H when \Sigma and H are
    diagonal.

    Note that this function will raise an exception if -2passvar yes
    was enabled when collecting the observation counts, since it
    requires them to consist of the sum of the outer products of the
    observation vectors scaled by their posterior probabilities,
    (L_m_r(t)o(t)o(t)^T in Cambridge papers).

    Currently this does only one class and one stream, but it will
    promptly be extended once the \"learning exercise\" is over.

    @param stats: Observation counts, as returned
                  by C{cmusphinx.s3gaucnt.accumdirs}
                  or C{cmusphinx.s3gaucnt.accumdirs_full}.
    @type stats: cmusphinx.s3gaucnt.S3GauCnt
    @param inmean: Input mean parameters
    @type inmean: cmusphinx.s3gau.S3Gau
    @param invar: Input covariance parameters
    @type inmvar: cmusphinx.s3gau.S3Gau
    @param Ws: Previously computed MLLR transformations of means
    @ptype Ws: list(numpy.ndarray)
    @return: MLLR transformations of variances
    @rtype: list(numpy.ndarray)
    """
    if stats.pass2var:
        raise RuntimeException, "Statistics using -2passvar yes are not allowed"
    Hs = []
    for i, W in enumerate(Ws):
        ndim = inmean.veclen[i]
        # Output "matrix" H
        H = np.zeros(ndim)
        # One-class MLLR: just sum over all densities
        norm = 0
        for j in range(0, inmean.n_mgau):
            for k in range(0, inmean.density):
                # Extended mean vector
                xmean = extend(inmean[j][i][k])
                # Transform it
                mean = np.dot(W, xmean)
                # Cholesky factorization not needed for diagonals...
                invvar = 1./invar[j][i][k].clip(1e-5,np.inf)
                if len(invvar.shape) > 1:
                    invvar = np.diag(invvar)
                # Note: the code actually just computes diagonals
                # sum(L_m_r o o^T) (obs squared)
                nom = stats.var[j][i][k]
                # \hat mu_m_r \bar o_m_r^T (cross term 1)
                nom -= mean * stats.mean[j][i][k]
                # \bar o_m_r \hat mu_m_r^T (cross term 2)
                nom -= stats.mean[j][i][k] * mean
                # \mu_m_r \mu_m_r^T sum(L_m_r) (mean squared)
                nom += mean * mean * stats.dnom[j][i][k]
                # Multiply in variances and accumulate
                H += invvar * nom
                # Accumulate normalizer
                norm += stats.dnom[j][i][k]
        Hs.append(H / norm)
    return Hs

if __name__ == '__main__':
    def usage():
        sys.stderr.write("Usage: %s INMEAN INVAR ACCUMDIRS...\n" % sys.argv[0])

    try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["help"])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    if len(args) < 3:
        usage()
        sys.exit(2)
    ldafn = None
    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()

    inmean = s3gau.open(args[0])
    invar = s3gau.open(args[1])
    accumdirs = args[2:]
    stats = s3gaucnt.accumdirs(accumdirs)

    Ws = estimate_mllr_mean(stats, inmean, invar)
    Hs = estimate_mllr_variance(stats, inmean, invar, Ws)
    write_mllr(sys.stdout, Ws, Hs)
