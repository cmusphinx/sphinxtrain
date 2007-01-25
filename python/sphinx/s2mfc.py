# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read and write Sphinx feature files.

This module reads and writes the acoustic feature files used by all
Sphinx speech recognition systems.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from struct import unpack, pack
from numpy import array, reshape

def open(f, mode=None, veclen=13):
    """Open a Sphinx-II format feature file for reading or writing.
    The mode parameter is 'rb' (reading) or 'wb' (writing)."""
    if mode is None:
        if hasattr(f, 'mode'):
            mode = f.mode
        else:
            mode = 'rb'
    if mode in ('r', 'rb'):
        return S2Feat_read(f,veclen)
    elif mode in ('w', 'wb'):
        return S2Feat_write(f,veclen)
    else:
        raise Exception, "mode must be 'r', 'rb', 'w', or 'wb'"

class S2Feat_read(object):
    "Read Sphinx-II format feature files"
    def __init__(self, filename=None, veclen=13):
        self.veclen = veclen
        if (filename != None):
            self.open(filename)

    def __iter__(self):
        self.fh.seek(4,0)
        return self

    def open(self, filename):
        self.filename = filename
        self.fh = file(filename, "rb")
        self.readheader()

    def readheader(self):
        self.fh.seek(0,2)
        self.filesize = (self.fh.tell() / 4 - 1);
        self.fh.seek(0,0)
        hdrint = self.fh.read(4)
        self.hdrsize = unpack(">I", hdrint)[0]
        if self.hdrsize != self.filesize:
            self.hdrsize = (unpack("<I", hdrint))[0]
            self.swap = "<"
        else:
            self.swap = ">"
        if self.hdrsize != self.filesize:
            raise Exception('header/file size mismatch: %lx vs %lx'
                            % (self.hdrsize, self.filesize))

    def seek(self, idx):
        self.fh.seek(4 + idx * self.veclen * 4, 0)

    def next(self):
        vec = self.fh.read(self.veclen * 4)
        if vec == "":
            raise StopIteration
        return unpack(self.swap + str(self.veclen) + "f", vec)

    def readvec(self):
        return self.next()

    def getall(self):
        self.fh.seek(4,0)
        self._mfc = reshape(
            array(unpack(self.swap + str(self.hdrsize) + "f",
                         self.fh.read(self.hdrsize * 4))),
            (self.hdrsize/self.veclen, self.veclen))
        return self._mfc

class S2Feat_write(object):
    "Write Sphinx-II format feature files"
    def __init__(self, filename=None, veclen=13):
        self.veclen = veclen
        self.filesize = 0
        if (filename != None):
            self.open(filename)

    def __del__(self):
        self.close()

    def open(self, filename):
        self.filename = filename
        self.fh = file(filename, "wb")
        self.writeheader()

    def close(self):
        self.writeheader()

    def writeheader(self):
        self.fh.seek(0,0)
        self.fh.write(pack("=i", self.filesize))

    def writevec(self, vec):
        if len(vec) != self.veclen:
            raise Exception("Vector length must be %d" % self.veclen)
        self.fh.write(pack("=" + str(self.veclen) + "f", *vec))
        self.filesize = self.filesize + self.veclen

    def writeall(self, arr):
        for row in arr:
            self.writevec(row)
