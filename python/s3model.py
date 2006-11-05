# s3model.py: Read Sphinx-III acoustic models
#
# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III
#
# Author: David Huggins-Daines

import s3gau, s3mixw, s3tmat, s3mdef, sys
from math import log, sqrt, pi, exp
from Numeric import shape,array,transpose

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

        print "Normalizing mixw and tmat..."
        # Normalize mixw, tmat and log-scale them
        for m in range(0, len(self.mixw)):
            feats = self.mixw[m]
            for f in range(0, len(feats)):
                feats[f] = feats[f] / sum(feats[f])
                for d in range(0, len(feats[f])):
                    # FIXME: There is a better way to do this
                    if feats[f,d] == 0:
                        feats[f,d] = WORSTSCORE
                    else:
                        feats[f,d] = log(feats[f,d])
        for t in range(0, len(self.tmat)):
            tmat = self.tmat[t]
            for r in range(0, len(tmat)):
                tmat[r] = tmat[r] / sum(tmat[r])
                for d in range(0, len(tmat[r])):
                    # FIXME: There is a better way to do this
                    if tmat[r,d] == 0:
                        tmat[r,d] = WORSTSCORE
                    else:
                        tmat[r,d] = log(tmat[r,d])

        print "Precomputing variance terms..."
        # Precompute normalizing terms
        self.norm = []
        for v in self.var:
            outfeat = []
            for f in v:
                outgau = []
                for g in f:
                    # log of 1/sqrt(2*pi**N * det(var))
                    try:
                        det = sum(log(x) for x in g)
                    except OverflowError:
                        det = -200
                    lrd = -0.5 * (det + len(g) * log(2 * pi))
                    outgau.append(lrd)
                outfeat.append(outgau)
            self.norm.append(outfeat)

        # "Invert" variances
        for m in range(0, len(self.var)):
            feats = self.var[m]
            for f in range(0, len(feats)):
                gau = feats[f]
                for g in range(0, len(gau)):
                    gau[g] = 1 / (gau[g] * 2)

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
