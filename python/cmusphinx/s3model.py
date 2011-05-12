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

import s3gau
import s3mixw
import s3tmat
import s3mdef
import s3file
import sys
import os
import numpy

WORSTSCORE = -100000

class S3Model(object):
    def __init__(self, path=None, topn=4):
        self.topn = topn
        self.mwfloor = 1e-5
        self.varfloor = 1e-5
        if path != None:
            self.read(path)

    def read(self, path):
        self.mdef = s3mdef.open(os.path.join(path, "mdef"))
        self.mean = s3gau.open(os.path.join(path, "means"))
        self.var = s3gau.open(os.path.join(path, "variances"))
        self.mixw = s3mixw.open(os.path.join(path, "mixture_weights"))
        self.tmat = s3tmat.open(os.path.join(path, "transition_matrices"))
        # Normalize transition matrices and mixture weights
        for t in range(0, len(self.tmat)):
            self.tmat[t] = (self.tmat[t].T / self.tmat[t].sum(1)).T
        for t in range(0, len(self.mixw)):
            self.mixw[t] = (self.mixw[t].T / self.mixw[t].sum(1)).T.clip(self.mwfloor, 1.0)
        # Floor variances and precompute normalizing and inverse variance terms
        self.norm = numpy.empty((len(self.var),
                                 len(self.var[0]),
                                 len(self.var[0][0])),'d')
        for m,mgau in enumerate(self.var):
            for f,feat in enumerate(mgau):
                fvar = feat.clip(self.varfloor, numpy.inf)
                # log of 1/sqrt((2*pi)^N * det(var))
                det = numpy.log(fvar).sum(1)
                lrd = -0.5 * (det + numpy.log(2 * numpy.pi) * feat.shape[1])
                self.norm[m,f] = lrd
                # "Invert" variances
                feat[:] = (1 / (fvar * 2))
        # Construct senone to codebook mapping
        if os.access(os.path.join(path, "senmgau"), os.F_OK):
            self.senmgau = s3file.S3File(os.path.join(path, "senmgau")).read1d()
        elif len(self.mean) == 1:
            self.senmgau = numpy.ones(len(self.mixw))
        else:
            self.senmgau = numpy.arange(0, len(self.mixw))
        self.senscr = numpy.ones(len(self.mixw)) * WORSTSCORE

    def cb_compute(self, mgau, feat, obs):
        "Compute codebook #mgau feature #feat for obs"
        mean = self.mean[mgau][feat]
        ivar = self.var[mgau][feat]
        norm = self.norm[mgau][feat]
        diff = obs - mean
        dist = (diff * ivar * diff).sum(1)
        return norm - dist
        
    def senone_compute(self, senones, *features):
        """Compute senone scores for given list of senones and a
        frame of acoustic features"""
        cbs = {}
        self.senscr[:] = WORSTSCORE
        for s in senones:
            m = self.senmgau[s]
            if not m in cbs:
                cbs[m] = [self.cb_compute(m, f, features[f])
                          for f in range(0,len(self.mean[m]))]
            score = 0
            for f, vec in enumerate(features):
                # Compute densities and scale by mixture weights
                d = cbs[m][f] + numpy.log(self.mixw[s,f])
                # Take top-N densities
                d = d.take(d.argsort()[-self.topn:])
                # Multiply into output score
                score += numpy.log(numpy.exp(d).sum())
            self.senscr[s] = score
        return numpy.exp(self.senscr - self.senscr.max())
