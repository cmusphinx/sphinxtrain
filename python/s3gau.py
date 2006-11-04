# s3gau.py: Read/write Sphinx-III Gaussian parameter files
#
# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III
#
# Author: David Huggins-Daines

from struct import unpack, pack
from Numeric import array, reshape, shape
from s3file import S3File, S3File_write

def open(filename, mode="rb", attr={"version":1.0}):
    if mode in ("r", "rb"):
        return S3GauFile(filename, mode)
    elif mode in ("w", "wb"):
        return S3GauFile_write(filename, mode, attr)
    else:
        raise Error, "mode must be 'r', 'rb', 'w', or 'wb'"

def open_full(filename, mode="rb", attr={"version":1.0}):
    if mode in ("r", "rb"):
        return S3FullGauFile(filename, mode)
    elif mode in ("w", "wb"):
        return S3FullGauFile_write(filename, mode, attr)
    else:
        raise Error, "mode must be 'r', 'rb', 'w', or 'wb'"

class S3GauFile(S3File):
    "Read Sphinx-III format Gaussian parameter files"
    def getall(self):
        try:
            return self._params
        except AttributeError:
            self._params = self._load()
            return self._params

    def readgauheader(self):
        if self.fileattr["version"] != "1.0":
            raise Exception("Version mismatch: must be 1.0 but is "
                            + self.fileattr["version"])
        self.fh.seek(self.data_start, 0)
        self.n_mgau = unpack(self.swap + "I", self.fh.read(4))[0]
        self.n_feat = unpack(self.swap + "I", self.fh.read(4))[0]
        self.density = unpack(self.swap + "I", self.fh.read(4))[0]
        self.veclen = unpack(self.swap + "I" * self.n_feat,
                             self.fh.read(4 * self.n_feat))
        self.blk = sum(self.veclen)
        self._nfloats = unpack(self.swap + "I", self.fh.read(4))[0]

    def _load(self):
        self.readgauheader()
        if self._nfloats != self.n_mgau * self.density * self.blk:
            raise Exception(("Number of data points %d doesn't match "
                             + "total %d = %d*%d*%d")
                            %
                            (self._nfloats,
                             self.n_mgau * self.density * self.blk,
                             self.n_mgau, self.density, self.blk))
        # First load everything into a really big Numeric array.
        # This is inefficient, but in the absence of fromfile()...
        self._allgau = array(unpack(self.swap + str(self._nfloats) + "f",
                                    self.fh.read(self._nfloats * 4)))
        # The on-disk layout is bogus so we have to slice and dice it.
        # Since feature streams are not the same dimensionality, we use
        # a two-dimensional outer list of Numeric array slices
        params = []
        r = 0
        for i in range(0, self.n_mgau):
            mgau = []
            params.append(mgau)
            for j in range(0, self.n_feat):
                rnext = r + self.density * self.veclen[j];
                gmm = reshape(self._allgau[r:rnext], (self.density, self.veclen[j]))
                mgau.append(gmm)
                r = rnext
        return params

class S3FullGauFile(S3GauFile):
    "Read Sphinx-III format Gaussian full covariance matrix files"
    def _load(self):
        self.readgauheader()
        if self._nfloats != self.n_mgau * self.density * self.blk * self.blk:
            raise Exception(("Number of data points %d doesn't match "
                             + "total %d = %d*%d*%d*%d")
                            %
                            (self._nfloats,
                             self.n_mgau * self.density * self.blk * self.blk,
                             self.n_mgau, self.density, self.blk, self.blk))
        # First load everything into a really big Numeric array.
        # This is inefficient, but in the absence of fromfile()...
        self._allgau = array(unpack(self.swap + str(self._nfloats) + "f",
                                    self.fh.read(self._nfloats * 4)))
        # The on-disk layout is bogus so we have to slice and dice it.
        # Since feature streams are not the same dimensionality, we use
        # a two-dimensional outer list of Numeric array slices
        params = []
        r = 0
        for i in range(0, self.n_mgau):
            mgau = []
            params.append(mgau)
            for j in range(0, self.n_feat):
                rnext = r + self.density * self.veclen[j] * self.veclen[j];
                gmm = reshape(self._allgau[r:rnext], (self.density,
                                                      self.veclen[j],
                                                      self.veclen[j]))
                mgau.append(gmm)
                r = rnext
        return params

class S3GauFile_write(S3File_write):
    "Write Sphinx-III format Gaussian parameter files"
    def writeall(self, stuff):
        # This will break for multi-stream files
        n_mgau, n_feat, density, veclen = shape(stuff)
        if n_feat != 1:
            raise Error, "Multi-stream files not supported"
        # Write the header
        self.fh.seek(self.data_start, 0)
        self.fh.write(pack("=IIIII",
                           n_mgau, n_feat, density, veclen,
                           n_mgau * n_feat * density * veclen))
        allgau = reshape(stuff, (n_mgau*n_feat*density*veclen,))
        self.fh.write(pack("=" + str(len(allgau)) + "f", *allgau))

class S3FullGauFile_write(S3GauFile_write):
    "Write Sphinx-III format Gaussian full covariance matrix files"
    def writeall(self, stuff):
        # This will break for multi-stream files
        n_mgau, n_feat, density, veclen, veclen2 = shape(stuff)
        if n_feat != 1:
            raise Error, "Multi-stream files not supported"
        # Write the header
        self.fh.seek(self.data_start, 0)
        self.fh.write(pack("=IIIII",
                           n_mgau, n_feat, density, veclen,
                           n_mgau * n_feat * density * veclen * veclen2))
        allgau = reshape(stuff, (n_mgau*n_feat*density*veclen*veclen2,))
        self.fh.write(pack("=" + str(len(allgau)) + "f", *allgau))
