"""Train maximum-likelihood linear transforms.

This module implements the MLLT technique as described in
R. A. Gopinath, "Maximum Likelihood Modeling with Gaussian
nDistributions for Classification", in proceedings of ICASSP 1998.
"""

# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from numpy import dot, prod, diag, log, eye, sqrt, newaxis
from numpy.random import random
from numpy.linalg import det, inv
from scipy.optimize import fmin_l_bfgs_b

import sys
import s3gaucnt
import s3lda
import getopt

class MLLTModel(object):
    """Train MLLT (maximum likelihood linear transformation) from a
    set of full covariance observation counts."""
    def __init__(self, gauden_counts, ldadim=None):
        if not gauden_counts.pass2var:
            raise Exception, "Please re-run bw with '-2passvar yes'"
        self.init_gauden(gauden_counts, ldadim)

    def init_gauden(self, gauden_counts, ldadim=None):
        if ldadim == None:
            ldadim = gauden_counts.veclen[0]
        self.cov = map(lambda x: x[0][0][0:ldadim,0:ldadim], gauden_counts.getvars())
        self.count = map(lambda x: x[0][0], gauden_counts.getdnom())
        self.totalcount = sum(self.count)
        
    def objective(self, A, r, c):
        """
        Log-likelihood function and gradient for MLLT::

          L(A) = N|A| - \\sum_j \\frac{N_j}{2} \\log |diag(A \\Sigma_j A^T)|
          \\nabla L(A) = N(A^T)^{-1} - \\sum_j N_j diag(A \\Sigma_j A^T)^{-1}A\\Sigma_j
        """
        # Note: A has been flattened to make it acceptable to scipy.optimize
        A = A.reshape((r,c))
        detA = det(A)
        ll = self.totalcount * log(detA)
        lg = self.totalcount * inv(A.T)
        for j, nj in enumerate(self.count):
            C = self.cov[j]
            cl = diag(dot(dot(A, C), A.T))
            ll = ll - (float(nj) / 2) * log(prod(cl))
            lg = lg - float(nj) * dot(dot(inv(diag(cl)), A), C)
        print "likelihood: %f" % ll
        # Flatten out the gradient
        lg = lg.ravel()
        print "gradient L2: %f" % sqrt(sum(lg*lg))
        # Note: we negate these to maximize likelihood
        return -ll, -lg

    def train(self, A=None):
        """Train an MLLT transform from an optional starting point."""
        if A == None:
            # Initialize it with a random positive-definite matrix of
            # the same shape as the covariances
            s = self.cov[0].shape
            d = -1
            while d < 0:
                A = eye(s[0]) + 0.1 * random(s)
                d = det(A)
            
        # Flatten out the matrix so scipy.optimize can handle it
        AA, f, d = fmin_l_bfgs_b(self.objective, A.ravel(), args=A.shape)
        if d['warnflag']:
            print "WARNING! MLLT optimization failed to converge"
        # Unflatten the return matrix
        return AA.reshape(A.shape)

if __name__ == '__main__':
    def usage:
        sys.stderr.write("Usage: %s [-l INFILE] OUTFILE ACCUMDIRS...\n" % (sys.argv[0]))

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hl:", ["help", "lda="])
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
        if o in ('-l', '--lda'):
            ldafn = a
    mlltfn, accumdirs = args
    gauden = s3gaucnt.accumdirs_full(accumdirs)
    m = MLLTModel(gauden)
    mllt = m.train()
    if ldafn != None:
        # Compose this with the LDA transform if given
        lda = s3lda.open(ldafn).getall()
        ldadim = mllt.shape[1]
        ldamllt = dot(mllt, lda[0:ldadim])
        s3lda.open(mlltfn, 'w').writeall(ldamllt[newaxis,:])
    else:
        s3lda.open(mlltfn, 'w').writeall(mllt[newaxis,:])
