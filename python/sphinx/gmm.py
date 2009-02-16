# Copyright (c) 2008 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Train generic Gaussian Mixture Models from speech data.

This module defines a GMM class which can be used to train generic
models of speech for use in speaker identification or VTLN.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

import sys
import os
import s3gau
import s3mixw
import numpy

def logadd(x,y):
    """Log-add two numbers."""
    return x + numpy.log(1 + numpy.exp(y-x))

class GMM(object):
    """
    Class representing a Gaussian Mixture Model.
    """
    def __init__(self, fromdir=None,
                 featlen=13, ndensity=256,
                 mixwfloor=0.001, varfloor=0.001):
        """
        Constructor for GMM class.
        @param fromdir: Directory to read initial parameters from.
        @ptype fromdir: string
        @param featlen: Dimensionality of input features.
        @ptype featlen: int
        @param ndensity: Number of Gaussian components.
        @ptype ndensity: int
        @param varfloor: Floor value to apply to variances before evaluation.
        @ptype varfloor: float
        @param mixwfloor: Floor value to apply to mixture weights before evaluation.
        @ptype mixwfloor: float
        """
        if fromdir != None:
            self.read(fromdir)
        else:
            self.random_init(featlen, ndensity)
        self.varfloor = varfloor
        self.mixwfloor = mixwfloor
        self.precompute()
        self.reset()

    def read(self, fromdir):
        """
        Read GMM parameters from files in a directory.
        @param fromdir: Directory to read parameters from.  The files
        'means', 'variances', and 'mixture_weights' will be read from
        this directory.
        @ptype fromdir: string
        """
        self.means = s3gau.open(os.path.join(fromdir, "means"))
        self.variances = s3gau.open(os.path.join(fromdir, "variances"))
        self.mixw = s3mixw.open(os.path.join(fromdir, "mixture_weights"))
        self.featlen = self.means.veclen[0]
        self.ndensity = self.means.density

    def write(self, todir):
        """
        Write GMM parameters to files in a directory.
        @param todir: Directory to read parameters from.  The files
        'means', 'variances', and 'mixture_weights' will be created in
        this directory.
        @ptype todir: string
        """
        s3gau.open(os.path.join(todir, "means"),'wb').writeall([[self.means]])
        s3gau.open(os.path.join(todir, "variances"),'wb').writeall([[self.variances]])
        s3mixw.open(os.path.join(todir, "mixture_weights"),'wb').writeall \
            (self.mixw[numpy.newaxis,numpy.newaxis,:])

    def random_init(self, featlen=13, ndensity=256):
        """
        Initialize parameters with arbitrary initial values.
        """
        self.means = numpy.random.random((ndensity, featlen)) * 10 - 5
        self.variances = numpy.ones((ndensity,featlen))
        self.mixw = numpy.random.random(ndensity)
        self.mixw /= self.mixw.sum()
        self.featlen = featlen
        self.ndensity = ndensity

    def precompute(self):
        """
        Precompute Gaussian invariants for density calculation.
        """
        variances = self.variances.clip(self.varfloor, numpy.inf)
        mixw = self.mixw.clip(self.mixwfloor, numpy.inf)
        self.inv_var = 0.5/variances
        self.log_det_var = (numpy.log(mixw) - # mixw * 1 /
                            0.5 * # sqrt
                            (self.featlen * numpy.log(2 * numpy.pi) # 2pi ** featlen
                             + numpy.log(variances).sum(1)))   # prod(v for v in variances)

    def reset(self):
        """
        Reset internal accumulators.
        """
        self.mixwacc = numpy.zeros(self.ndensity, 'd')
        self.meanacc = numpy.zeros((self.ndensity,self.featlen), 'd')
        self.varacc = numpy.zeros((self.ndensity,self.featlen), 'd')
        self.nfr = 0
        self.avgll = 0.0

    def evaluate(self, frames, accumulate=True):
        """
        Evaluate one or more frames of data according to the model.
        @param frames: Array of frames of data.
        @ptype frames: numpy.ndarray
        @param accumulate: Whether to accumulate counts for training from this data.
        @ptype accumulate: boolean
        @return: Average log-likelihood of data per frame.
        @rtype: float
        """
        diff = numpy.zeros((self.ndensity, self.featlen), 'd')
        post = numpy.zeros(self.ndensity, 'd')
        avgll = 0.0
        for frame in frames:
            self.nfr += 1
            diff = frame - self.means
            post = self.log_det_var - (diff * self.inv_var * diff).sum(1)
            # Likelihood = sum of Gaussian densities
            ll = reduce(logadd, post)
            self.avgll += ll
            avgll += ll
            if accumulate:
                # Normalize them to get posterior probabilities for each mixture
                post = numpy.exp(post - ll)
                # Mixture weight counts are just sums of posteriors
                self.mixwacc += post
                # Accumulate mean and variance counts
                self.meanacc += post[:,numpy.newaxis] * frame
                self.varacc += post[:,numpy.newaxis] * diff * diff
        return avgll / len(frames)

    def normalize(self):
        """
        Normalize accumulation counts to obtain updated parameters.
        """
        for i in range(0, self.ndensity):
            if self.mixwacc[i] == 0:
                sys.stderr.write("Warning: mixture %d never observed\n" % i)
                # Copy from previous density if possible
                idx = max(i-1, 0)
                self.means[i] = self.means[idx]
                self.variances[i] = self.variances[idx]
                self.mixwacc[i] = self.mixwacc[idx]
            else:
                # Mixture weight counts conveniently serve as occupation counts
                self.means[i] = self.meanacc[i] / self.mixwacc[i]
                self.variances[i] = self.varacc[i] / self.mixwacc[i]
        self.mixw = self.mixwacc / self.nfr
        # Recompute things for evaluation
        self.precompute()
        return self.avgll / self.nfr

