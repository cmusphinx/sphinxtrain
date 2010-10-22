#!/usr/bin/env python

import sys
import s3mixw
import struct
import numpy

def usage():
    print "Usage: %s IN_SENDUMP OUT_MIXW" % sys.argv[0]


integer_format = None

def unpack_endian(s):
    global integer_format
    if integer_format == None:
        integer_format = 'I'
    nbytes = struct.unpack(integer_format, s)[0]
    if nbytes > 65536:
        integer_format = '!I';
        nbytes = struct.unpack(integer_format, s)[0]
    return nbytes

def readstr(fh):
    nbytes = unpack_endian(fh.read(4))
    if nbytes == 0:
        return None
    else:
        return fh.read(nbytes)[0:-1]

if len(sys.argv) < 3:
    usage()
    sys.exit(2)

mixture_count = 0
model_count = 0
mixw_shift = 10
cluster_bits = 8
feature_count = 0
cluster_count = 0

sendump = open(sys.argv[1])
title = readstr(sendump)
while True:
    header = readstr(sendump)
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
    model_count = unpack_endian(sendump.read(4))
    mixture_count = unpack_endian(sendump.read(4))
print "rows (model_count): %d, columns (mixture_count): %d" % (model_count, mixture_count)

# Now read the stuff
opdf_8b = numpy.empty((mixture_count, feature_count, model_count))
for i in range(0,feature_count):
    for j in range(0,model_count):
        # Read bytes, expand to ints, shift them up
        mixw = numpy.fromfile(sendump, 'B', mixture_count).astype('i') << mixw_shift
        # Negate, exponentiate, and untranspose
        opdf_8b[:,i,j] = numpy.power(1.0001, -mixw)

s3mixw.open(sys.argv[2], 'wb').writeall(opdf_8b)
