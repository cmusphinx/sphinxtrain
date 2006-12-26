# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Sphinx-III acoustic models.

This module provides a class which wraps a set of acoustic models, as
used by SphinxTrain, Sphinx-III, and PocketSphinx.  It provides
functions for computing Gaussian mixture densities for acoustic
feature vectors.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import s3gau, s3mixw, s3tmat, s3mdef, sys
from math import sqrt, pi, exp
from numpy import shape, array, transpose, log, clip, zeros

WORSTSCORE = -100000

def logadd(x,y):
    try:
        return x + log(1 + exp(y-x))
    except OverflowError:
        print("Warning: Overflow (%f + log(1 + exp(%f - %f)))"
              % (x, y, x))
        return WORSTSCORE

class S3Model(object):
    def __init__(self, path=None, topn=4):
        self.topn = topn
        self.mwfloor = 1e-5
        self.tpfloor = 1e-4
        if path != None:
            self.read(path)

    def read(self, path):
        self.mdef = s3mdef.open(path + "/mdef")
        self.mean = s3gau.open(path + "/means").getall()
        print ("Loaded means (%d cb, %d feat, %s gau)"
               % (len(self.mean), len(self.mean[0]), len(self.mean[0][0])))
        self.var = s3gau.open(path + "/variances").getall()
        print ("Loaded variances (%d cb, %d feat, %s gau)" %
               (len(self.var), len(self.var[0]), len(self.var[0][0])))
        self.mixw = s3mixw.open(path + "/mixture_weights").getall()
        print "Loaded mixture weights", shape(self.mixw)
        self.tmat = s3tmat.open(path + "/transition_matrices").getall()
        print "Loaded transition matrices", shape(self.tmat)

        sys.stdout.write("Flooring and normalizing mixw and tmat...")
        sys.stdout.flush()
        for t in range(0, len(self.tmat)):
            tmat = transpose(self.tmat[t])
            tmat = transpose(tmat / sum(tmat))
            self.tmat[t] = log(clip(tmat, self.tpfloor, 1.0)).astype('f')

        for t in range(0, len(self.mixw)):
            mixw = transpose(self.mixw[t])
            mixw = transpose(mixw / sum(mixw))
            self.mixw[t] = log(clip(mixw, self.mwfloor, 1.0)).astype('f')
        print "done"

        sys.stdout.write("Precomputing variance terms... ")
        sys.stdout.flush()
        # Precompute normalizing terms
        self.norm = zeros((len(self.var),
                           len(self.var[0]),
                           len(self.var[0][0])),'d')
        for m,mgau in enumerate(self.var):
            for f,feat in enumerate(mgau):
                for g,gau in enumerate(feat):
                    # log of 1/sqrt(2*pi**N * det(var))
                    try:
                        det = sum(log(x) for x in gau)
                    except OverflowError:
                        det = -200
                    lrd = -0.5 * (det + len(gau) * log(2 * pi))
                    self.norm[m,f,g] = lrd
        
        # "Invert" variances
        for m in range(0, len(self.var)):
            feats = self.var[m]
            for f in range(0, len(feats)):
                gau = feats[f]
                for g in range(0, len(gau)):
                    gau[g] = (1 / (gau[g] * 2)).astype('f')
        print "done"

    def gau_compute(self, mgau, mixw, feat, gau, obs):
        "Compute density for obs given parameters mgau,mixw,feat,gau"
        mean = self.mean[mgau][feat][gau]
        ivar = self.var[mgau][feat][gau]
        norm = self.norm[mgau][feat][gau]
        mixw = self.mixw[mixw,feat,gau]

        diff = obs - mean
        dist = sum(diff * ivar * diff)
        return norm - dist + mixw

    def cb_compute(self, mgau, feat, obs):
        "Compute codebook #mgau feature #feat for obs"
        mean = self.mean[mgau][feat]
        ivar = self.var[mgau][feat]
        norm = self.norm[mgau][feat]

        # Vectorize this for unreadability and efficiency
        obsplus = array((obs,) * len(self.mean[mgau][feat]))
        diff = obsplus - mean
        dist = sum(transpose(diff * ivar * diff))
        return norm - dist
        
    def gmm_compute(self, mgau, mixw, *features):
        "Evaluate features according to codebook #mgau with mixw #mixw"
        densities = []
        # Compute mixture density for each feature
        for f,vec in enumerate(features):
            # Compute codebook and apply mixture weights (in parallel)
            d = self.cb_compute(mgau, f, vec) + self.mixw[mixw,f]
            # Take top N:
            # Convert to a list
            d = list(d)
            # Sort in reverse orter
            d.sort(None, None, True)
            # Log-add them
            d = reduce(logadd, d[0:self.topn])
            densities.append(d)

        # Multiply their probabilities (in log domain)
        return sum(densities)
