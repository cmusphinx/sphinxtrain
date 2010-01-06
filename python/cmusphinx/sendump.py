#!/usr/bin/env python

import sys
import s3mixw
import struct
import numpy

def usage():
    print "Usage: %s IN_SENDUMP OUT_MIXW" % sys.argv[0]

def readstr(fh):
    nbytes = struct.unpack('I', fh.read(4))[0]
    if nbytes == 0:
        return None
    else:
        return fh.read(nbytes)

if len(sys.argv) < 3:
    usage()
    sys.exit(2)

sendump = open(sys.argv[1])
title = readstr(sendump)
while True:
    header = readstr(sendump)
    if header == None:
        break

# Number of codewords and pdfs
r, c = struct.unpack('II', sendump.read(8))
print "rows: %d, columns: %d" % (r,c)

# Now read the stuff
opdf_8b = numpy.empty((c,4,r))
for i in range(0,4):
    for j in range(0,r):
        # Read bytes, expand to ints, shift them up
        mixw = numpy.fromfile(sendump, 'B', c).astype('i') << 10
        # Negate, exponentiate, and untranspose
        opdf_8b[:,i,j] = numpy.power(1.0001, -mixw)
        
s3mixw.open(sys.argv[2], 'wb').writeall(opdf_8b)
