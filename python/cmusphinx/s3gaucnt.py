# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read/write Sphinx-III Gaussian parameter count files.

This module reads and writes the expected Gaussian mixture occupancy
count files created by SphinxTrain's implementation of the
Forward-Backward algorithm for training (semi-)continuous HMMs.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from struct import unpack, pack
from numpy import array, reshape, shape, fromstring
from s3file import S3File
import os

def open(filename, mode="rb", attr={"version":1.0}):
    if mode in ("r", "rb"):
        return S3GauCntFile(filename, mode)
    else:
        raise Exception, "mode must be 'r' or 'rb'"

def accumdirs(accumdirs):
    "Read and accumulate counts from several directories"
    gauden = None
    for d in accumdirs:
        try:
            subgau = S3GauCntFile(os.path.join(d, "gauden_counts"), "rb")
        except:
            subgau = None
            continue
        if gauden == None:
            gauden = subgau
        else:
            for m, mgau in enumerate(gauden.mean):
                for f, feat in enumerate(mgau):
                    gauden.mean[m][f] += subgau.mean[m][f]
                    gauden.var[m][f] += subgau.var[m][f]
                    gauden.dnom[m][f] += subgau.dnom[m][f]
    return gauden

def accumdirs_full(accumdirs):
    "Read and accumulate full-covariance counts from several directories"
    gauden = None
    for d in accumdirs:
        try:
            subgau = S3FullGauCntFile(os.path.join(d, "gauden_counts"), "rb")
        except:
            subgau = None
            continue
        if gauden == None:
            gauden = subgau
        else:
            for m, mgau in enumerate(gauden.mean):
                for f, feat in enumerate(mgau):
                    gauden.mean[m][f] += subgau.mean[m][f]
                    gauden.var[m][f] += subgau.var[m][f]
                    gauden.dnom[m][f] += subgau.dnom[m][f]
    return gauden

def open_full(filename, mode="rb", attr={"version":1.0}):
    if mode in ("r", "rb"):
        return S3FullGauCntFile(filename, mode)
    else:
        raise Exception, "mode must be 'r', 'rb'"

class S3GauCntFile(S3File):
    "Read Sphinx-III format Gaussian count files"
    def __init__(self, file, mode):
        S3File.__init__(self, file, mode)
        self._load()

    def readgauheader(self):
        if self.fileattr["version"] != "1.0":
            raise Exception("Version mismatch: must be 1.0 but is "
                            + self.fileattr["version"])
        self.fh.seek(self.data_start, 0)
        self.has_mean = unpack(self.swap + "I", self.fh.read(4))[0]
        self.has_var = unpack(self.swap + "I", self.fh.read(4))[0]
        self.pass2var = unpack(self.swap + "I", self.fh.read(4))[0]
        self.n_mgau = unpack(self.swap + "I", self.fh.read(4))[0]
        self.density = unpack(self.swap + "I", self.fh.read(4))[0]
        self.n_feat = unpack(self.swap + "I", self.fh.read(4))[0]
        self.veclen = unpack(self.swap + "I" * self.n_feat,
                             self.fh.read(4 * self.n_feat))
        self.blk = sum(self.veclen)

    def _load(self):
        self.readgauheader()
        if self.has_mean:
            self.mean = self._loadgau()
        if self.has_var:
            self.var = self._loadgau()
        self.dnom = self.read3d()

    def _loadgau(self):
        self._nfloats = unpack(self.swap + "I", self.fh.read(4))[0]
        if self._nfloats != self.n_mgau * self.density * self.blk:
            raise Exception(("Number of data points %d doesn't match "
                             + "total %d = %d*%d*%d")
                            %
                            (self._nfloats,
                             self.n_mgau * self.density * self.blk,
                             self.n_mgau, self.density, self.blk))
        spam = self.fh.read(self._nfloats * 4)
        data = fromstring(spam, 'f')
        if self.otherend:
            data = data.byteswap()
        params = []
        r = 0
        for i in range(0, self.n_mgau):
            mgau = []
            params.append(mgau)
            for j in range(0, self.n_feat):
                rnext = r + self.density * self.veclen[j];
                gmm = reshape(data[r:rnext], (self.density, self.veclen[j]))
                mgau.append(gmm)
                r = rnext
        return params        

class S3FullGauCntFile(S3GauCntFile):
    "Read Sphinx-III format Gaussian full covariance matrix files"
    def _load(self):
        self.readgauheader()
        if self.has_mean:
            self.mean = self._loadgau()
        if self.has_var:
            self.var = self._loadfullgau()
        self.dnom = self.read3d()

    def _loadfullgau(self):
        self._nfloats = unpack(self.swap + "I", self.fh.read(4))[0]
        if self._nfloats != self.n_mgau * self.density * self.blk * self.blk:
            raise Exception(("Number of data points %d doesn't match "
                             + "total %d = %d*%d*%d*%d")
                            %
                            (self._nfloats,
                             self.n_mgau * self.density * self.blk * self.blk,
                             self.n_mgau, self.density, self.blk, self.blk))
        spam = self.fh.read(self._nfloats * 4)
        data = fromstring(spam, 'f')
        if self.otherend:
            data = data.byteswap()
        params = []
        r = 0
        for i in range(0, self.n_mgau):
            mgau = []
            params.append(mgau)
            for j in range(0, self.n_feat):
                rnext = r + self.density * self.veclen[j] * self.veclen[j];
                gmm = reshape(data[r:rnext], (self.density,
                                              self.veclen[j],
                                              self.veclen[j]))
                mgau.append(gmm)
                r = rnext
        return params
