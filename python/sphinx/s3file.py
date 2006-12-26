# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read/write Sphinx-III binary parameter files.

All the various binary parameter files created by SphinxTrain and used
by Sphinx-III and PocketSphinx share a common file format.  This
module contains some base classes for reading and writing these files.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from struct import unpack, pack
from numpy import array,reshape,shape,fromstring

class S3File(object):
    "Read Sphinx-III binary files"
    def __init__(self, filename=None, mode="rb"):
        if filename != None:
            self.open(filename, mode)

    def open(self, filename, mode="rb"):
        self.filename = filename
        self.fh = file(filename, mode)
        self.readheader()

    def readheader(self):
        """Read binary header.  Sets the following attributes:
        fileattr (a dictionary of attribute-value pairs)
        swap (a byteswap string as used by the struct module)
        otherend (a flag indicating if the file is wrong-endian
                  for the current platform)
        data_start (offset of the start of data in the file)"""
	spam = self.fh.readline()
        self.fileattr = {}
        if spam != "s3\n":
            raise Exception("File ID not found or invalid: " + spam)
        while True:
            spam = self.fh.readline()
            if spam == "":
                raise Exception("EOF while reading headers")
            if spam == "endhdr\n":
                break
            k, v = spam.split()
            self.fileattr[k] = v
        spam = unpack("<i", self.fh.read(4))[0]
        if spam == 0x11223344:
            self.swap = "<"
        elif spam == 0x44332211:
            self.swap = ">"
        else:
            raise Exception("Invalid byte-order mark %08x" % spam)
        self.otherend = (unpack('=i', pack(self.swap + 'i', spam)) == spam)
        self.data_start = self.fh.tell()        

    def read3d(self):
        self.d1 = unpack(self.swap + "I", self.fh.read(4))[0]
        self.d2 = unpack(self.swap + "I", self.fh.read(4))[0]
        self.d3 = unpack(self.swap + "I", self.fh.read(4))[0]
        self._nfloats = unpack(self.swap + "I", self.fh.read(4))[0]
        if self._nfloats != self.d1 * self.d2 * self.d3:
            raise Exception(("Number of data points %d doesn't match "
                             + "total %d = %d*%d*%d*%d")
                            %
                            (self._nfloats,
                             self.d1 * self.d2 * self.d3,
                             self.d1, self.d2, self.d3))
        spam = self.fh.read(self._nfloats * 4)
        params = fromstring(spam, 'f')
        if self.otherend:
            params = params.byteswap()
        return reshape(params, (self.d1, self.d2, self.d3)).astype('d')
        
class S3File_write:
    "Write Sphinx-III binary files"
    def __init__(self, filename=None, mode="wb", attr={"version":1.0}):
        self.fileattr = attr
        if filename != None:
            self.open(filename)

    def open(self, filename):
        self.filename = filename
        self.fh = file(filename, "wb")
        self.writeheader()

    def writeheader(self):
        self.fh.write("s3\n")
        for k,v in self.fileattr.iteritems():
            self.fh.write("%s %s\n" % (k,v))
        self.fh.write("endhdr\n")
        self.fh.write(pack("=i", 0x11223344))
        self.data_start = self.fh.tell()

    def write3d(self, stuff):
        d1, d2, d3 = shape(stuff)
        self.fh.write(pack("=IIII",
                           d1, d2, d3,
                           d1 * d2 * d3))
        flat = reshape(stuff, (d1*d2*d3,)).astype('f')
        self.fh.write(pack("=" + str(len(flat)) + "f", *flat))

    def __del__(self):
        self.close()

    def close(self):
        self.fh.close()
