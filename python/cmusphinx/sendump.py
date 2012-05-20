#!/usr/bin/env python

import sys
import s3mixw
import struct
import numpy


class Sendump():
    def __init__(self,filename):
	self.integer_format = None
	self.load(filename)

    def unpack_endian(self, s):
        if self.integer_format == None:
            self.integer_format = 'I'
        nbytes = struct.unpack(self.integer_format, s)[0]
        if nbytes > 65536:
            self.integer_format = '!I';
            nbytes = struct.unpack(self.integer_format, s)[0]
        return nbytes

    def readstr(self, fh):
        nbytes = self.unpack_endian(fh.read(4))
        if nbytes == 0:
             return None
        else:
             return fh.read(nbytes)[0:-1]
    
    def mixw(self):
	return self.opdf

    def load(self,filename):    
        mixture_count = 0
        model_count = 0
        mixw_shift = 10
        cluster_bits = 8
        feature_count = 0
        cluster_count = 0
        
        sendump = open(filename)
        title = self.readstr(sendump)
        while True:
            header = self.readstr(sendump)
            if header == None:
                break
            if header.startswith("mixture_count"):
                mixture_count = (int)(header.split()[1])
            if header.startswith("model_count"):
                model_count = (int)(header.split()[1])
            if header.startswith("mixw_shift"):
                mixw_shift = (int)(header.split()[1])
            if header.startswith("cluster_bits"):
                cluster_bits = (int)(header.split()[1])
            if header.startswith("feature_count"):
                feature_count = (int)(header.split()[1])
            if header.startswith("cluster_count"):
                cluster_count = (int)(header.split()[1])
        
        if cluster_count == 0:
            # Number of codewords and pdfs
            model_count = self.unpack_endian(sendump.read(4))
            mixture_count = self.unpack_endian(sendump.read(4))
        print "rows (model_count): %d, columns (mixture_count): %d, features (feature_count): %d" % (model_count, mixture_count, feature_count)
        
        # Now read the stuff
        self.opdf = numpy.empty((mixture_count, feature_count, model_count))
        for i in range(0,feature_count):
            for j in range(0,model_count):
                # Read bytes, expand to ints, shift them up
                mixw = numpy.fromfile(sendump, 'B', mixture_count).astype('i') << mixw_shift
                # Negate, exponentiate, and untranspose
                self.opdf[:,i,j] = numpy.power(1.0001, -mixw)

def usage():
    print "Usage: %s IN_SENDUMP OUT_MIXW" % sys.argv[0]

if __name__ == '__main__':
    if len(sys.argv) < 3:
        usage()
        sys.exit(2)
    s3mixw.open(sys.argv[2], 'wb').writeall(Sendump(sys.argv[1]).mixw())
