#!/usr/bin/env python

# ====================================================================
# Copyright (c) 2009 Carnegie Mellon University.  All rights
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================

"""
Functions for reading, writing, and creating Sphinx senone-to-codebook
mapping files (and eventually compressed versions thereof).
"""
__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"

from s3file import S3File, S3File_write

import struct
import numpy

VERSION = "1.2"

def open(filename, mode="rb"):
    if mode in ("r", "rb"):
        return S3SenmgauFile(filename, mode)
    elif mode in ("w", "wb"):
        return S3SenmgauFile_write(filename, mode)
    else:
        raise Exception, "mode must be 'r', 'rb', 'w', or 'wb'"

class S3SenmgauFile(S3File):
    "Read Sphinx-III format senone mapping files"
    def __init__(self, file, mode):
        S3File.__init__(self, file, mode)
        self._params = self._load()

    def readgauheader(self):
        if self.fileattr["version"] != VERSION:
            raise Exception("Version mismatch: must be %s but is %s"
                            % (VERSION, self.fileattr["version"]))

    def read_mapping(self):
        "Read senone mapping (which uses uint32 instead of floats)"
        self.nclust = struct.unpack(self.swap + "I", self.fh.read(4))[0]
        self.nsen = struct.unpack(self.swap + "I", self.fh.read(4))[0]
        mapping = numpy.fromfile(self.fh, "int32")
        if self.otherend:
            return mapping.byteswap()
        else:
            return mapping

    def _load(self):
        self.readgauheader()
        self.fh.seek(self.data_start, 0)
        return self.read_mapping()

class S3SenmgauFile_write(S3File_write):
    "Write Sphinx-III format mixture weight files"

    def __init__(self, filename=None, mode="wb", attr={"version":VERSION}):
        S3File_write.__init__(self, filename, mode, attr)

    def write_mapping(self, mapping):
        """
        Write a senone to codebook mapping.

        The mapping takes the form of a sequence of codebook IDs, one
        for each senone.
        """
        marr = numpy.array(mapping, 'int32')
        # Number of codebooks (assumed they are sequential...)
        self.fh.write(struct.pack("=I", marr.max() + 1))
        # Number of senones
        self.fh.write(struct.pack("=I", marr.shape[0]))
        # Array of ints
        marr.tofile(self.fh)

def write_ptm_senmgau(outfile, mdef):
    """
    Create and write a senone to codebook mapping for phonetically
    tied mixtures based on a model definition (s3mdef).
    """
    mixwmap = numpy.zeros(mdef.n_sen, 'int32')
    for i in range(0, mdef.n_sen):
        mixwmap[i] = mdef.ciphone_id_from_senone_id(i)
    S3SenmgauFile_write(outfile).write_mapping(mixwmap)

def write_ptm_ci_senmgau(outfile, mdef):
    """
    Create and write a context-independent senone to codebook
    mapping for phonetically tied mixtures based on a model
    definition (s3mdef).
    """
    mixwmap = numpy.zeros(mdef.n_ci_sen, 'int32')
    for i in range(0, mdef.n_ci_sen):
        mixwmap[i] = mdef.ciphone_id_from_senone_id(i)
    S3SenmgauFile_write(outfile).write_mapping(mixwmap)

