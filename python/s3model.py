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
from Numeric import shape

WORSTSCORE = -100000

class S3Model(object):
    def __init__(self, path=None, topn=4):
        self.topn = topn
        if path != None:
            self.read(path)

    def read(self, path):
        self.mdef = s3mdef.open(path + "/mdef")
        self.mean = s3gau.open(path + "/means").getall()
        print "Loaded means", shape(self.mean)
        self.var = s3gau.open(path + "/variances").getall()
        print "Loaded variances", shape(self.mean)
        self.mixw = s3mixw.open(path + "/mixture_weights").getall()
        print "Loaded mixture weights", shape(self.mean)
        self.tmat = s3tmat.open(path + "/transition_matrices").getall()
        print "Loaded transition matrices", shape(self.mean)

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
        for m in range(0, len(self.mixw)):
            feats = self.var[m]
            for f in range(0, len(feats)):
                gau = feats[f]
                for g in range(0, len(gau)):
                    gau[g] = 1 / (gau[g] * 2)

    def gmm_compute(self, mgau, frame):
        "Evaluate frame according to GMM #mgau"

        def logadd(x,y):
            try:
                return x + log(1 + exp(y-x))
            except OverflowError:
                print("Warning: Overflow (%f + log(1 + exp(%f - %f)))"
                      % (x, y, x))
                return WORSTSCORE
            
        # For each feature 
        densities = []
        for f in range(0,len(self.mean[mgau])):
            # For each mixture
            densities.append([])
            for g in range(0, len(self.mean[mgau][f])):
                mean = self.mean[mgau][f][g]

                ivar = self.var[mgau][f][g]
                norm = self.norm[mgau][f][g]

                diff = frame - mean
                dist = sum(diff * ivar * diff)
                den = norm - dist
                densities[f].append(den)

            # Take top N
            densities[f].sort()
            densities[f].reverse()
            # Log-add them
            #print "mgau %d feat %d densities %s" % (mgau, f, str(densities[f]))
            densities[f] = reduce(logadd, densities[f][0:self.topn])
        return sum(densities)
