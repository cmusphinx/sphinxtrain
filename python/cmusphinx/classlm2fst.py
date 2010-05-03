#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
FST utility functions
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision $"

import sphinxbase
import fstutils
import openfst
import sys

if __name__ == '__main__':
    lmfile, probdef = sys.argv[1:]
    lm = sphinxbase.NGramModel(lmfile)
    lmfst = fstutils.build_class_lmfst(lm, probdef, True)
    openfst.StdVectorFst(lmfst).Write(lmfile + ".fst")
