# s3gau.py: Read/write Sphinx-III Gaussian parameter count files
#
# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III
#
# Author: David Huggins-Daines

from struct import unpack, pack
from numpy import array, reshape, shape, fromstring
from s3file import S3File

def open(filename, mode="rb", attr={"version":1.0}):
    if mode in ("r", "rb"):
        return S3GauCntFile(filename, mode)
    else:
        raise Exception, "mode must be 'r' or 'rb'"

def open_full(filename, mode="rb", attr={"version":1.0}):
    if mode in ("r", "rb"):
        return S3FullGauCntFile(filename, mode)
    else:
        raise Exception, "mode must be 'r', 'rb'"

class S3GauCntFile(S3File):
    "Read Sphinx-III format Gaussian count files"
    def getall(self):
        try:
            return self._means, self._vars
        except AttributeError:
            self._load()
            return self._means, self._vars
        
    def getmeans(self):
        try:
            return self._means
        except AttributeError:
            self._load()
            return self._means

    def getvars(self):
        try:
            return self._vars
        except AttributeError:
            self._load()
            return self._vars

    def readgauheader(self):
        if self.fileattr["version"] != "1.0":
            raise Exception("Version mismatch: must be 1.0 but is "
                            + self.fileattr["version"])
        self.fh.seek(self.data_start, 0)
        self.has_means = unpack(self.swap + "I", self.fh.read(4))[0]
        self.has_vars = unpack(self.swap + "I", self.fh.read(4))[0]
        self.pass2var = unpack(self.swap + "I", self.fh.read(4))[0]
        self.n_mgau = unpack(self.swap + "I", self.fh.read(4))[0]
        self.density = unpack(self.swap + "I", self.fh.read(4))[0]
        self.n_feat = unpack(self.swap + "I", self.fh.read(4))[0]
        self.veclen = unpack(self.swap + "I" * self.n_feat,
                             self.fh.read(4 * self.n_feat))
        self.blk = sum(self.veclen)

    def _load(self):
        self.readgauheader()
        if self.has_means:
            self._means = self._loadgau()
        if self.has_vars:
            self._vars = self._loadgau()

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
        if self.has_means:
            self._means = self._loadgau()
        if self.has_vars:
            self._vars = self._loadfullgau()

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
